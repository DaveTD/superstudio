#ifndef JSON_VALUE_
#define JSON_VALUE_

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "json_builder.h"

void set_key_values(JSONDocumentBuilder* builder, unsigned long* string_sizes, char** row, unsigned long hash, JSONLevelBuilder* level_definitions, unsigned long visible_depth, JSONLevelBuilder* parent_level);
unsigned long finalize_key_values(JSONDocumentBuilder* builder, SingleValueJSON* single_values, JSONObject* parent_json, unsigned long counter);

#endif