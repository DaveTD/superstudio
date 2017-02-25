#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include "json_builder.h"
#include "hash_linked_list.h"

void hl_initialize(HashList *list, unsigned long query_rows)
{
  long i;
  long bucket_count = ((long)(query_rows / (log10(query_rows)/log10(2))) + 1);
  list->length = 0;
  list->bucket_interval = UINT64_MAX / bucket_count;
  list->next = NULL;
  list->last = NULL;
  list->buckets = malloc(bucket_count * sizeof(HashListNode*));
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

HashListNode* hl_insert_or_find(HashList *list, uint64_t passed_hash, JSONObject* related_object, JSONLevelBuilder* related_single_json_info)
{
  HashListNode* found_node;
  int bucket_number = find_target_bucket(passed_hash, list->bucket_interval);

  if (!list->next) {
    HashListNode *new_node = (HashListNode*)malloc(sizeof(HashListNode));

    new_node->hash = passed_hash;
    new_node->next = NULL;
    new_node->bucket_next = NULL;
    new_node->bucket_previous = NULL;
    new_node->related_JSON_object = related_object;
    new_node->single_object_info_list = related_single_json_info;
    list->last = list->next = list->buckets[bucket_number] = new_node;

    increment_length(list);
    return list->next;
  }
  found_node = hl_find_node(list, passed_hash);
  if (found_node) {
    if (found_node->bucket_previous != NULL) {
      found_node->bucket_previous->bucket_next = found_node->bucket_next;
      found_node->bucket_next = list->buckets[bucket_number];
      list->buckets[bucket_number] = found_node;
    }
    return found_node;
  } else {
    HashListNode *new_node = (HashListNode*)malloc(sizeof(HashListNode));
    new_node->hash = passed_hash;
    new_node->next = NULL;
    new_node->bucket_next = NULL;
    new_node->bucket_previous = NULL;
    new_node->related_JSON_object = related_object;
    new_node->single_object_info_list = related_single_json_info;
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

HashListNode* find_or_create_node(
  JSONDocumentBuilder* builder, 
  JSONLevelBuilder* level_definitions, 
  JSONLevelBuilder* single_object_children, 
  JSONObject* parent_object, 
  uint64_t hash, 
  uint64_t parent_hash, 
  unsigned char parent_type,
  char** row_strings,
  unsigned long* string_sizes,
  unsigned long visible_depth
  )
{
  HashListNode* found_node;

  found_node = hl_find_node(level_definitions->search_list, hash);
  if (found_node == NULL) {
    builder->json_char_count += 3; // Starting, ending brace and comma

    if (parent_type == 4) {
      create_array_object(builder, hash, parent_hash, level_definitions);
    } else {
      create_single_object(builder, hash, parent_hash, level_definitions->identifier_int, single_object_children, level_definitions, parent_object);
    }
    set_key_values(builder, string_sizes, row_strings, hash, level_definitions, visible_depth, level_definitions);
    found_node = hl_find_node(level_definitions->search_list, hash);
  }

  return found_node;
}

void hl_set_json_object(HashListNode* node, JSONObject* related_object)
{
  node->related_JSON_object = related_object;
}

int find_target_bucket(uint64_t passed_hash, unsigned long interval)
{
  return passed_hash / interval;
}
