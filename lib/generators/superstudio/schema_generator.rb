require 'rails/generators'
require 'rails/generators/base'

module Superstudio
  module Generators
    class SchemaGenerator < Rails::Generators::NamedBase
      source_root File.expand_path("../../templates", __FILE__)

      desc "Creates a json schema (draft v4) for all the columns in a database table."

      def create_schema_file
        model_klass = name.classify.constantize
        model_columns = {}

        model_klass.columns.each do |column|
          column_type = column.type
          column_type = :string if [:datetime].include? column.type
          column_type = :number if [:decimal].include? column.type

          model_columns[column.name] = column_type
        end

        file_data = %Q({
  "$schema": "http://json-schema.org/draft-04/schema#",
  "title": "Get a patient",
  "type": "object",
  "properties": {)

      model_columns.each do |name, type|
        file_data << %Q(
    "#{name}": {
      "type": "#{type}"
    },)
      end
      file_data = file_data.chomp(",")

      file_data << "
  }
}"
        create_file "app/json_schemas/#{model_klass}.json.schema", file_data
      end
    end
  end
end
