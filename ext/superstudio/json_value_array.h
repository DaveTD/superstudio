#ifndef ARRAY_JSON_VALUE_
#define ARRAY_JSON_VALUE_

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "json_builder.h"

void add_to_array(
  JSONDocumentBuilder* builder,
  uint64_t hash,
  char* string_value,
  unsigned long string_size,
  char* column_name,
  unsigned long column_name_length,
  unsigned long repeatable,
  unsigned long quoted
  );

void initialize_value_item(JSONDocumentBuilder* builder, ArrayValueListItem *target, unsigned long value_characters, char* array_value, unsigned long quoted);
unsigned long finalize_value_array(JSONDocumentBuilder* builder, ArrayValueJSON* value_arrays, unsigned long counter);

#endif