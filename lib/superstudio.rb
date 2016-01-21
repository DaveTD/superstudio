module Superstudio
  class SqlJsonBuilder
    attr_accessor :sql_columns, :row_being_used, :json_result, :schema, :template_bodies, :template_types, :array_paths, :json_nodes

    def initialize(query, file_name = nil)
      file_class_name = self.class.name
      file_class_name.slice!("Mapper")
      file_name ||= file_class_name.snakecase << ".json.schema"

      @schema = parse_json_schema(file_name)
      @sql_columns, @row_being_used, @array_paths = [], [], []
      @json_result = ""
      @json_nodes = {}

      json_schema_interpretation = interpret_json_schema(@schema)

      if query.present?
        result_set = get_sql_results(query)
        create_template(json_schema_interpretation)
        assemble_json(result_set, json_schema_interpretation)
      else
        create_template(json_schema_interpretation)
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

    def parse_json_schema(file_name)
      directory = schemas_directory
      master_file = directory.join(file_name)
      contents = File.read(master_file)
      schema = JSON.parse(contents)
      return schema
    end

    def get_sql_results(query)
      query = query.to_sql if query.respond_to?(:to_sql)
      result_set = ActiveRecord::Base.connection.select_all(query)
    end

    def value_by_column_name(name)
      return @row_being_used[@sql_columns.index(name)]
    end

    def value_by_column_number(number)
      return @row_being_used[number]
    end

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
          new_path_array = path_array.dup
          new_path_array << node_name
          array_paths.push(new_path_array)
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
        referenced_contents = replace_reference_keys(referenced_contents)
        json_hash.delete("$ref")
        json_hash = json_hash.merge(referenced_contents)
      end
      return json_hash
    end


    def create_template(expected_mappings)
      fork_nodes = expected_mappings.uniq { |i| i[:path] }
      max_depth = expected_mappings.max_by { |x| x[:depth] }[:depth]
      nested_objects = {}

      template_nodes = []
      @template_types = {}

      @template_bodies = {}
      @array_paths.each do |array_path|
        @template_bodies[array_path] = ""
      end

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
          object_string = object_string.chomp(",")
          object_string << "}"
          # if this isn't the root node, put it in the nested_objects hash
          if max_depth > 1
            if @template_bodies.has_key?(object[:path])
              @template_bodies[object[:path]] = object_string
            else
              nested_objects[object[:path]] = object_string
            end
          else
            @template_bodies[['root']] = object_string
          end
        end
        max_depth = max_depth - 1
      end
    end

    def assemble_json(result_set, expected_mappings)
      @sql_columns = result_set.columns
      complete_templates = Hash.new{|hash, key| hash[key] = Hash.new}

      # map row comes from the inheriting class
      result_set.rows.each do |row|
        @row_being_used = row
        @json_nodes = {}
        map_row
        row_hash = clean_data_types(@json_nodes, @template_types)
        key_depth = 0
        # Sort the template bodies so that the longest keys come first
        @template_bodies = @template_bodies.sort{ |k,v| k.length }.reverse.to_h

        @template_bodies.each do |key, template|
          key_eliminator = ''
          working_hash = row_hash.dup
          local_values = {}
          # Find the depth of this key
          key_depth = key.length
          # Find all other keys of that depth
          keys_at_depth = @template_bodies.select{ |k,v| k.length == key_depth }
          # Eliminate variables of that depth, or deeper
          keys_at_depth.keys.each do |depth_key|
            key_eliminator = depth_key.join('_')
            local_values = working_hash.select { |k,v| k.to_s.start_with?(key_eliminator) }
            working_hash.delete_if { |k,v| k.to_s.start_with?(key_eliminator)}
          end

          # Key on the remaining values
          valued_key = ""
          working_hash.each do |key, value|
            valued_key << value.to_s << "_"
          end
          valued_key = valued_key.chomp("_").to_sym

          # set the array key value for this row to the new, generated key, as a replacable template
          # this goes in the row_hash, because it will be copied down into further templates with shorter keys
          # where the key_eliminator is this array (for this row only)
          row_hash[key_eliminator.to_sym] = "[%{#{valued_key}}]"

          # fill in the array values for the row_hash
          complete_template = template % local_values

          # Merge this template with any other templates of that key - this is another item in that array
          if complete_templates[key_depth][valued_key].present?
            if !complete_templates[key_depth][valued_key].include?(complete_template)
              complete_templates[key_depth][valued_key] = complete_templates[key_depth][valued_key] << "," << complete_template
            end
          else
            complete_templates[key_depth][valued_key] = complete_template
          end
        end
      end
      # Merge templates together
      max_depth = complete_templates.max_by{|k,v| k}.first
      while max_depth > 0
        complete_templates[max_depth].each do |k,v|
          complete_templates[max_depth][k] = complete_templates[max_depth][k] % complete_templates[max_depth + 1]
        end
        max_depth -= 1
      end
      @json_result << complete_templates[1].values.join(',')
    end

    def clean_data_types(row_hash, template_types)
      row_hash.keys.each do |key|
        if template_types[key] == "string" && row_hash[key] != nil
          row_hash[key] = "\"#{row_hash[key]}\""
          row_hash[key] = row_hash[key].gsub("%", "%%")
        end
        row_hash[key] = "null" if row_hash[key].nil?
      end
      return row_hash
    end
  end
end

# Development Plan

# version 0.6.x - support arrays of objects
# version 0.7.x - automatically generate data maps with all expected variables
# version 0.8.x - support all settings for current draft of json schema
# version 1.0.x - test suite

# At some point I will address the problem of referencing schemas that define a node to be used, and of using references that are not on the same server
# Each row returned from an SQL query will be used to populate these variables in a hash
    # There is a chance that using more c-like notation like %s and %i would improve speeds
    # Need to test out a mapping between these templates and an ordering of items instead to
    # see if performance improves
# Eventually I'll take a look at having variable keys
# Eventually I'll look at performance for replacing all this array/hash/string stuff with a few tree structure objects
