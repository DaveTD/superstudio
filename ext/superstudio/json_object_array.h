#ifndef JSON_OBJECT_ARRAY_
#define JSON_OBJECT_ARRAY_

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "json_builder.h"

void create_array_object(JSONDocumentBuilder* builder, uint64_t hash, uint64_t parent_hash, JSONLevelBuilder* parent_level);
unsigned long finalize_object_array(JSONDocumentBuilder* builder, ArrayObjectJSON* object_array, unsigned long counter);

#endif