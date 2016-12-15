#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "json_builder.h"
#include "hash_linked_list.h"
#include "fnv_64.h"

void json_builder_initialize(JSONBuilder *builder)
{
  builder->mapping_array = NULL;
  builder->quote_array = NULL;
  builder->do_not_hash = NULL;
  builder->depth_array = NULL;
  builder->real_depth_array = NULL;
  builder->row_being_worked_on = NULL;
  builder->resulting_json = NULL;
  builder->repeating_array_columns = NULL;
  builder->root = (ArrayObjectJSON*)malloc(sizeof(ArrayObjectJSON));
  
  builder->root->name = "root";
  builder->root->identifier = 0;
  builder->root->value_list = NULL;

  builder->search_list = (HashList*)malloc(sizeof(HashList));
  builder->row_count = 0;
  builder->column_count = 0;
  builder->json_char_count = 3; //Starts with "[]\0"
  builder->max_depth = 0;
  builder->max_real_depth = 0;
  
  builder->column_name_lengths = NULL;
  builder->column_names = NULL;
}

void set_row_count(JSONBuilder *builder, unsigned long count)
{
  builder->row_count = count;
  hl_initialize(builder->search_list, count);
}

unsigned long get_row_count(JSONBuilder *builder)
{
  return builder->row_count;
}

void set_repeating_array_columns(JSONBuilder *builder, unsigned long* repeating)
{
  builder->repeating_array_columns = (unsigned long*)malloc(sizeof(unsigned long) * builder->column_count);
  memcpy(builder->repeating_array_columns, repeating, sizeof(unsigned long) * builder->column_count);
}

void set_column_count(JSONBuilder *builder, unsigned long count)
{
  builder->column_count = count;
}

unsigned long get_column_count(JSONBuilder *builder)
{
  return builder->column_count;
}

void set_quote_array(JSONBuilder *builder, unsigned long* quotes)
{
  builder->quote_array = (unsigned long*)malloc(sizeof(unsigned long) * builder->column_count);
  memcpy(builder->quote_array, quotes, sizeof(unsigned long) * (builder->column_count));
}

void set_hashing_array(JSONBuilder *builder, unsigned long* do_not_hashes)
{
  builder->do_not_hash = (unsigned long*)malloc(sizeof(unsigned long) * builder->column_count);
  memcpy(builder->do_not_hash, do_not_hashes, sizeof(unsigned long) * (builder->column_count));
}

void set_depth_array(
  JSONBuilder *builder, 
  unsigned long* depths, 
  unsigned long max, 
  unsigned long* real_depths, 
  unsigned long max_real
  )
{
  builder->depth_array = (unsigned long*)malloc(sizeof(unsigned long) * builder->column_count);
  memcpy(builder->depth_array, depths, sizeof(unsigned long) * (builder->column_count));

  builder->max_depth = max;
  
  builder->real_depth_array = (unsigned long*)malloc(sizeof(unsigned long) * builder->column_count);
  memcpy(builder->real_depth_array, real_depths, sizeof(unsigned long) * (builder->column_count));
  
  builder->max_real_depth = max_real;
}

void set_column_names_sizes(JSONBuilder *builder, char** column_names, unsigned long* column_string_sizes)
{
  builder->column_name_lengths = (unsigned long*)malloc(sizeof(unsigned long) * builder->column_count);
  builder->column_names = (char**)malloc(sizeof(char*) * builder->column_count);

  memcpy(builder->column_name_lengths, column_string_sizes, sizeof(unsigned long*) * (builder->column_count));
  memcpy(builder->column_names, column_names, sizeof(char*) * (builder->column_count));
}

void set_mapping_array(JSONBuilder *builder, char** internal_map)
{
  builder->mapping_array = (char**)malloc(sizeof(char*) * builder->column_count);
  memcpy(builder->mapping_array, internal_map, sizeof(char*) * (builder->column_count));

  unsigned int counter = 0;
  char* column_mapping;
  unsigned int sub_column_indentifiers[builder->column_count]; // Max possible size
  unsigned int sub_column_count = 0; // Actual count

  // Recursively set sub-mapping arrays here
  // builder->sub_rows
  
  //Get a list of the type 4s
  while (counter < builder->column_count)
  {
    // turn back on to see internal mapped types
    //printf("%c\n", internal_map[counter][0]);
    if (internal_map[counter][0] == '4')
    {
      column_mapping = internal_map[counter];
      //printf("4 column mapping: %s\n", column_mapping);
    }
    else
    {
      //printf("Other column mapping: %s\n", internal_map[counter]);
    }
    counter++;
  }

  // Recursively set sub-mapping arrays here


  // unsigned long counter = 0;
  // while(counter < builder->column_count)
  // {
  //   printf("Mapping array value: %s\n", builder->mapping_array[counter]);
  //   counter++;
  // }
}

