module SchemaInterpreter

  # TODO: Refactor this, it's only being used in the generator and it's out of step with SchemaInternalDefiner.set_human_to_internal_mappings
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
  # TODO: Refactor this, it's only being used in the generator and it's out of step with SchemaInternalDefiner.set_human_to_internal_mappings

  

  def interpret_json_schema(json_hash, depth = 0, path_array = [], expected_mappings = [], node_name = 'root')
    level_keys = json_hash.keys
    level_keys.each do |key|
      # need to put in a big set of ignore keys here, otherwise they might mess up how everything else works
      # if IGNORE_SET.contains?(key)
      #   return
      # end

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
          # we need something in here for viewed and real depths
          if json_hash[key]["type"] != "array" && key != "items"
            @type_2_paths << new_path_array
          end
        end
        expected_mappings << interpret_json_schema(json_hash[key], new_depth, new_path_array, [], key)
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
          if json_hash[key]["type"].downcase == "object"
            # Something like: { "type": "array", "items": { "type": "object", "properties" : { <stuff...> }}}
            # don't think we need the new path, look at later
            new_depth = new_depth + 1
            @type_4_paths << new_path_array
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