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
  unsigned long quoted,
  JSONObject* parent_object
  )
{
  ArrayValueJSON* array_search;
  ArrayValueListItem* array_values_search;
  unsigned char item_added = 0;
  int repeat_found = 0;

  array_search = parent_object->array_values;
  while(!item_added) {
    if (!array_search->set_flag) {
      array_search->name = (char*)calloc(1, column_name_length);
      memcpy(array_search->name, column_name, column_name_length);
      array_search->name_characters = column_name_length;

      array_search->quoted = quoted;

      array_search->value_list = (ArrayValueListItem*)calloc(1, sizeof(ArrayValueListItem));
      array_search->last_list_value = array_search->value_list;
      array_search->next_value = (ArrayValueJSON*)calloc(1, sizeof(ArrayValueJSON));

      array_search->set_flag = 1;
      parent_object->value_array_count += 1;

      builder->json_char_count += column_name_length;
      builder->json_char_count += 6; // Two quotes, a colon, starting and ending brackets, a comma (might want to add the comma space somewhere else)
    }

    if (!memcmp(array_search->name, column_name, column_name_length) && (array_search->name_characters == column_name_length) && !item_added) {
      builder->json_char_count += 1; // This array already existed, it needs a comma
      if (repeatable) {
        initialize_value_item(builder, array_search->last_list_value, string_size, string_value, quoted);
        array_search->last_list_value = array_search->last_list_value->next_value;
        item_added = 1;
      } else {
        array_values_search = array_search->value_list;
        while(array_values_search->set_flag && !repeat_found) {
          if (((memcmp(array_values_search->array_value, string_value, string_size) == 0) && (array_values_search->value_characters == string_size))) {
            repeat_found = 1;
          }
          array_values_search = array_values_search->next_value;
        }
        if (!repeat_found) {
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
  target->array_value = (char*)calloc(1, value_characters);
  memcpy(target->array_value, array_value, value_characters);
  target->set_flag = 1;

  builder->json_char_count += value_characters;
  if (quoted) { builder->json_char_count += 2; }
}

void set_value_arrays(
  JSONDocumentBuilder* builder, 
  JSONLevelBuilder* level_definitions, 
  uint64_t hash, 
  unsigned long column_count, 
  char** row_strings, 
  unsigned long* string_sizes, 
  unsigned long accessing_depth,
  JSONObject* parent_object
  )
{
  int counter = 0;
    while(counter < column_count) {
      if ((level_definitions->depth_array[counter] == (accessing_depth + 1)) && (level_definitions->do_not_hash[counter])) {
        add_to_array(builder, 
          hash, 
          row_strings[counter],
          string_sizes[counter],
          level_definitions->column_names[counter],
          level_definitions->column_name_lengths[counter],
          level_definitions->repeating_array_columns[counter],
          level_definitions->quote_array[counter],
          parent_object
          );
      }
      counter++;
    }
}

unsigned long finalize_value_array(JSONDocumentBuilder* builder, ArrayValueJSON* value_arrays, unsigned long counter)
{
  ArrayValueListItem* value_array_values = NULL;
  
  while(value_arrays->set_flag) {
    value_array_values = value_arrays->value_list;

    memcpy(builder->resulting_json + counter, oq, 1);
    counter++;

    memcpy(builder->resulting_json + counter, value_arrays->name, value_arrays->name_characters);
    counter += value_arrays->name_characters;

    memcpy(builder->resulting_json + counter, oqc, 2);
    counter += 2;

    memcpy(builder->resulting_json + counter, ob, 1);
    counter++;

    while(value_array_values->set_flag) {
      if (value_arrays->quoted) {
        memcpy(builder->resulting_json + counter, oq, 1);
        counter++;
      }

      memcpy(builder->resulting_json + counter, value_array_values->array_value, value_array_values->value_characters);
      counter += value_array_values->value_characters;

      if (value_arrays->quoted) {
        memcpy(builder->resulting_json + counter, oq, 1);
        counter++;
      }

      if (value_array_values->next_value->set_flag) {
        memcpy(builder->resulting_json + counter, cm, 1);
        counter++;
      }

      value_array_values = value_array_values->next_value;
    }
    
    memcpy(builder->resulting_json + counter, cb, 1);
    counter++;

    if (value_arrays->next_value->set_flag) {
      memcpy(builder->resulting_json + counter, cm, 1);
      counter++;
    }

    value_arrays = value_arrays->next_value;
  }
  return counter;  
}