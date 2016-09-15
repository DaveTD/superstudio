#include <jsonbroker.h>

void Init_jsonbroker()
{
  VALUE cJsonBroker = rb_define_class_under(mSuperstudio, "JsonBroker", rb_cObject);

  rb_define_alloc_func(cJsonBroker, json_broker_allocate);
  rb_define_method(cJsonBroker, "set_mapper", json_broker_set_mapper, 1);
  rb_define_method(cJsonBroker, "set_row_count", json_broker_set_row_count, 1);
  rb_define_method(cJsonBroker, "set_quotes", json_broker_set_quotes, 1);
  rb_define_method(cJsonBroker, "set_hashing", json_broker_set_hashing, 1);
  rb_define_method(cJsonBroker, "set_depths", json_broker_set_depths, 2);
  rb_define_method(cJsonBroker, "get_row_count", json_broker_get_row_count, 0);
  rb_define_method(cJsonBroker, "get_column_count", json_broker_get_mapper_length, 0);
  rb_define_method(cJsonBroker, "consume_row", json_broker_consume_row, 1);
  rb_define_method(cJsonBroker, "finalize_json", json_broker_finalize_json, 0);
}

static VALUE json_broker_allocate(VALUE klass)
{
  JSONBuilder *builder = malloc(sizeof(JSONBuilder));
  json_builder_initialize(builder);
  return Data_Wrap_Struct(klass, NULL, deallocate_broker, builder);
}

static VALUE json_broker_set_mapper(VALUE self, VALUE tags)
{
  Check_Type(tags, T_ARRAY);
  JSONBuilder *builder;
  Data_Get_Struct(self, JSONBuilder, builder);

  unsigned long length = RARRAY_LENINT(tags);
  set_column_count(builder, length);

  struct RArray* cTags = RARRAY(tags);
  VALUE* tag_pointer = RARRAY_PTR(cTags);
  unsigned long counter = 0;

  struct RString* in_loop_rstring;
  char* in_loop_string;
  char* mapping_array[length];

  while(counter < length)
  {
    Check_Type(tag_pointer[counter], T_STRING);
    in_loop_rstring = RSTRING(tag_pointer[counter]);
    in_loop_string = RSTRING_PTR(in_loop_rstring);
    mapping_array[counter] = in_loop_string;
    counter++;
  }
  set_mapping_array(builder, mapping_array);
  return Qnil;
}

static VALUE json_broker_set_quotes(VALUE self, VALUE quotes)
{
  Check_Type(quotes, T_ARRAY);
  JSONBuilder *builder;
  Data_Get_Struct(self, JSONBuilder, builder);

  unsigned long length = get_column_count(builder);
  struct RArray* cQuotes = RARRAY(quotes);
  VALUE* quotes_pointer = RARRAY_PTR(cQuotes);
  unsigned long quotes_array[length];
  unsigned long counter = 0;

  while(counter < length)
  {
    quotes_array[counter] = FIX2LONG(quotes_pointer[counter]);
    counter++;
  }
  set_quote_array(builder, quotes_array);
  return Qnil;
}

static VALUE json_broker_set_hashing(VALUE self, VALUE do_not_hash)
{
  Check_Type(do_not_hash, T_ARRAY);
  JSONBuilder *builder;
  Data_Get_Struct(self, JSONBuilder, builder);

  unsigned long length = get_column_count(builder);
  struct RArray* cDoNotHash = RARRAY(do_not_hash);
  VALUE* hashing_pointer = RARRAY_PTR(cDoNotHash);
  unsigned long hashing_array[length];
  unsigned long counter = 0;

  while(counter < length)
  {
    hashing_array[counter] = FIX2LONG(hashing_pointer[counter]);
    counter++;
  }
  set_hashing_array(builder, hashing_array);
  return Qnil;
}

static VALUE json_broker_set_depths(VALUE self, VALUE depths, VALUE real_depths)
{
  Check_Type(depths, T_ARRAY);
  JSONBuilder *builder;
  Data_Get_Struct(self, JSONBuilder, builder);

  unsigned long length = get_column_count(builder);
  struct RArray* cDepths = RARRAY(depths);
  struct RArray* cRealDepths = RARRAY(real_depths);
  VALUE* depths_pointer = RARRAY_PTR(cDepths);
  VALUE* real_depths_pointer = RARRAY_PTR(cRealDepths);

  unsigned long depths_array[length];
  unsigned long real_depths_array[length];
  unsigned long counter = 0;
  unsigned long max_depth = 0;
  unsigned long max_real_depth = 0;

  while(counter < length)
  {
    depths_array[counter] = FIX2LONG(depths_pointer[counter]);
    real_depths_array[counter] = FIX2LONG(real_depths_pointer[counter]);
    if(depths_array[counter] > max_depth)
    {
      max_depth = depths_array[counter];
    }
    if(real_depths_array[counter] > max_real_depth)
    {
      max_real_depth = real_depths_array[counter];
    }
    counter++;
  }
  set_depth_array(builder, depths_array, max_depth, real_depths_array, max_real_depth);
  return Qnil;
}

static VALUE json_broker_get_mapper_length(VALUE self)
{
  JSONBuilder *builder;
  Data_Get_Struct(self, JSONBuilder, builder);
  unsigned long get_count = get_column_count(builder);
  VALUE count = LONG2FIX(get_count);
  return count;
}

static VALUE json_broker_set_row_count(VALUE self, VALUE row_count)
{
  JSONBuilder *builder;
  Data_Get_Struct(self, JSONBuilder, builder);
  unsigned long set_count = FIX2LONG(row_count);
  set_row_count(builder, set_count);
  return Qnil;
}

static VALUE json_broker_get_row_count(VALUE self)
{
  JSONBuilder *builder;
  Data_Get_Struct(self, JSONBuilder, builder);
  unsigned long get_count = get_row_count(builder);
  VALUE count = LONG2FIX(get_count);
  return count;
}

static VALUE json_broker_consume_row(VALUE self, VALUE row)
{
  Check_Type(row, T_ARRAY);
  JSONBuilder *builder;
  Data_Get_Struct(self, JSONBuilder, builder);
  struct RArray* cRow = RARRAY(row);
  VALUE* row_pointer = RARRAY_PTR(cRow);

  unsigned long length = RARRAY_LENINT(row);
  unsigned long counter = 0;
  struct RString* in_loop_rstring;
  char* in_loop_string;
  char* row_strings[length];
  unsigned long string_sizes[length];

  while(counter < length)
  {
    Check_Type(row_pointer[counter], T_STRING);
    in_loop_rstring = RSTRING(row_pointer[counter]);
    in_loop_string = RSTRING_PTR(in_loop_rstring);

    //printf("Found value: %s\n", in_loop_string);
    row_strings[counter] = in_loop_string;
    string_sizes[counter] = RSTRING_LEN(in_loop_rstring);
    counter++;
  }
  consume_row(builder, row_strings, string_sizes, 0, NULL);
  return Qnil;
}

static VALUE json_broker_finalize_json(VALUE self)
{
  JSONBuilder *builder;
  Data_Get_Struct(self, JSONBuilder, builder);
  char* final_json = finalize_json(builder);
  return rb_str_new2(final_json);
}

static void deallocate_broker(void * broker)
{
  free(broker);
}