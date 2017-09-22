#ifndef JSON_OBJECT_ARRAY_
#define JSON_OBJECT_ARRAY_

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "json_builder.h"
#include "hash_linked_list.h"
#include "finalize.h"

void create_array_object(JSONDocumentBuilder* builder, 
  uint64_t hash, 
  HashList* parent_list,
  JSONLevelBuilder* parent_level_definitions,  
  JSONLevelBuilder* empty_child_object, 
  JSONLevelBuilder* empty_child_array,
  int identifier_int);
void count_increment_or_create_json_level_child_array(JSONDocumentBuilder* builder, JSONLevelBuilder* child_array_levels, int identifier_length, char* identifier, int identifier_int, uint64_t parent_hash, uint64_t child_hash);
void initialize_child_array_levels(JSONLevelBuilder* child_array_levels, SSMemoryStack* memory_stack);
void set_array_object_child_level_definitions(JSONLevelBuilder* level_definitions, JSONLevelBuilder* array_object_info_list, uint64_t hash, unsigned long column_count, unsigned long accessing_depth);
void assign_array_object_data(JSONLevelBuilder* level_definitions, JSONLevelBuilder* array_object_info_list, char** row_strings, unsigned long* string_sizes, unsigned long column_count, unsigned long accessing_depth);
void add_or_find_array_object_child_hashes(
  JSONDocumentBuilder* builder,
  JSONLevelBuilder* level_definitions,
  JSONLevelBuilder* child_array_levels, 
  unsigned long accessing_depth,
  unsigned long visible_depth,
  unsigned long column_count,
  unsigned long* string_sizes,
  char** row_strings,
  JSONObject* related_object
  );

#endif