void consume_row(
  JSONBuilder *builder,
  char** row,
  unsigned long* string_sizes,
  unsigned long accessing_depth,
  unsigned long column_count,
  uint64_t parent_hash,
  unsigned long* visible_depth_array
  )
{
  /*
  *   When accessing_depth is 0, we're looking at the root. This would be the only depth that
  *   we should look at creating an entirely new struture - otherwise we should be
  *   able to find another structure that this row can add new data into.
  */
  uint64_t hash = FNV_OFFSET;
  unsigned long counter = 0;
  unsigned long inner_counter = 0;
  unsigned int dnh_counter = 0;
  unsigned long next_depth = accessing_depth + 1;
  HashListNode* found_node;
  HashListNode* parent_node;
  JSONObject* parent_object;

  /*
  *   Variables for when sub-objects are found
  */
  char* sub_object_row[column_count];
  unsigned long sub_object_column_count = 0;
  unsigned long sub_object_string_sizes[column_count];
  unsigned int sub_object_groups[column_count];
  unsigned long sub_object_visible_depth_array[column_count];
  /*
  *   Variables for when sub-objects are found
  */

  RowBuilder* sub_items;

  while(counter < column_count)
  {
    //printf("Real depth value found: %lu\n", builder->real_depth_array[counter]);
    inner_counter = 0;
    if ((builder->real_depth_array[counter] == accessing_depth) && (!builder->do_not_hash[counter]))
    {
      //printf("Entered First Condition\n");
      while (inner_counter < string_sizes[counter])
      {
        // Make sure this column is the correct depth, and should be hashed
        hash = fnv_hash_byte(hash, &row[counter][inner_counter]);
        //printf("Char: %c Hash value updated to: %lu\n", row[counter][inner_counter], hash);
        inner_counter++;
      }
    }
    else if ((builder->real_depth_array[counter] == accessing_depth) && (builder->do_not_hash[counter]))
    {
      //printf("Do not hash value: %lu\n", builder->do_not_hash[counter]);
      //printf("Type 3 Found!\n");

      dnh_counter++;
    }
    else if(builder->real_depth_array[counter] != accessing_depth)
    {
      // Build out the next deepest definitions to send off later      
      // This new array will contain the second number after the "4.", which will make it easier to access which items go to which next-level row consume
      // Start forwards 2 characters
      inner_counter = 2;
      char identifier[string_sizes[counter] - 2];

      // printf("String Size found: %lu\n", string_sizes[counter]);

      while(inner_counter < string_sizes[counter])
      {
        // Is this efficient?
        // identifier[inner_counter - 2] = builder->mapping_array[counter][inner_counter];

        //printf("%lu", inner_counter);
        //printf("%c", builder->mapping_array[counter][inner_counter]);
        inner_counter++;
      }

      // Check if idendifier exists as the identifier to any of the RowBuilders in the JSONBuilder

      sub_object_column_count++;
    }
    counter++;
  }
  counter = 0;
  found_node = hl_find_node(builder->search_list, hash);
  if(found_node == NULL)
  {
    if(accessing_depth == 0)
    {
      //printf("Creating root item... \n");
      builder->json_char_count += 3; // Starting, ending brace and comma
      create_new_root_item(builder, hash, row, string_sizes, 0);
      //hl_insert_or_find(builder->search_list, hash);
    }
    else
    {
      // A type 4
      printf("Found parent item... \n");
      // Find your parent node
      parent_node = hl_find_node(builder->search_list, parent_hash);
      parent_object = parent_node->related_JSON_object;

      insert_for_parent(builder, accessing_depth, visible_depth_array, parent_object);

      // Find deeper items (4s), recursive party
      if(sub_object_column_count)
      {
        // Send this for each set of 4s found from before this section starts
        sub_build(builder, sub_object_row, sub_object_string_sizes, next_depth, sub_object_column_count, sub_object_groups, parent_object, visible_depth_array);
      }
    }
  }

  // Find 3s, add those in
  if(dnh_counter)
  {
    while (counter < column_count)
    {
      if ((builder->real_depth_array[counter] == accessing_depth) && (builder->do_not_hash[counter]))
      {
        add_to_array(
          builder, 
          hash, 
          row[counter], 
          string_sizes[counter], 
          builder->column_names[counter], 
          builder->column_name_lengths[counter], 
          builder->repeating_array_columns[counter],
          builder->quote_array[counter]
          );
      }
      counter++;
    }
  }
}

