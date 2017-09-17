#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include "hash_linked_list.h"

void hl_initialize(HashList *list, unsigned long query_rows)
{
  long i;
  long bucket_count = ((long)((query_rows + 1) / (log10(query_rows + 1)/log10(2))) + 1);
  list->length = 0;
  list->bucket_interval = UINT64_MAX / bucket_count;
  list->next = NULL;
  list->last = NULL;
  list->buckets = calloc(1, bucket_count * sizeof(HashListNode*));

  for (i=0; i<bucket_count; i++) {
    *(list->buckets + i) = NULL;
  }
}

void print_list_details(HashList *list)
{
  printf("Length: %i\n", list->length);
  HashListNode *i;
  i = list->next;
  while(i) {
    printf("Hash: %lu\n", i->hash);
    i = i->next;
  }
}

void increment_length(HashList *list)
{
  int tmp_length;
  tmp_length = list->length;
  list->length = ++tmp_length;
  return;
}

uint64_t hl_first(HashList *list)
{
  HashListNode first_node;
  first_node = *(list->next);
  return first_node.hash;
}

HashListNode* hl_insert_or_find(HashList *list, 
  uint64_t passed_hash, 
  JSONObject* related_object,
  JSONLevelBuilder* related_parent_level, 
  JSONLevelBuilder* related_single_json_info,
  JSONLevelBuilder* related_object_info_list
  )
{
  HashListNode* found_node;
  int bucket_number = find_target_bucket(passed_hash, list->bucket_interval);

  if (!list->next) {
    HashListNode *new_node = (HashListNode*)calloc(1, sizeof(HashListNode));

    new_node->hash = passed_hash;
    new_node->next = NULL;
    new_node->bucket_next = NULL;
    new_node->bucket_previous = NULL;
    new_node->related_JSON_object = related_object;
    new_node->related_parent_level = related_parent_level;
    new_node->single_object_info_list = related_single_json_info;
    new_node->array_object_info_list = related_object_info_list;

    list->last = list->next = list->buckets[bucket_number] = new_node;
    increment_length(list);
    return list->next;
  }
  found_node = hl_find_node(list, passed_hash);
  if (found_node) {
    if (found_node->bucket_previous != NULL) {
      found_node->bucket_previous->bucket_next = found_node->bucket_next;
      found_node->bucket_next = list->buckets[bucket_number];

      found_node->bucket_previous = NULL;

      list->buckets[bucket_number] = found_node;
    }
    return found_node;
  } else {
    HashListNode *new_node = (HashListNode*)calloc(1, sizeof(HashListNode));
    new_node->hash = passed_hash;
    new_node->next = NULL;
    new_node->bucket_next = NULL;
    new_node->bucket_previous = NULL;
    new_node->related_JSON_object = related_object;
    new_node->related_parent_level = related_parent_level;
    new_node->single_object_info_list = related_single_json_info;
    new_node->array_object_info_list = related_object_info_list;

    if (list->buckets[bucket_number] != NULL) {
      list->buckets[bucket_number]->bucket_previous = new_node;
      new_node->bucket_next = list->buckets[bucket_number];
    }
    list->buckets[bucket_number] = new_node;

    increment_length(list);

    list->last->next = new_node;
    list->last = new_node;

    return list->last;
  }
}

HashListNode* hl_find_node(HashList *list, uint64_t passed_hash)
{
  int bucket_number = find_target_bucket(passed_hash, list->bucket_interval);
  HashListNode *i = list->buckets[bucket_number];

  if (i == NULL) {
    return NULL;
  }
  while(i) {
    if (i->hash == passed_hash) {
      return i;
    }
    i = i->bucket_next;
  }
  return NULL;
}

JSONObject* find_or_create_node(
  JSONDocumentBuilder* builder, 
  JSONLevelBuilder* level_definitions, 
  JSONLevelBuilder* child_levels,
  JSONLevelBuilder* child_array_start,
  JSONObject* parent_object, 
  uint64_t hash, 
  uint64_t parent_hash, 
  unsigned char parent_type,
  char** row_strings,
  unsigned long* string_sizes,
  unsigned long visible_depth,
  HashList* parent_search_list
  )
{
  HashListNode* found_node;
  JSONObject* found_object = NULL;
  JSONLevelBuilder* found_level_definitions;
  SingleObjectJSON* related_child_objects;
  SingleObjectJSON* single_parent_object; // We need to search in two directions - through each child and through each child's single object children
  int found = 0;

  builder->json_char_count += 3; // Starting, ending brace and comma

  // Dev note:
  // The create calls will handle duplicates where additional stuff doesn't need to be added
  if (parent_type == 4) {

    create_array_object(builder, 
      hash, 
      parent_hash, 
      parent_search_list, 
      level_definitions, 
      child_levels, 
      child_array_start, 
      level_definitions->identifier_int);
    
    found_node = hl_find_node(parent_search_list, hash);
    if (found_node) {
      found_object = found_node->related_JSON_object;
      found_level_definitions = found_node->related_parent_level;
    }
  } else {

    create_single_object(builder, 
      hash, 
      parent_hash, 
      level_definitions->identifier_int, 
      child_levels, 
      child_array_start, 
      level_definitions, 
      parent_search_list,
      parent_object);  

    // Dev note:
    // When adding a single object, we're finding the parent hash because the single object is hashed as well
    // Single objects are a 1-1 relationship

    related_child_objects = parent_object->single_objects;
    while (related_child_objects->set_flag && !found) {
      single_parent_object = related_child_objects;
      if (single_parent_object->identifier_int == level_definitions->identifier_int) {
        found = 1;
      } else {
        single_parent_object = single_parent_object->value->single_objects;
        while (single_parent_object->set_flag && !found) {
          if (single_parent_object->identifier_int == level_definitions->identifier_int) {
            found = 1;
          } else {
            single_parent_object = single_parent_object->next_item;
          }
        }
      }
      related_child_objects = related_child_objects->next_item;
    }

    if (found) {
      found_object = single_parent_object->value;
    } else {
      found_object = parent_object;
    }
    found_level_definitions = level_definitions;
  }

  if (found_object) {
    set_key_values(builder, string_sizes, row_strings, hash, visible_depth, found_level_definitions, found_object);
    set_value_arrays(builder, found_level_definitions, hash, found_level_definitions->column_count, row_strings, string_sizes, visible_depth, found_object);
  }
  
  return found_object;
}

void hl_set_json_object(HashListNode* node, JSONObject* related_object)
{
  node->related_JSON_object = related_object;
}

int find_target_bucket(uint64_t passed_hash, unsigned long interval)
{
  return passed_hash / interval;
}
