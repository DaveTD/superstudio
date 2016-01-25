require 'rails/generators'
require 'rails/generators/base'

module Superstudio
  module Generators
    class SchemaMapGenerator < Rails::Generators::NamedBase
      source_root File.expand_path("../../templates", __FILE__)

      desc "Creates a simple json schema mapping class, inferring required json nodes and required columns from a json schema (draft v4)."

      def create_map_file
        file_name = name
        file_name = file_name << ".json.schema" unless name.end_with?(".json.schema")
        class_name = file_name.chomp(".json.schema")

        interpreted_schema = Superstudio::SqlJsonBuilder.new(nil, file_name)

        file_data = %Q(=begin
Interpreted schema has the following data bodies. Multiple bodies indicates array nesting - non-included node names should not be included in @json_nodes because they are placeholders for array inserts.
)

        interpreted_hashes = []

        interpreted_schema.template_bodies.each do |key, template|
          inter = JSON.parse("{" << template.gsub("%", "").slice(1..template.length).chomp("}").gsub('{', '"').gsub('}', '"') << "}")
          temp_string = PP.pp(inter, '')
          interpreted_hashes << inter
          file_data << %Q(
#{temp_string}
            )
        end
        file_data << %Q(
=end)

        file_data << %Q(
class #{class_name}Mapper < Superstudio::SqlJsonBuilder
  def map_row)
        interpreted_hashes.each do |hash|
          hash.each do |name, node|
            file_data << %Q(
    @json_nodes[:#{node}] = value_by_column_name("#{name}"))
          end
        end

        file_data << %Q(
  end
end)
        create_file "app/json_schemas/#{class_name}.rb", file_data
      end
    end
  end
end
