#include "json_value_array.h"
#include "fnv_64.h"
#include "hash_linked_list.h"

void add_to_array(
  JSONDocumentBuilder* builder,
  uint64_t hash,
  char* string_value,
  unsigned long string_size,
  char* column_name,
  unsigned long column_name_length,
  unsigned long repeatable,
  unsigned long quoted
  )
{
  HashListNode* found_node;
  JSONObject* parent_object;
  ArrayValueJSON* array_search;
  ArrayValueListItem* array_values_search;
  unsigned char item_added = 0;
  int repeat_found = 0;

  found_node = hl_find_node(builder->search_list, hash);
  parent_object = found_node->related_JSON_object;
  array_search = parent_object->array_values;

  while (array_search && !item_added)
  {
    // Check if this value array has been fully set
    if (!array_search->set_flag)
    {
      array_search->name = (char*)malloc(column_name_length);
      memcpy(array_search->name, column_name, column_name_length);
      array_search->name_characters = column_name_length;

      array_search->quoted = quoted;

      array_search->value_list = (ArrayValueListItem*)calloc(1, sizeof(ArrayValueListItem));
      array_search->last_list_value = array_search->value_list;
      array_search->next_value = (ArrayValueJSON*)calloc(1, sizeof(ArrayValueJSON));;

      array_search->set_flag = 1;
      parent_object->value_array_count += 1;

      builder->json_char_count += column_name_length;
      builder->json_char_count += 6; // Two quotes, a colon, starting and ending brackets, a comma (might want to add the comma space somewhere else)
    }

    // Loop through, check length and memcmp names if it matches
    if (!memcmp(array_search->name, column_name, column_name_length) && (array_search->name_characters == column_name_length) && !item_added)
    {
      builder->json_char_count += 1; // This array already existed, it needs a comma

      // Finding a match, if repeats are allowed, just add, otherwise loop through and match size, then names again, discarding if a match was found
      if (repeatable)
      {
        initialize_value_item(builder, array_search->last_list_value, string_size, string_value, quoted);
        array_search->last_list_value = array_search->last_list_value->next_value;
        item_added = 1;
      }
      else
      {
        array_values_search = array_search->value_list;
        // Since we can't repeat, we need to identify if this value already exists
        while (array_values_search->set_flag && !repeat_found)
        {
          if (((memcmp(array_values_search->array_value, string_value, string_size) == 0) && (array_values_search->value_characters == string_size)))
          {
            repeat_found = 1;
          }

          array_values_search = array_values_search->next_value;
        }
        if (!repeat_found)
        {
          initialize_value_item(builder, array_search->last_list_value, string_size, string_value, quoted);
          array_search->last_list_value = array_search->last_list_value->next_value;
          item_added = 1;
        }
      }
    }

    array_search = array_search->next_value;
  }
}

void initialize_value_item(JSONDocumentBuilder* builder, ArrayValueListItem *target, unsigned long value_characters, char* array_value, unsigned long quoted)
{
  target->next_value = (ArrayValueListItem*)calloc(1, sizeof(ArrayValueListItem));
  target->value_characters = value_characters;
  target->array_value = (char*)malloc(value_characters);
  memcpy(target->array_value, array_value, value_characters);
  target->set_flag = 1;

  builder->json_char_count += value_characters;
  if (quoted)
  {
    builder->json_char_count += 2;
  }
}

unsigned long finalize_value_array(JSONDocumentBuilder* builder, ArrayValueJSON* value_arrays, unsigned long counter)
{
  ArrayValueListItem* value_array_values = NULL;
  
  while(value_arrays->set_flag)
  {
    value_array_values = value_arrays->value_list;

    memcpy(builder->resulting_json + counter, oq, 1);
    counter++;

    memcpy(builder->resulting_json + counter, value_arrays->name, value_arrays->name_characters);
    counter += value_arrays->name_characters;

    memcpy(builder->resulting_json + counter, oqc, 2);
    counter += 2;

    memcpy(builder->resulting_json + counter, ob, 1);
    counter++;

    while(value_array_values->set_flag)
    {
      if (value_arrays->quoted)
      {
        memcpy(builder->resulting_json + counter, oq, 1);
        counter++;
      }

      memcpy(builder->resulting_json + counter, value_array_values->array_value, value_array_values->value_characters);
      counter += value_array_values->value_characters;

      if (value_arrays->quoted)
      {
        memcpy(builder->resulting_json + counter, oq, 1);
        counter++;
      }

      if (value_array_values->next_value->set_flag)
      {
        memcpy(builder->resulting_json + counter, cm, 1);
        counter++;
      }

      value_array_values = value_array_values->next_value;
    }
    
    memcpy(builder->resulting_json + counter, cb, 1);
    counter++;

    if (value_arrays->next_value->set_flag)
    {
      memcpy(builder->resulting_json + counter, cm, 1);
      counter++;
    }

    value_arrays = value_arrays->next_value;
  }
  return counter;  
}