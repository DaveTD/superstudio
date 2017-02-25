#ifndef HASH_LIST_
#define HASH_LIST_

#include <stdint.h>

typedef struct HashList
{
  int length;
  unsigned long bucket_interval;
  struct HashListNode* next;
  struct HashListNode* last;
  struct HashListNode** buckets;
} HashList;

typedef struct HashListNode
{
  uint64_t hash;
  struct HashListNode* next;
  struct HashListNode* bucket_next;
  struct HashListNode* bucket_previous;
  struct JSONObject* related_JSON_object;
  struct JSONLevelBuilder* single_object_info_list;
} HashListNode;

void hl_initialize(HashList *list, unsigned long query_rows);
void print_list_details(HashList *list);
void increment_length(HashList *list);
uint64_t hl_first(HashList *list);
int find_target_bucket(uint64_t passed_hash, unsigned long interval);
void hl_set_json_object(HashListNode* node, JSONObject* related_object);

HashListNode* find_or_create_node(JSONDocumentBuilder* builder, JSONLevelBuilder* level_definitions, JSONLevelBuilder* single_object_children, JSONObject* parent_object, uint64_t hash, uint64_t parent_hash, unsigned char parent_type, char** row_strings, unsigned long* string_sizes, unsigned long visible_depth);
HashListNode* hl_insert_or_find(HashList *list, uint64_t passed_hash, JSONObject* related_object, JSONLevelBuilder* related_single_json_info);
HashListNode* hl_find_node(HashList *list, uint64_t passed_hash);

#endif // HASH_LIST_
