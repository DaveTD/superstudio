module Superstudio
  module SchemaInterpreter

    # TODO: Refactor this, it's only being used in the generator and it's out of step with SchemaInternalDefiner.set_human_to_internal_mappings
    def create_template(expected_mappings)
      fork_nodes = expected_mappings.uniq { |i| i[:path] }
      max_depth = expected_mappings.max_by { |x| x[:depth] }[:depth]

      @template_bodies = {}
      mappings = set_human_to_internal_mappings(expected_mappings)

      while max_depth != 0
        objects_at_depth = fork_nodes.select { |j| j[:depth] == max_depth}
        at_depth_hash = {}

        objects_at_depth.each do |object|
          object_path_string = object[:path].join("_B_")
          object_properties = expected_mappings.select { |mappings| mappings[:path] == object[:path] }
          object_string = "{"

          if @type_3_paths.include?(object[:path]) || @type_5_paths.include?(object[:path])
            object_properties.each do |property, index|
              expected_hash_key = "#{object_path_string}_A_#{property[:name]}"
              object_string << "\"#{property[:name]}\":{#{expected_hash_key}},"
            end
          else
            object_properties.each do |property, index|
              expected_hash_key = "#{object_path_string}_P_#{property[:name]}"
              object_string << "\"#{property[:name]}\":{#{expected_hash_key}},"
            end
          end
          object_string = object_string.chomp(",")
          object_string << "}"

          @template_bodies[object[:path]] = object_string
        end

        max_depth = max_depth - 1
      end

    end
    # TODO: Refactor this, it's only being used in the generator and it's out of step with SchemaInternalDefiner.set_human_to_internal_mappings

    

    def interpret_json_schema(json_hash, depth = 0, path_array = [], expected_mappings = [], node_name = 'root')
      level_keys = json_hash.keys
      level_keys.each do |key|

        if key == "type"
          if ["integer", "boolean", "number", "string"].include?(json_hash[key])
            return {depth: depth, path: path_array, name: node_name, node_type: json_hash[key]}
          end
        end

        if key == "uniqueItems"
          # if this is set to true, record the path so that we can match the type 3s against it to know where to check for uniqueness
          # default is false
          if json_hash[key].to_s == "true"
            new_path_array  = path_array.dup
            new_path_array << node_name
            @unique_threes_paths << new_path_array
          end
        end

        # byebug
        if json_hash[key].is_a?(Hash)
          new_path_array = path_array.dup
          new_depth = depth
          # byebug
          if node_name != "properties" && node_name != "items" && node_name != "dependencies"
            new_path_array << node_name
            new_depth = new_depth + 1
            # we need something in here for viewed and real depths
            if json_hash[key]["type"] != "array"  && key != "items"
              @type_2_paths << new_path_array
              # @type_2_indicator_names << node_name
            end
          end
          # if node_name == "properties" && json_hash[key]["type"] == "array"
          #   # return { depth: new_depth, path: new_path_array << key, name: node_name, node_type: "array" }
          #   # expected_mappings << interpret_json_schema(json_hash[key], new_depth + 1, new_path_array << key, [], key)
          #   alt_path = new_path_array.dup
          #   alt_path << key
          #   expected_mappings << { depth: new_depth + 1, path: alt_path, name: node_name, node_type: "array" }

          # end
            expected_mappings << interpret_json_schema(json_hash[key], new_depth, new_path_array, [], key)
          # end
        end

        if key == "items"
          if json_hash[key].is_a?(Array)
            # Something like: { "type": "array", "items": [ { "type": "number" }, { "type": "string" } ] }
            # What gets passed in has to be a string in exactly the format the user wants it in
            # This is a type 5 path
            new_depth = depth + 1
            new_path_array = path_array.dup
            new_path_array << node_name
            @type_5_paths << new_path_array  

            return { depth: depth, path: new_path_array, name: node_name, node_type: "custom_array" }
          else
            # detect if this is an array of objects or values
            # if path_array.last != "root"
            #   expected_mappings << { depth: depth, path: path_array, name: path_array.last, node_type: "array" }
            # end
            if json_hash[key]["type"].downcase == "object"
              # Something like: { "type": "array", "items": { "type": "object", "properties" : { <stuff...> }}}
              # don't think we need the new path, look at later
              new_depth = new_depth + 1
              @type_4_paths << new_path_array
              # @type_4_indicator_names << node_name
            else
              # this is an array of values
              # Something like: { "type": "array", "items": { "type: "number" } }

              @type_3_paths << new_path_array
            end
          end
        end

      end
      return expected_mappings.flatten
    end
  end
end