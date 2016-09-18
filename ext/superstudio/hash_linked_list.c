#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
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
  for (i=0; i<bucket_count; i++)
  {
    *(list->buckets + i) = NULL;
  }
}

void print_list_details(HashList *list)
{
  printf("Length: %i\n", list->length);
  HashListNode *i;
  i = list->next;
  while(i)
  {
    printf("Hash: %llu\n", i->hash);
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

HashListNode* hl_insert_or_find(HashList *list, uint64_t passed_hash)
{
  HashListNode* found_node;
  int bucket_number = find_target_bucket(passed_hash, list->bucket_interval);

  if(!list->next)
  {
    HashListNode *new_node = (HashListNode*)malloc(sizeof(HashListNode));

    new_node->hash = passed_hash;
    new_node->next = NULL;
    new_node->bucket_next = NULL;
    new_node->bucket_previous = NULL;
    list->last = list->next = list->buckets[bucket_number] = new_node;

    increment_length(list);
    return list->next;
  }
  found_node = hl_find_node(list, passed_hash);
  if(found_node)
  {
    //printf("Found previously existing node, bumping %llu to front\n", passed_hash);
    if(found_node->bucket_previous != NULL)
    {
      found_node->bucket_previous->bucket_next = found_node->bucket_next;
      found_node->bucket_next = list->buckets[bucket_number];
      list->buckets[bucket_number] = found_node;
    }
    return found_node;
  }
  else
  {
    HashListNode *new_node = (HashListNode*)malloc(sizeof(HashListNode));
    new_node->hash = passed_hash;
    new_node->next = NULL;
    new_node->bucket_next = NULL;
    new_node->bucket_previous = NULL;
    if(list->buckets[bucket_number] != NULL)
    {
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

  if(i == NULL)
  {
    return NULL;
  }

  while(i)
  {
    if(i->hash == passed_hash)
    {
      return i;
    }
    i = i->bucket_next;
  }
  return NULL;
}

int find_target_bucket(uint64_t passed_hash, unsigned long interval)
{
  return passed_hash / interval;
}
