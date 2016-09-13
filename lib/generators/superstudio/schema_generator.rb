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

        file_data = template_header
        model_columns.each do |name, type|
          file_data << data_column(name, type)
        end

        file_data = file_data.chomp(",")
        file_data << template_footer

        model_klass_name = model_klass.name.gsub(":", "").underscore
        create_file "app/json_schemas/#{model_klass_name}.json.schema", file_data
      end

      private
      def template_header
      %Q({
  "$schema": "http://json-schema.org/draft-04/schema#",
  "type": "object",
  "properties": {)
      end

      def data_column(name, type)
      %Q(
    "#{name}": {
      "type": "#{type}"
    },)
      end

      def template_footer
      %Q(},
  "required": ["id"]
})
      end
    end
  end
end
