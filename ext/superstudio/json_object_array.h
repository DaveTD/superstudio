#ifndef JSON_OBJECT_ARRAY_
#define JSON_OBJECT_ARRAY_

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "json_builder.h"
#include "hash_linked_list.h"

void create_array_object(JSONDocumentBuilder* builder, 
  uint64_t hash, 
  uint64_t parent_hash, 
  HashList* parent_list,
  JSONLevelBuilder* parent_level_definitions,  
  JSONLevelBuilder* empty_child_object, 
  JSONLevelBuilder* empty_child_array,
  int identifier_int);
void count_increment_or_create_json_level_child_array(JSONDocumentBuilder* builder, JSONLevelBuilder* child_array_levels, int identifier_length, char* identifier, int identifier_int, uint64_t parent_hash, uint64_t child_hash);
void initialize_child_array_levels(JSONLevelBuilder* child_array_levels);
void set_array_object_child_level_definitions(JSONLevelBuilder* level_definitions, JSONLevelBuilder* array_object_info_list, uint64_t hash, unsigned long column_count, unsigned long accessing_depth, char** row_strings, unsigned long* string_sizes);
void assign_array_object_data(JSONLevelBuilder* level_definitions, JSONLevelBuilder* array_object_info_list, char** row_strings, unsigned long* string_sizes, unsigned long column_count, unsigned long accessing_depth);
unsigned long finalize_object_array(JSONDocumentBuilder* builder, ArrayObjectJSON* object_array, unsigned long counter);


#endif