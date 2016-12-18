#ifndef JSON_VALUE_
#define JSON_VALUE_

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "json_builder.h"

void set_key_values(JSONDocumentBuilder* builder, unsigned long* string_sizes, char** row, unsigned long target_hash, unsigned char at_depth_count);
unsigned long finalize_key_values(JSONDocumentBuilder* builder, SingleValueJSON* single_values, ArrayObjectListItem* item, unsigned long counter);

#endif