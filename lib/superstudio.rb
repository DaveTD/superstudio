module Superstudio
  class SqlJsonBuilder

    attr_accessor :sql_columns, :row_being_used, :json_result, :schema, :template_body, :template_bodies, :template_types, :array_paths, :json_nodes

    def initialize(query, file_name = nil)
      file_class_name = self.class.name
      file_class_name.slice!("Mapper")
      file_name ||= file_class_name.snakecase << ".json.schema"

      @schema = parse_json_schema(file_name)
      @sql_columns = []
      @row_being_used = []
      @array_paths = [] # a path name points to a depth so that we can build from the deepest up
      @json_result = ""
      @json_nodes = {}

      json_schema_interpretation = interpret_json_schema(@schema)

      if query.present?
        result_set = get_sql_results(query)
        assemble_json(result_set, json_schema_interpretation)
      else
        @template_body, @template_types = create_template(json_schema_interpretation)
      end
    end

    private
    def schemas_directory
      if ENV['JSON_SCHEMAS_DIRECTORY'].nil?
        Rails.root.join('app', 'json_schemas')
      else
        ENV['JSON_SCHEMAS_DIRECTORY']
      end
    end

    # Get the file contents as a string and interpret as JSON
    def parse_json_schema(file_name)
    # version 0.4.0 will need to nest referenced schema pieces into the file_location's schema here
      directory = schemas_directory
      master_file = directory.join(file_name)
      contents = File.read(master_file)
      schema = JSON.parse(contents)
      return schema
    end

    def get_sql_results(query)
      # if this is an AR query, find the resulting query that would be run
      if query.respond_to?(:to_sql)
        query = query.to_sql
      end

      result_set = ActiveRecord::Base.connection.select_all(query)
    end

    # Convenience for finding the row value for the column name
    def value_by_column_name(name)
      return @row_being_used[@sql_columns.index(name)]
    end

    # Convenience for finding the row value for a column number - in case people went and had multiple columns named the same thing
    def value_by_column_number(number)
      return @row_being_used[number]
    end

    # inserting to the json_result, figuring out if this is the first item in the json_result or not first
    def insert_json_to_response(template, row_hash)
      new_response_node = template % row_hash
      if @json_result.empty?
        @json_result << new_response_node
      else
        @json_result << ",#{new_response_node}"
      end
    end

    #  #####################  #
    #  Parsing the JSON spec  #
    #  #####################  #

    # Search for "type" nodes
    # A type of "object" denotes a branch
    # A type of "array" indicates an array
    # All other types are expected columns
    # For non-object types, find the parent node, that will be the related name
    # Almost everything else can be ignored
    def interpret_json_schema(json_hash, depth = 0, path_array = [], expected_mappings = [], node_name = 'root')
      json_hash = replace_reference_keys(json_hash)
      level_keys = json_hash.keys
      level_keys.each do |key|
        if key == "type"
          if ["integer", "boolean", "number", "string"].include?(json_hash[key])
            return {depth: depth, path: path_array, name: node_name, node_type: json_hash[key]}
          end
        end

        if json_hash[key].is_a?(Hash)
          new_path_array = path_array.dup
          new_depth = depth
          if node_name != "properties" && node_name != "items"
            new_path_array << node_name
            new_depth = new_depth + 1
          end
          expected_mappings << interpret_json_schema(json_hash[key], new_depth, new_path_array, [], key)
        end

        if key == "items"
          # when we find items, we need to indicate that this is an array, and that the types that follow can be
          # repeated multiple times.
          new_path_array = path_array.dup
          new_path_array << node_name
          array_paths.push(new_path_array)
          #new_name = node_name.dup << '_array'
          expected_mappings << {depth: depth, path: path_array, name: node_name, node_type: json_hash[key]}
        end

      end
      return expected_mappings.flatten
    end

    def replace_reference_keys(json_hash)
      if json_hash.keys.include?("$ref")
        # split on a hash character - the first piece is the file name
        file_name = json_hash["$ref"].split('#')[0]
        referenced_contents = parse_json_schema(file_name)
        # replace reference keys in the referenced contents
        referenced_contents = replace_reference_keys(referenced_contents)
        json_hash.delete("$ref")
        json_hash = json_hash.merge(referenced_contents)
      end
      return json_hash
    end

    # Generate a string JSON template like:
    # {
    #   "firstName": %{root_firstName},
    #   "lastName": %{root_lastName},
    #   "age": %{root_age},
    #   "foo": {
    #     "bar": %{root_foo_bar},
    #     "bin": %{root_foo_bin}
    #   }
    # }
    # Each row returned from an SQL query will be used to populate these variables in a hash
    # There is a chance that using more c-like notation like %s and %i would improve speeds
    # Need to test out a mapping between these templates and an ordering of items instead to
    # see if performance improves
    def create_template(expected_mappings)
      fork_nodes = expected_mappings.uniq { |i| i[:path] }
      max_depth = expected_mappings.max_by { |x| x[:depth] }[:depth]
      nested_objects = {}

      template_nodes = []
      @template_body = ""
      @template_types = {}

      @template_bodies = {}
      @array_paths.each do |array_path|
        @template_bodies[array_path] = ""
      end

      # go through each depth, building upwards
      while max_depth != 0
        objects_at_depth = fork_nodes.select {|j| j[:depth] == max_depth}
        objects_at_depth.each do |object|
          object_path_string = object[:path].join("_")

          object_properties = expected_mappings.select { |mappings| mappings[:path] == object[:path] }
          object_string = "{"
          object_properties.each do |property, index|
            expected_hash_key = "#{object_path_string}_#{property[:name]}"
            object_string << "\"#{property[:name]}\":%{#{expected_hash_key}},"
            template_nodes << expected_hash_key
            @template_types[expected_hash_key.to_sym] = property[:node_type]
          end
          # check for and add nested objects
          nested_objects.each do |nested_object|
            if object[:path] == (nested_object[0].slice(0, object[:path].length)) && (object[:path].length + 1 == nested_object[0].length)
              object_string << "\"#{nested_object[0].slice(object[:path].length)}\":"
              object_string << "#{nested_object[1]},"
            end
          end
          # cut off the last "," from the string
          object_string = object_string[0..-2]
          object_string << "}"
          # if this isn't the root node, put it in the nested_objects hash
          if max_depth > 1
            if @template_bodies.has_key?(object[:path])
              @template_bodies[object[:path]] = object_string
            else
              nested_objects[object[:path]] = object_string
            end
          else
            @template_body = object_string
            @template_bodies[['root']] = object_string
          end
        end
        max_depth = max_depth - 1
      end
    end

    #  #####################  #
    #  Parsing the JSON spec  #
    #  #####################  #

    #  ###################  #
    #  Assembling the JSON  #
    #  ###################  #

    def assemble_json(result_set, expected_mappings)
      create_template(expected_mappings)
      @sql_columns = result_set.columns

      # we need a set of row hashes to which new rows can have their array items added
      # example:
      # array_paths = [['root'], ['root', 'variants'], ['root', 'variants', 'tax_category']]
      #
      # 1. Find the deepest path (root, variants, tax_category)
      # 2. Find the associated keys to that path (variants_tax_category_id, variants_tax_category_name)
      # 3. Find the associated keys to its parent and up (id, foo, variants_id, variants_name)

      # map row comes from the inheriting class
      result_set.rows.each do |row|
        @json_nodes = {}
        @row_being_used = row
        map_row
        # based on template types, determine what needs to go in quotes and what needs to be called "null"
        row_hash = clean_data_types(@json_nodes, @template_types)

        # Slap the cleaned hash onto each template body


        # Sort through the template bodies to figure out where each should go

        insert_json_to_response(@template_body, row_hash)
      end
    end

    def clean_data_types(row_hash, template_types)
      row_hash.keys.each do |key|
        if template_types[key] == "string" && row_hash[key] != nil
          row_hash[key] = "\"#{row_hash[key]}\""
        end
        row_hash[key] = "null" if row_hash[key].nil?
      end
      return row_hash
    end

    #  ###################  #
    #  Assembling the JSON  #
    #  ###################  #
  end
end

# Development Plan

# version 0.1.x - single schema works
# version 0.2.x - use mapping files to map from result set to schema
# version 0.3.x - allow n-depth schemas, correctly handle nulls
# version 0.4.x - allow "$ref" to nest schemas
# version 0.5.x - testing types between result set and expected schema values
# version 0.6.x - support arrays of objects
# version 0.7.x - automatically generate data maps with all expected variables
# version 0.8.x - support all settings for current draft of json schema
# version 1.0.x - test suite

# At some point I will address the problem of referencing schemas that define a node to be used, and of using references that are not on the same server