void add_to_array(
  JSONBuilder* builder,
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
        array_search->last_list_value->next_value = (ArrayValueListItem*)calloc(1, sizeof(ArrayValueListItem));
        array_search->last_list_value->value_characters = string_size;
        array_search->last_list_value->array_value = (char*)malloc(string_size);
        memcpy(array_search->last_list_value->array_value, string_value, string_size);
        array_search->last_list_value->set_flag = 1;
        array_search->last_list_value = array_search->last_list_value->next_value;

        builder->json_char_count += string_size;
        if (quoted)
        {
          builder->json_char_count += 2; // Add space for the quotes on the value, if needed
        }

        item_added = 1;
      }
      else
      {
        array_values_search = array_search->value_list;
        // Since we can't repeat, we need to identify if this value already exists
        while (array_values_search && !repeat_found)
        {
          if (((memcmp(array_values_search->array_value, string_value, string_size) == 0) && (array_values_search->value_characters == string_size)))
          {
            repeat_found = 1;
          }

          array_values_search = array_values_search->next_value;
        }
        if (!repeat_found)
        {
          array_search->last_list_value->next_value = (ArrayValueListItem*)calloc(1, sizeof(ArrayValueListItem));
          array_search->last_list_value->value_characters = string_size;
          array_search->last_list_value->array_value = (char*)malloc(string_size);
          memcpy(array_search->last_list_value->array_value, string_value, string_size);
          array_search->last_list_value->set_flag = 1;
          array_search->last_list_value = array_search->last_list_value->next_value;

          builder->json_char_count += string_size;
          if (quoted)
          {
            builder->json_char_count += 2; // Add space for the quotes on the value, if needed
          }

          item_added = 1;
        }
      }
    }

    array_search = array_search->next_value;
  }
}

void sub_build(
  JSONBuilder* builder, 
  char** sub_object_row, 
  unsigned long * sub_object_string_sizes, 
  unsigned int next_depth, 
  unsigned long sub_object_column_count, 
  unsigned int* sub_object_groups, 
  JSONObject* parent_object,
  unsigned long* visible_depth_array
  )
{
  // The only job here is to separate out items at the given depth to call consume row on
}

