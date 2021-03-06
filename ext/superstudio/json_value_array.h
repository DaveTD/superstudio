#ifndef ARRAY_JSON_VALUE_
#define ARRAY_JSON_VALUE_

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "json_builder.h"
#include "finalize.h"

void add_to_array(
  JSONDocumentBuilder* builder,
  char* string_value,
  unsigned long string_size,
  char* column_name,
  unsigned long column_name_length,
  unsigned long repeatable,
  unsigned long quoted,
  JSONObject* parent_object
  );
void set_value_arrays(JSONDocumentBuilder* builder, JSONLevelBuilder* level_definitions, unsigned long column_count, char** row_strings, unsigned long* string_sizes, unsigned long accessing_depth, JSONObject* parent_object);
void initialize_value_item(JSONDocumentBuilder* builder, ArrayValueListItem *target, unsigned long value_characters, char* array_value, unsigned long quoted);

#endif