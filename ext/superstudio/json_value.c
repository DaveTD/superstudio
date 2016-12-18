#include "json_value.h"
#include "hash_linked_list.h"
  
void set_key_values(JSONDocumentBuilder* builder, unsigned long* string_sizes, char** row, unsigned long hash, unsigned char at_depth_count)
{
  HashListNode* found_node;
  JSONObject* parent_object;

  found_node = hl_find_node(builder->search_list, hash);
  parent_object = found_node->related_JSON_object;

  unsigned long counter = 0;

  while (counter < at_depth_count)
  {
    if (builder->mapping_array[counter][0] == '1')
    {
      builder->json_char_count += 2; // Add space for quotes for the key
      builder->json_char_count += builder->column_name_lengths[counter]; // Add space for the key

      builder->json_char_count += 1; // Add space for the colon 
      
      if (builder->quote_array[counter])
      {
        builder->json_char_count += 2; // Add space for the quotes on the value, if needed
      }
      builder->json_char_count += string_sizes[counter]; // Add space for the value
      
      if (parent_object->single_values) 
      {
        parent_object->last_single_value->next_value->next_value = (SingleValueJSON*)malloc(sizeof(SingleValueJSON));

        parent_object->last_single_value->next_value->name = (char*)malloc(builder->column_name_lengths[counter] + 1);
        memcpy(parent_object->last_single_value->next_value->name, builder->column_names[counter], (builder->column_name_lengths[counter]) + 1);

        parent_object->last_single_value->next_value->value = (char*)malloc(string_sizes[counter] + 1);
        memcpy(parent_object->last_single_value->next_value->value, row[counter], (string_sizes[counter]) + 1);

        parent_object->last_single_value->next_value->value_characters = string_sizes[counter];
        parent_object->last_single_value->next_value->name_characters = builder->column_name_lengths[counter];
        parent_object->last_single_value->next_value->quoted = builder->quote_array[counter];

        parent_object->last_single_value = parent_object->last_single_value->next_value;
      }
      else
      {
        parent_object->single_values = (SingleValueJSON*)malloc(sizeof(SingleValueJSON));
        parent_object->single_values->next_value = (SingleValueJSON*)malloc(sizeof(SingleValueJSON));
        
        parent_object->single_values->name = (char*)malloc(builder->column_name_lengths[counter] + 1);
        memcpy(parent_object->single_values->name, builder->column_names[counter], (builder->column_name_lengths[counter]) + 1);

        parent_object->single_values->value = (char*)malloc(string_sizes[counter] + 1);
        memcpy(parent_object->single_values->value, row[counter], (string_sizes[counter]) + 1);

        parent_object->single_values->value_characters = string_sizes[counter];
        parent_object->single_values->name_characters = builder->column_name_lengths[counter];
        parent_object->single_values->quoted = builder->quote_array[counter];

        parent_object->last_single_value = parent_object->single_values;
      }
    }

    /* THIS IS PROBABLY CAUSING EXTRA MEMORY ALLOCATION COME BACK TO IT AFTER */
    if (at_depth_count != (counter + 1) )
    {
      builder->json_char_count += 1; // Add a space for the comma, as long as this not the last item in the object
    }
    /* THIS IS PROBABLY CAUSING EXTRA MEMORY ALLOCATION COME BACK TO IT AFTER */

    counter++;
  }
}

unsigned long finalize_key_values(JSONDocumentBuilder* builder, SingleValueJSON* single_values, ArrayObjectListItem* item, unsigned long counter)
{
  memcpy(builder->resulting_json + counter, obr, 1);
  counter++;

  while (single_values != item->array_object->last_single_value->next_value)
  {
    memcpy(builder->resulting_json + counter, oq, 1);
    counter++;
    memcpy(builder->resulting_json + counter, single_values->name, single_values->name_characters);
    counter += single_values->name_characters;

    memcpy(builder->resulting_json + counter, oqc, 2);
    counter += 2;

    if (single_values->quoted)
    {
      memcpy(builder->resulting_json + counter, oq, 1);
      counter++;
      memcpy(builder->resulting_json + counter, single_values->value, single_values->value_characters);
      counter += single_values->value_characters;
      memcpy(builder->resulting_json + counter, oq, 1);
      counter++;
    }
    else
    {
      memcpy(builder->resulting_json + counter, single_values->value, single_values->value_characters);
      counter += single_values->value_characters;
    }

    if ((single_values->next_value != item->array_object->last_single_value->next_value) || item->array_object->array_values->set_flag)
    {
      memcpy(builder->resulting_json + counter, cm, 1);
      counter++;
    }
    single_values = single_values->next_value;
  }
  return counter;
}