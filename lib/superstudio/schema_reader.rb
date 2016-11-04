module SchemaReader
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

  def replace_reference_keys(json_hash)
    json_hash.each do |key, value|
      if value.is_a?(Hash)
        json_hash[key] = replace_reference_keys(value)
      end
    end

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

  def read_schema(file_name)
    json_hash = parse_json_schema(file_name)
    json_hash = replace_reference_keys(json_hash)
  end
end