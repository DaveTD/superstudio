require 'superstudio.bundle'

module Superstudio
  class SqlJsonBuilder
    attr_accessor :sql_columns, :json_result, :schema, :template_bodies, :template_types, :array_paths, :json_nodes, :required_columns, :human_to_internal

    def initialize(query, file_name = nil)
      file_class_name = self.class.name
      file_class_name.slice!("Mapper")
      file_name ||= file_class_name.underscore << ".json.schema"

      @schema = parse_json_schema(file_name)
      @sql_columns, @row_being_used = [], []
      @json_result = ""
      @json_nodes, @required_columns = {}, {}

      @human_readable_tags, @internal_use_tags, @quoted_tags, @do_not_hash, @depth_tags, @real_depth_tags = [], [], [], [], [], []
      @type_2_paths, @type_3_paths, @type_4_paths, @type_5_paths = [], [], [], []

      json_schema_interpretation = interpret_json_schema(@schema)

      if query.present?
        result_set = get_sql_results(query)
        create_template(json_schema_interpretation)
        set_human_to_internal_mappings(json_schema_interpretation)
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
      value = @row_being_used[@sql_columns.index(name)]
      value ||= ""
      return value
    end

    def value_by_column_number(number)
      value = @row_being_used[number]
      value ||= ""
      return value
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

        if key == "required"
          # this will be especially important since according to http://spacetelescope.github.io/understanding-json-schema/reference/object.html
          # each required string must be unique - which gives us an opportunity to use much shorter keys
        end

        if key == "patternProperties"
          # this will let this library output uniquely keyed properties - which will be especially useful if we want to create ID keyed hashes
        end

        if json_hash[key].is_a?(Hash)
          new_path_array = path_array.dup
          new_depth = depth
          if node_name != "properties" && node_name != "items" && node_name != "dependencies"
            new_path_array << node_name
            new_depth = new_depth + 1
            @type_2_paths << new_path_array
          end
          expected_mappings << interpret_json_schema(json_hash[key], new_depth, new_path_array, [], key)
        end

        if key == "items"
          new_path_array = path_array.dup
          new_path_array << node_name
          if json_hash[key].is_a?(Array)
            # solve this later
          else
            # detect if this is an array of objects or values
            if json_hash.count == 2

            else

            end
          end

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

    def set_human_to_internal_mappings(expected_mappings)
      fork_nodes = expected_mappings.uniq { |i| i[:path] }
      max_depth = expected_mappings.max_by { |x| x[:depth] }[:depth]
      internal_fork_numbers = convert_fork_paths_to_base_route_numbers(fork_nodes, max_depth)
      depth_counter = 0
      while depth_counter <= max_depth
        # Assign all internal numbers to their respective human-readable forms
        objects_at_depth = expected_mappings.select { |j| j[:depth] == depth_counter }
        forks_at_depth = internal_fork_numbers.select { |k,v| v[:depth] == depth_counter }
        type_1_count, type_3_count, type_4_count, type_5_count = 0

        objects_at_depth.each do |candidate|
          forks_at_depth.each do |p|
            if candidate[:path] == p[0]
              # When we have a match, determine the type that this object is, by checking the non-1 arrays

              # Add the internal path to the internal mapping
              type_1_count += 1
              add_to_describe_arrays("#{candidate[:path].join("_B_")}_P_#{candidate[:name]}", p[1][:internal_path], "1.#{type_1_count}", candidate[:node_type], candidate[:depth])
            end
          end
          # Unless there are no forks for this depth, then we're at the root node, and we should do this differently
          if !forks_at_depth.present?
            # Check the type, increment, send
            type_1_count += 1
            add_to_describe_arrays("#{candidate[:path].join("_B_")}_P_#{candidate[:name]}", "", "1.#{type_1_count}", candidate[:node_type], candidate[:depth])
          end
        end
        depth_counter += 1
      end
    end

    def add_to_describe_arrays(human_route, parent_path, item_path, node_type, depth)
      @human_readable_tags << human_route
      @depth_tags << depth

      if node_type == "string"
        @quoted_tags << 1
      else
        @quoted_tags << 0
      end

      if parent_path.present?
        item_string = "#{parent_path}-#{item_path}"
        @internal_use_tags << item_string
        @real_depth_tags << item_string.count("-4")
      else
        @internal_use_tags << "#{item_path}"
        @real_depth_tags << 0
      end

      if item_path.chr == '3'
        @do_not_hash << 1
      else
        @do_not_hash << 0
      end
    end

    def convert_fork_paths_to_base_route_numbers(fork_nodes, max_depth)
      depth_counter = 0
      internal_fork_numbers = {}

      while depth_counter <= max_depth
        two_at_depth_counter, four_at_depth_counter, five_at_depth_counter = 0
        objects_at_depth = fork_nodes.select {|j| j[:depth] == depth_counter}
        possible_parents = fork_nodes.select {|j| j[:depth] == (depth_counter - 1)}
        objects_at_depth.each do |o|

          parent = nil
          if o[:path].join("_B_") == 'root'
            internal_fork_numbers[o[:path]] = { internal_path: '', depth: o[:depth] }
            break
          end

          if @type_2_paths.include?(o[:path])
            two_at_depth_counter += 1
            # here we need to record that the path
            if possible_parents
              # find parent, there will be one that starts with this path
              possible_parents.each do |pp|
                parent = pp if o[:path][0..-2] = pp[:path]
              end
            end

            internal_path = ""
            internal_path = "#{internal_fork_numbers[parent[:path]][:internal_path]}" if parent.present?
            if parent.present? && parent[:path].join("_B_") != 'root'
              internal_path << "-" # if parent.present? && parent[:path] != 'root'
            end
            internal_path << "2.#{two_at_depth_counter}"
            internal_fork_numbers[o[:path]] = { internal_path: internal_path, depth: o[:depth] }
          end

          if @type_3_paths.include?(o[:path])

          end

          if @type_4_paths.include?(o[:path])

          end

          if @type_5_paths.include?(o[:path])

          end

        end

        depth_counter += 1
      end

      return internal_fork_numbers
    end

    def create_template(expected_mappings)
      fork_nodes = expected_mappings.uniq { |i| i[:path] }
      max_depth = expected_mappings.max_by { |x| x[:depth] }[:depth]

      @template_bodies = {}

      while max_depth != 0
        objects_at_depth = fork_nodes.select {|j| j[:depth] == max_depth}
        objects_at_depth.each do |object|
          object_path_string = object[:path].join("_B_")
          object_properties = expected_mappings.select { |mappings| mappings[:path] == object[:path] }
          object_string = "{"
          object_properties.each do |property, index|
            expected_hash_key = "#{object_path_string}_P_#{property[:name]}"
            object_string << "\"#{property[:name]}\":{#{expected_hash_key}},"
          end
          object_string = object_string.chomp(",")
          object_string << "}"

          @template_bodies[object[:path]] = object_string
        end
        max_depth = max_depth - 1
      end
    end

    def create_internal_row
      working_row = []

      @human_readable_tags.each_with_index do |value, index|
        #puts "#{@internal_use_tags[index]} -- #{@json_nodes[value.to_sym]}"
        working_row << (@json_nodes[value.to_sym].to_s << '\0')
      end

      return working_row
    end

    def assemble_json(result_set, expected_mappings)
      @sql_columns = result_set.columns

      broker = Superstudio::JsonBroker.new()
      broker.set_mapper(@internal_use_tags)
      broker.set_row_count(result_set.count)
      broker.set_quotes(@quoted_tags)
      broker.set_depths(@depth_tags, @real_depth_tags)
      broker.set_hashing(@do_not_hash)
      # We need to have map_row know what the current row is without passing it in
      # Use @row_being_used for that, piggyback off that for broker consuming that row
      result_set.rows.each do |row|
        @row_being_used = row
        @json_nodes = {}
        map_row

        working_row = create_internal_row()
        broker.consume_row(working_row)
      end

      @json_result = broker.finalize_json
    end

  end
end
