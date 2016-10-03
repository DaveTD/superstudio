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
  builder->root = (ArrayObjectJSON*)malloc(sizeof(ArrayObjectJSON));
  builder->search_list = (HashList*)malloc(sizeof(HashList));
  builder->row_count = 0;
  builder->column_count = 0;
  builder->json_char_count = 2; //Starts with "{}"
  builder->max_depth = 0;
  builder->max_real_depth = 0;
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
  builder->quote_array = quotes;

  // unsigned long counter = 0;
  // while(counter < builder->column_count)
  // {
  //   printf("Mapping array value: %lu\n", builder->quote_array[counter]);
  //   counter++;
  // }
}

void set_hashing_array(JSONBuilder *builder, unsigned long* do_not_hashes)
{
  builder->do_not_hash = do_not_hashes;
}

void set_depth_array(
  JSONBuilder *builder, 
  unsigned long* depths, 
  unsigned long max, 
  unsigned long* real_depths, 
  unsigned long max_real
  )
{
  builder->depth_array = depths;
  builder->max_depth = max;
  builder->real_depth_array = real_depths;
  builder->max_real_depth = max_real;

  // unsigned long counter = 0;
  // while(counter < builder->column_count)
  // {
  //   printf("Mapping depth value: %lu\n", builder->depth_array[counter]);
  //   counter++;
  // }
}

void set_mapping_array(JSONBuilder *builder, char** internal_map)
{
  builder->mapping_array = internal_map;
  unsigned int counter = 0;
  char* column_mapping;
  unsigned int sub_column_indentifiers[builder->column_count]; // Max possible size
  unsigned int sub_column_count = 0; // Actual count

  // Recursively set sub-mapping arrays here
  // builder->sub_rows
  
  // Get a list of the type 4s
  while (counter < builder->column_count)
  {
    printf("%c\n", internal_map[counter][0]);
    if (internal_map[counter][0] == '4')
    {
      column_mapping = internal_map[counter];
      printf("%s\n", column_mapping);
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
  *   Variables for when arrays are found
  */
  char* sub_array_row_items[column_count];
  unsigned long sub_array_column_count = 0;
  unsigned long sub_array_string_sizes[column_count];
  unsigned long sub_array_visible_depth_array[column_count];
  /*
  *   Variables for when arrays are found
  */

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
    inner_counter = 0;
    if ((builder->real_depth_array[counter] == accessing_depth) && (!builder->do_not_hash[counter]))
    {
      while (inner_counter < string_sizes[counter])
      {
        // Make sure this column is the correct depth, and should be hashed
        hash = fnv_hash_byte(hash, &row[counter][inner_counter]);
        inner_counter++;
      }

      // We need to build a set for inserting here - inserting is done by VISIBLE depth, not REAL depth
      sub_object_string_sizes[sub_array_column_count] = string_sizes[counter];
      sub_array_row_items[sub_array_column_count] = row[counter];
      sub_array_visible_depth_array[sub_array_column_count] = visible_depth_array[counter];
      sub_array_column_count++;
    }
    else if ((builder->real_depth_array[counter] == accessing_depth) && (builder->do_not_hash[counter]))
    {
      dnh_counter++;
      // Similarly, add this item to the hash of stuff at this depth, which will only be used if we have type 3s in here, becuase we need to check where those actually go
      sub_object_string_sizes[sub_array_column_count] = string_sizes[counter];
      sub_array_row_items[sub_array_column_count] = row[counter];
      sub_array_visible_depth_array[sub_array_column_count] = visible_depth_array[counter];
      sub_array_column_count++;
    }
    else if(builder->real_depth_array[counter] != accessing_depth)
    {
      // Build out the next deepest definitions to send off later      
      // This new array will contain the second number after the "4.", which will make it easier to access which items go to which next-level row consume
      // Start forwards 2 characters
      inner_counter = 2;
      char identifier[string_sizes[counter] - 2];

      while(inner_counter < string_sizes[counter])
      {
        // Is this efficient?
        identifier[inner_counter - 2] = builder->mapping_array[counter][inner_counter];
        inner_counter++;
      }

      // Check if idendifier exists as the identifier to any of the RowBuilders in the JSONBuilder

      // sub_object_column_count++;
    }
    counter++;
  }

  found_node = hl_find_node(builder->search_list, hash);
  if(found_node == NULL)
  {
    if(accessing_depth == 0)
    {
      create_new_root_item(builder, hash, row, string_sizes, 0);
    }
    else
    {
      // Find your parent node
      parent_node = hl_find_node(builder->search_list, parent_hash);
      parent_object = parent_node->related_JSON_object;
      hl_insert_or_find(builder->search_list, hash);

      insert_for_parent(builder, accessing_depth, visible_depth_array, parent_object);

      // Find deeper items (4s), recursive party
      if(sub_object_column_count)
      {
        // Send this for each set of 4s found from before this section starts
        sub_build(builder, sub_object_row, sub_object_string_sizes, next_depth, sub_object_column_count, sub_object_groups, parent_object, visible_depth_array);
      }
    }
  }
  else
  {
    // In this case the node was found exactly
    // Find 3s, add those in
    if(dnh_counter)
    {
      array_build(builder, sub_array_row_items, sub_array_string_sizes, accessing_depth, sub_array_column_count, parent_object, dnh_counter, sub_array_visible_depth_array); 
    }
  }
}

void insert_for_parent(
  JSONBuilder* builder, 
  unsigned int accessing_depth, 
  unsigned long* sub_array_visible_depth_array,
  JSONObject* parent_object
  )
{


}


void array_build(
  JSONBuilder* builder, 
  char** sub_array_row_items, 
  unsigned long* sub_array_string_sizes, 
  unsigned int accessing_depth, // Keep in mind, this is the accessing depth, which is based on real depth, not viewed depth
  unsigned long sub_array_column_count, 
  JSONObject* parent_object, 
  unsigned int dnh_counter, 
  unsigned long* sub_array_visible_depth_array
  )
{
  // Add in the additional size to the final string size here too, when something gets added
  // Superstudio doesn't allow duplicates, as we loop through, if the string values are the same, just break
  unsigned int counter = 0;
  while (counter < sub_array_column_count)
  {



    counter++;
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
  hl_insert_or_find(builder->search_list, hash);

  //Create a new JSON Object

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
  //print_list_details(builder->search_list);
  printf("Nodes: %i\n", builder->search_list->length);
  return "This is from inside";
}