void create_new_root_item(
  JSONBuilder* builder,
  uint64_t hash,
  char** row,
  unsigned long* string_sizes,
  uint64_t parent_hash
  )
{
  unsigned long counter = 0;

  // move through list to end, create a new type 1 object here
  if(!builder->root->value_list)
  {
    builder->root->value_list = (ArrayObjectListItem*)malloc(sizeof(ArrayObjectListItem));
    builder->root->value_list->next_item = NULL;
    
    builder->root->value_list->array_object = (JSONObject*)malloc(sizeof(JSONObject));

    builder->root->value_list->array_object->single_values = NULL;
    builder->root->value_list->array_object->single_objects = NULL;
    builder->root->value_list->array_object->array_values = (ArrayValueJSON*)calloc(1, sizeof(ArrayValueJSON));
    builder->root->value_list->array_object->array_objects = NULL;

    builder->root->last_list_object = builder->root->value_list;
  }
  else
  {
    //set the target down until it's null
    builder->root->last_list_object->next_item = (ArrayObjectListItem*)malloc(sizeof(ArrayObjectListItem));
    builder->root->last_list_object->next_item->next_item = NULL;

    builder->root->last_list_object->next_item->array_object = (JSONObject*)malloc(sizeof(JSONObject));

    builder->root->last_list_object->next_item->array_object->single_values = NULL;
    builder->root->last_list_object->next_item->array_object->single_objects = NULL;
    builder->root->last_list_object->next_item->array_object->array_values = (ArrayValueJSON*)calloc(1, sizeof(ArrayValueJSON));
    builder->root->last_list_object->next_item->array_object->array_objects = NULL;

    builder->root->last_list_object = builder->root->last_list_object->next_item; 
  }
  // target_array_object should now be en empty JSONObject

  // make this new item the hashlist item's object
  hl_insert_or_find(builder->search_list, hash, builder->root->last_list_object->array_object);
  //hl_set_json_object(inserted_item, );

  while (counter < builder->column_count)
  {
    //printf("Indicator char: %c\n", builder->mapping_array[counter][0]);
    
    if (builder->mapping_array[counter][0] == '1')
    {
      builder->json_char_count += 2; // Add space for quotes for the key
      builder->json_char_count += builder->column_name_lengths[counter]; // Add space for the key

      builder->json_char_count += 1; // Add space for the colon 

      
      if (builder->quote_array[counter])
      {
        builder->json_char_count += 2; // Add space for the quotes on the value, if needed
      }
      //printf("String found with length: %lu\n", string_sizes[counter]);
      builder->json_char_count += string_sizes[counter]; // Add space for the value
      
      //use the malloc'd target array object
      if (builder->root->last_list_object->array_object->single_values) 
      {
        //target_array_object->last_single_value->next_value = (SingleValueJSON*)malloc(sizeof(SingleValueJSON));
        builder->root->last_list_object->array_object->last_single_value->next_value->next_value = (SingleValueJSON*)malloc(sizeof(SingleValueJSON));

        builder->root->last_list_object->array_object->last_single_value->next_value->name = (char*)malloc(builder->column_name_lengths[counter] + 1);
        memcpy(builder->root->last_list_object->array_object->last_single_value->next_value->name, builder->column_names[counter], (builder->column_name_lengths[counter]) + 1);

        builder->root->last_list_object->array_object->last_single_value->next_value->value = (char*)malloc(string_sizes[counter] + 1);
        memcpy(builder->root->last_list_object->array_object->last_single_value->next_value->value, row[counter], (string_sizes[counter]) + 1);

        builder->root->last_list_object->array_object->last_single_value->next_value->value_characters = string_sizes[counter];
        builder->root->last_list_object->array_object->last_single_value->next_value->name_characters = builder->column_name_lengths[counter];
        builder->root->last_list_object->array_object->last_single_value->next_value->quoted = builder->quote_array[counter];

        builder->root->last_list_object->array_object->last_single_value = builder->root->last_list_object->array_object->last_single_value->next_value;
      }
      else
      {
        // if there's nothing there, this is the first and last
        builder->root->last_list_object->array_object->single_values = (SingleValueJSON*)malloc(sizeof(SingleValueJSON));
        builder->root->last_list_object->array_object->single_values->next_value = (SingleValueJSON*)malloc(sizeof(SingleValueJSON));
        
        builder->root->last_list_object->array_object->single_values->name = (char*)malloc(builder->column_name_lengths[counter] + 1);
        memcpy(builder->root->last_list_object->array_object->single_values->name, builder->column_names[counter], (builder->column_name_lengths[counter]) + 1);

        builder->root->last_list_object->array_object->single_values->value = (char*)malloc(string_sizes[counter] + 1);
        memcpy(builder->root->last_list_object->array_object->single_values->value, row[counter], (string_sizes[counter]) + 1);

        builder->root->last_list_object->array_object->single_values->value_characters = string_sizes[counter];
        builder->root->last_list_object->array_object->single_values->name_characters = builder->column_name_lengths[counter];
        builder->root->last_list_object->array_object->single_values->quoted = builder->quote_array[counter];

        builder->root->last_list_object->array_object->last_single_value = builder->root->last_list_object->array_object->single_values;
      }
    }

    /* THIS IS PROBABLY CAUSING EXTRA MEMORY ALLOCATION COME BACK TO IT AFTER */
    if (builder->column_count != (counter + 1) )
    {
      builder->json_char_count += 1; // Add a space for the comma, as long as this not the last item in the object
    }
    /* THIS IS PROBABLY CAUSING EXTRA MEMORY ALLOCATION COME BACK TO IT AFTER */

    counter++;
  }

  //For each non-4 object, create and insert

  //Go through each type-4 object, call adding their stuff, as deep as it goes (aka call sub_build)


}

