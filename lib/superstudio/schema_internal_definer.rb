module SchemaInternalDefiner
  def set_human_to_internal_mappings(expected_mappings)
    fork_nodes = expected_mappings.uniq { |i| i[:path] }
    max_depth = expected_mappings.max_by { |x| x[:depth] }[:depth]
    internal_fork_numbers = convert_fork_paths_to_base_route_numbers(fork_nodes, max_depth)
    depth_counter = 0
    while depth_counter <= max_depth
      # Assign all internal numbers to their respective human-readable forms
      objects_at_depth = expected_mappings.select { |j| j[:depth] == depth_counter }
      forks_at_depth = internal_fork_numbers.select { |k,v| v[:depth] == depth_counter }
      type_5_already_found, type_3_already_found = [], []
      type_1_count = 0

      objects_at_depth.each do |candidate|
        # ex. {depth: 1, path: ["root"], name: "id", node_type: "integer"}
        forks_at_depth.each do |p|
          if candidate[:path] == p[0]
            # Add the internal path to the internal mapping
            if @type_5_paths.include?(p[0])
              handle_array_stop_path(type_5_already_found, p[0], p[1][:internal_path], "#{candidate[:path].join("_B_")}_A_#{candidate[:name]}")
            elsif @type_3_paths.include?(p[0])
              handle_array_stop_path(type_3_already_found, p[0], p[1][:internal_path], "#{candidate[:path].join("_B_")}_A_#{candidate[:name]}")
            else
              type_1_count += 1
              add_to_describe_arrays("#{candidate[:path].join("_B_")}_P_#{candidate[:name]}", p[1][:internal_path], "1.#{type_1_count}", candidate[:node_type], candidate[:depth])
            end
          end
        end
        # Unless there are no forks for this depth, then we're at the root node, and we should do this differently
        if !forks_at_depth.present?
          # Check the type, increment, send
          if @type_5_paths.include?(p[0])
            handle_array_stop_path(type_5_already_found, p[0], p[1][:internal_path], "#{candidate[:path].join("_B_")}_A_#{candidate[:name]}")
          elsif @type_3_paths.include?(p[0])
            handle_array_stop_path(type_3_already_found, p[0], p[1][:internal_path], "#{candidate[:path].join("_B_")}_A_#{candidate[:name]}")
          else
            type_1_count += 1
            add_to_describe_arrays("#{candidate[:path].join("_B_")}_P_#{candidate[:name]}", "", "1.#{type_1_count}", candidate[:node_type], candidate[:depth])
          end
        end
      end
      depth_counter += 1
    end
  end

  def handle_array_stop_path(type_already_found, path, internal_use_tag, human_readable_tag)
    if type_already_found.include?(path)
      # Do nothing.
    else
      type_already_found << path
      @internal_use_tags << internal_use_tag
      @human_readable_tags << human_readable_tag
    end
  end

  def add_to_describe_arrays(human_route, parent_path, item_path, node_type, depth)
    @human_readable_tags << human_route
    @depth_tags << depth
    item_string = item_path

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
      @internal_use_tags << item_string
      @real_depth_tags << 0
    end

    if item_string.chr == '3'
      @do_not_hash << 1
    else
      @do_not_hash << 0
    end
  end

  def convert_fork_paths_to_base_route_numbers(fork_nodes, max_depth)
    depth_counter = 0
    internal_fork_numbers = {}

    while depth_counter <= max_depth
      two_at_depth_counter, three_at_depth_counter, four_at_depth_counter, five_at_depth_counter = 0, 0, 0, 0
      objects_at_depth = fork_nodes.select {|j| j[:depth] == depth_counter}
      possible_parents = fork_nodes.select {|j| j[:depth] == (depth_counter - 1)}
      
      objects_at_depth.each do |o|
        if o[:path].join("_B_") == 'root'
          internal_fork_numbers[o[:path]] = { internal_path: '', depth: o[:depth] }
          break
        end
        if @type_2_paths.include?(o[:path])
          two_at_depth_counter += 1
          generate_internal_fork_number_of_type(internal_fork_numbers, possible_parents, o, 2, two_at_depth_counter) 
        end

        if @type_3_paths.include?(o[:path])
          three_at_depth_counter += 1
          generate_internal_fork_number_of_type(internal_fork_numbers, possible_parents, o, 3, three_at_depth_counter)
        end

        if @type_4_paths.include?(o[:path])
          four_at_depth_counter += 1
          generate_internal_fork_number_of_type(internal_fork_numbers, possible_parents, o, 4, four_at_depth_counter)
        end

        if @type_5_paths.include?(o[:path])
          five_at_depth_counter += 1
          generate_internal_fork_number_of_type(internal_fork_numbers, possible_parents, o, 5, five_at_depth_counter)
        end
      end

      depth_counter += 1
    end
    return internal_fork_numbers
  end

  def generate_internal_fork_number_of_type(internal_fork_numbers, possible_parents, object_at_depth, type, number)
    parent = nil
    if possible_parents
      possible_parents.each do |pp|
        parent = pp if object_at_depth[:path][0..-2] = pp[:path]
      end
    end

    internal_path = ""
    internal_path = "#{internal_fork_numbers[parent[:path]][:internal_path]}" if parent.present?
    if parent.present? && parent[:path].join("_B_") != 'root'
      internal_path << "-" # if parent.present? && parent[:path] != 'root'
    end
    internal_path << "#{type}.#{number}"
    internal_fork_numbers[object_at_depth[:path]] = { internal_path: internal_path, depth: object_at_depth[:depth] }
  end
end