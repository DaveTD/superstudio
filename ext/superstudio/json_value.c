#include "json_value.h"
#include "hash_linked_list.h"
#include "ss_alloc.h"
  
void set_key_values(
  JSONDocumentBuilder* builder, 
  unsigned long* string_sizes, 
  char** row, 
  unsigned long visible_depth,
  JSONLevelBuilder* level_definitions,
  JSONObject* parent_object 
  )
{
  unsigned long counter = 0;
  unsigned long visible_counter = 0;
  int found = 0;
  SingleValueJSON* duplicate_finder;

  while(counter < level_definitions->column_count) {
    if (visible_depth == level_definitions->depth_array[counter]) {
      if (read_type(visible_depth, level_definitions->mapping_array[counter], level_definitions->mapping_array_lengths[counter]) == '1') {
        found = 0;

        if (parent_object->single_values) {
          duplicate_finder = parent_object->single_values;
          while(duplicate_finder->set_flag) {
            if (duplicate_finder->name_characters == level_definitions->column_name_lengths[counter]) {
              if (!memcmp(duplicate_finder->name, level_definitions->column_names[counter], duplicate_finder->name_characters)) {
                found = 1;
              }
            }
            duplicate_finder = duplicate_finder->next_value;
          }
        }

        if (!found) {
          builder->json_char_count += 2; // Add space for quotes for the key
          builder->json_char_count += level_definitions->column_name_lengths[counter]; // Add space for the key

          builder->json_char_count += 1; // Add space for the colon 
          if (level_definitions->quote_array[visible_counter]) {
            builder->json_char_count += 2; // Add space for the quotes on the value, if needed
          }
          builder->json_char_count += string_sizes[visible_counter]; // Add space for the value
          
          if (!parent_object->single_values) {
            parent_object->single_values = (SingleValueJSON*)ss_alloc(builder->memory_stack, 1, sizeof(SingleValueJSON));
            parent_object->last_single_value = parent_object->single_values;
          }

          if (parent_object->single_values->set_flag) {
            parent_object->last_single_value = parent_object->last_single_value->next_value;
          }
          parent_object->last_single_value->next_value = (SingleValueJSON*)ss_alloc(builder->memory_stack, 1, sizeof(SingleValueJSON));
          parent_object->last_single_value->set_flag = 1;          
          parent_object->last_single_value->name = level_definitions->column_names[counter];

          parent_object->last_single_value->value = (char*)ss_alloc(builder->memory_stack, 1, string_sizes[counter]);
          memcpy(parent_object->last_single_value->value, row[counter], (string_sizes[counter]));

          parent_object->last_single_value->value_characters = string_sizes[visible_counter];
          parent_object->last_single_value->name_characters = level_definitions->column_name_lengths[counter];
          parent_object->last_single_value->quoted = level_definitions->quote_array[visible_counter];
        }

        if (level_definitions->column_count != (counter + 1)) {
          builder->json_char_count += 1; // Add a space for the comma, as long as this not the last item in the object
        }
      }
      visible_counter++;
    }
    counter++;
  }
}

unsigned long finalize_key_values(JSONDocumentBuilder* builder, SingleValueJSON* single_values, JSONObject* parent_json, unsigned long counter)
{
  memcpy(builder->resulting_json + counter, obr, 1);
  counter++;
  while(single_values->set_flag) {
    memcpy(builder->resulting_json + counter, oq, 1);
    counter++;
    memcpy(builder->resulting_json + counter, single_values->name, single_values->name_characters);
    counter += single_values->name_characters;

    memcpy(builder->resulting_json + counter, oqc, 2);
    counter += 2;

    if (single_values->quoted) {
      memcpy(builder->resulting_json + counter, oq, 1);
      counter++;
      memcpy(builder->resulting_json + counter, single_values->value, single_values->value_characters);
      counter += single_values->value_characters;

      memcpy(builder->resulting_json + counter, oq, 1);
      counter++;
    } else {
      memcpy(builder->resulting_json + counter, single_values->value, single_values->value_characters);
      counter += single_values->value_characters;
    }

    if ((single_values != parent_json->last_single_value) || 
      ((single_values == parent_json->last_single_value) && parent_json->single_objects->set_flag) ||
      ((single_values == parent_json->last_single_value) && !(parent_json->single_objects->set_flag) && parent_json->array_values->set_flag)
    ) {
      memcpy(builder->resulting_json + counter, cm, 1);
      counter++;
    }

    single_values = single_values->next_value;
  }
  return counter;
}