char* read_value(char depth_start, char is_first, char* mapped_value)
{
  char* return_string;
  unsigned int starting_character = 0;
  char depth_counter = 0;
  if (depth_start > 0)
  {
    while (depth_counter < depth_start)
    {
      if (starting_character > strlen(mapped_value))
      {
        // Returns 0 if the mapped value doesn't go deep enough
        return 0;
      }
      else if (mapped_value[starting_character] == '-')
      {
        depth_counter++;
      }
      starting_character++;
    }
  }

  if (is_first == 0)
  {
    starting_character += 2;
    while(starting_character < strlen(mapped_value)
      && mapped_value[starting_character] != '.')
    {
      return_string = append(return_string, mapped_value[starting_character]);
    }
  }
  else
  {
    return_string = malloc(sizeof(char));
    ++starting_character;
    return_string[0] = mapped_value[starting_character];
  }

  return return_string;
}

char* append(const char *input, const char c)
{
  size_t len = strlen(input);
  char *new_string = malloc(len + 1 + 1 );
  strcpy(new_string, input);
  new_string[len] = c;
  new_string[len + 1] = '\0';

  return new_string;
}

char* finalize_json(JSONBuilder *builder)
{
  builder->resulting_json = (char*)malloc(builder->json_char_count - 1); // Remove extra character from last comma
  builder->resulting_json[0] = '[';
  builder->resulting_json[1] = '\0';

  char* obr = "{";
  char* ob = "[";
  char* oq = "\"";
  char* oqc = "\":";
  char* cm = ",";
  char* cbr = "}";
  char* cb = "]";
  char* end = "\0";

  unsigned long counter;
  counter = 1;

  ArrayObjectListItem* i = builder->root->value_list;
  SingleValueJSON* j = NULL;
  ArrayValueJSON* value_arrays = NULL;
  ArrayValueListItem* value_array_values = NULL;

  while (i)
  {
    // ----------------------------------------------------------------------------------------

    /*
      THIS SECTION ADDS TYPE 1S
    */
    j = i->array_object->single_values;
    memcpy(builder->resulting_json + counter, obr, 1);
    counter++;

    while (j != i->array_object->last_single_value->next_value)
    {
      memcpy(builder->resulting_json + counter, oq, 1);
      counter++;
      memcpy(builder->resulting_json + counter, j->name, j->name_characters);
      counter += j->name_characters;

      memcpy(builder->resulting_json + counter, oqc, 2);
      counter += 2;

      if (j->quoted)
      {
        memcpy(builder->resulting_json + counter, oq, 1);
        counter++;
        memcpy(builder->resulting_json + counter, j->value, j->value_characters);
        counter += j->value_characters;
        memcpy(builder->resulting_json + counter, oq, 1);
        counter++;
      }
      else
      {
        memcpy(builder->resulting_json + counter, j->value, j->value_characters);
        counter += j->value_characters;
      }

      if ((j->next_value != i->array_object->last_single_value->next_value) || i->array_object->array_values->set_flag)
      {
        memcpy(builder->resulting_json + counter, cm, 1);
        counter++;
      }
      j = j->next_value;
    }
    
    /*
      THIS SECTION ADDS TYPE 1S
    */

    // ----------------------------------------------------------------------------------------

    /*
      THIS SECTION ADDS TYPE 2S
    */
    /*
      THIS SECTION ADDS TYPE 2S
    */

    // ----------------------------------------------------------------------------------------

    /*
      THIS SECTION ADDS TYPE 3S
    */

    value_arrays = i->array_object->array_values;

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
        //if (value_arrays->next_value->set_flag)
        //{
          memcpy(builder->resulting_json + counter, cm, 1);
          counter++;
        //}
      }

      value_arrays = value_arrays->next_value;
    }
    /*
      THIS SECTION ADDS TYPE 3S
    */

    // ----------------------------------------------------------------------------------------

    /*
      THIS SECTION ADDS TYPE 4S
    */


    /*
      THIS SECTION ADDS TYPE 4S
    */

    // ----------------------------------------------------------------------------------------

    memcpy(builder->resulting_json + counter, cbr, 1);
    counter++;

    if (i->next_item)
    {
      memcpy(builder->resulting_json + counter, cm, 1);
      counter++;
    }
    else
    {
      memcpy(builder->resulting_json + counter, cb, 1);
      counter++;
    }
    i = i->next_item;
  }

  printf("Length of buffer: %lu\n", builder->json_char_count);
  memcpy(builder->resulting_json + counter, end, 1);
  return builder->resulting_json;
}
