require 'superstudio/superstudio'
require 'superstudio/schema_interpreter'
require 'superstudio/schema_reader'
require 'superstudio/schema_internal_definer'

module Superstudio
  class SqlJsonBuilder
    include Superstudio::SchemaReader
    include Superstudio::SchemaInterpreter
    include Superstudio::SchemaInternalDefiner

    attr_accessor :sql_columns, :json_result, :schema, :template_types, :array_paths, :json_nodes, :required_columns, :human_to_internal, :template_bodies

    def initialize(query, file_name = nil)
      file_class_name = self.class.name
      file_class_name.slice!("Mapper")
      file_name ||= file_class_name.underscore << ".json.schema"

      @schema = read_schema(file_name)

      @sql_columns, @row_being_used = [], []
      @json_result = ""
      @json_nodes, @required_columns = {}, {}

      @unique_threes_tags, @human_readable_tags, @internal_use_tags, @quoted_tags, @do_not_hash, @depth_tags, @real_depth_tags = [], [], [], [], [], [], []
      @type_2_paths, @type_3_paths, @type_4_paths, @type_5_paths = [], [], [], []
      @unique_threes_paths = []

      json_schema_interpretation = interpret_json_schema(@schema)

      if query.present?
        result_set = get_sql_results(query)
        # create_template(json_schema_interpretation)
        set_human_to_internal_mappings(json_schema_interpretation)
        assemble_json(result_set)
      else
        create_template(json_schema_interpretation)
      end
    end

    private
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

    def create_internal_row
      working_row = []

      @human_readable_tags.each_with_index do |value, index|
        working_row << (@json_nodes[value.to_sym].to_s)
      end

      return working_row
    end

    def assemble_json(result_set)
      @sql_columns = result_set.columns

      broker = Superstudio::JsonBroker.new()
      broker.set_row_count(result_set.count)
      broker.set_mapper(@internal_use_tags)
      broker.set_quotes(@quoted_tags)
      broker.set_depths(@depth_tags, @real_depth_tags)
      broker.set_hashing(@do_not_hash)
      broker.set_column_names(@sql_columns)
      broker.set_repeating_arrays(@unique_threes_tags)
      
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