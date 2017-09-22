#ifndef JSON_SINGLE_OBJECT_
#define JSON_SINGLE_OBJECT_

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "json_builder.h"
#include "finalize.h"

void count_increment_or_create_json_level_child(JSONDocumentBuilder* builder, JSONLevelBuilder* child_levels, int identifier_length, char* identifier, int identifier_int, uint64_t parent_hash);
void initialize_child_levels(JSONLevelBuilder* single_object_children, SSMemoryStack* memory_stack);
void create_single_object(JSONDocumentBuilder* builder, 
  uint64_t hash, 
  int identifier_int, 
  JSONObject* parent_object);
void set_single_object_child_level_definitions(JSONLevelBuilder* level_definitions, JSONLevelBuilder* single_object_info_list, uint64_t hash, unsigned long column_count, unsigned long accessing_depth);
void assign_single_object_data(JSONLevelBuilder* level_definitions, JSONLevelBuilder* single_object_info_list, char** row_strings, unsigned long* string_sizes, unsigned long column_count, unsigned long accessing_depth);
void add_or_find_single_object_child_hashes(
  JSONDocumentBuilder* builder,
  JSONLevelBuilder* child_levels,
  JSONObject* related_object,
  JSONObject* parent_object,
  uint64_t hash,
  unsigned char parent_type,
  int parent_identifier
  );



#endif