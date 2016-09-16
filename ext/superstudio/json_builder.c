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

void set_depth_array(JSONBuilder *builder, unsigned long* depths, unsigned long max, unsigned long* real_depths, unsigned long max_real)
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

  // unsigned long counter = 0;
  // while(counter < builder->column_count)
  // {
  //   printf("Mapping array value: %s\n", builder->mapping_array[counter]);
  //   counter++;
  // }
}

void consume_row(JSONBuilder *builder, char** row, unsigned long* string_sizes, unsigned long accessing_depth, unsigned long column_count, uint64_t parent_hash)
{
  /*
  *   When accessing_depth is 0, we're looking at the root. This would be the only depth that
  *   we should look at creating an entirely new struture - otherwise we should be
  *   able to find another structure that this row can add new data into.
  */
  uint64_t hash = FNV_OFFSET;
  unsigned long counter = 0;
  unsigned long inner_counter = 0;
  HashListNode* found_node;

  while(counter < column_count)
  {
    inner_counter = 0;
    while (inner_counter < string_sizes[counter])
    {
      // Make sure this column is the correct depth, and should be hashed
      if (builder->real_depth_array[counter] == accessing_depth) && !(builder->do_not_hash[counter])
      {
        hash = fnv_hash_byte(hash, &row[counter][inner_counter]);
      }
      inner_counter++;
    }
    counter++;
  }

  found_node = hl_find_node(builder->search_list, hash);
  if(found_node == NULL)
  {
    if(accessing_depth == 0)
    {
      create_new_root_item(builder, hash, row, string_sizes, NULL);
    }
    else
    {
      // Find your parent node
      parent_node = hl_find_node(builder->search_list, parent_hash);
      // Determine what kind this is

      // Insert
    }
  }
  else
  {
    // Split out the smaller parts at the next depth, call consume_row on those with accessing_depth + 1
    // Skip forward accessing depth -1 dashes (or none of accessing depth is 0)
    // Skip forward 2 characters
    // Read until the next dash - this is where this item belongs
    // Group those values together, and call consume row with those subsets
  }

}

void create_new_root_item(JSONBuilder* builder, uint64_t hash, char** row, unsigned long* string_sizes)
{
  hl_insert_or_find(builder->search_list, hash);

  //Create a new JSON Object

  //For each non-4 object, create and insert

  //Go through each type-4 object, call adding their stuff, as deep as it goes

}

char* read_value(char depth_start, char is_first, char* mapped_value)
{
  char* return_string;
  char starting_character = 0;
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
    return_string[0] = mapped_value[++starting_character];
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
