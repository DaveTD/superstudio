#include "json_object_array.h"
#include "hash_linked_list.h"

void create_array_object(JSONDocumentBuilder* builder, uint64_t hash, uint64_t parent_hash)
{
  HashListNode* found_node = NULL;
  ArrayObjectJSON* parent_array_object = NULL;

  found_node = hl_find_node(builder->search_list, hash);
  if (found_node)
  {
    parent_array_object = found_node->related_JSON_object->array_objects;
  }
  else
  {
    parent_array_object = builder->root;
  }

  if(!parent_array_object->value_list)
  {
    parent_array_object->value_list = (ArrayObjectListItem*)calloc(1, sizeof(ArrayObjectListItem));
    parent_array_object->value_list->array_object = (JSONObject*)calloc(1, sizeof(JSONObject));
    parent_array_object->value_list->array_object->array_values = (ArrayValueJSON*)calloc(1, sizeof(ArrayValueJSON));
    parent_array_object->last_list_object = parent_array_object->value_list;
  }
  else
  {
    parent_array_object->last_list_object->next_item = (ArrayObjectListItem*)calloc(1, sizeof(ArrayObjectListItem));
    parent_array_object->last_list_object->next_item->array_object = (JSONObject*)calloc(1, sizeof(JSONObject));
    parent_array_object->last_list_object->next_item->array_object->array_values = (ArrayValueJSON*)calloc(1, sizeof(ArrayValueJSON));
    parent_array_object->last_list_object = parent_array_object->last_list_object->next_item; 
  }

  hl_insert_or_find(builder->search_list, hash, parent_array_object->last_list_object->array_object);
}

unsigned long finalize_object_array(JSONDocumentBuilder* builder, ArrayObjectJSON* object_array, unsigned long counter)
{
  ArrayObjectListItem* target_object = object_array->value_list;
  memcpy(builder->resulting_json + counter, ob, 1);
  counter++;

  while (target_object)
  {
    counter = finalize_key_values(builder, target_object->array_object->single_values, target_object, counter);
    // Something similar for key->objects goes here
    counter = finalize_value_array(builder, target_object->array_object->array_values, counter);
    // Something similar for object arrays goes here

    memcpy(builder->resulting_json + counter, cbr, 1);
    counter++;

    if (target_object->next_item)
    {
      memcpy(builder->resulting_json + counter, cm, 1);
      counter++;
    }
    else
    {
      memcpy(builder->resulting_json + counter, cb, 1);
      counter++;
    }
    target_object = target_object->next_item;
  }

  return counter;
}