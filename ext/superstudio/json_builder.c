#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "json_builder.h"
#include "json_value.h"
#include "json_value_array.h"
#include "json_object_array.h"
#include "hash_linked_list.h"
#include "fnv_64.h"

void json_builder_initialize(JSONDocumentBuilder *builder)
{
  builder->mapping_array = NULL;
  builder->quote_array = NULL;
  builder->do_not_hash = NULL;
  builder->depth_array = NULL;
  builder->real_depth_array = NULL;
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

void set_row_count(JSONDocumentBuilder *builder, unsigned long count)
{
  builder->row_count = count;
  hl_initialize(builder->search_list, count);
}

unsigned long get_row_count(JSONDocumentBuilder *builder)
{
  return builder->row_count;
}

void set_repeating_array_columns(JSONDocumentBuilder *builder, unsigned long* repeating)
{
  builder->repeating_array_columns = (unsigned long*)malloc(sizeof(unsigned long) * builder->column_count);
  memcpy(builder->repeating_array_columns, repeating, sizeof(unsigned long) * builder->column_count);
}

void set_column_count(JSONDocumentBuilder *builder, unsigned long count)
{
  builder->column_count = count;
}

unsigned long get_column_count(JSONDocumentBuilder *builder)
{
  return builder->column_count;
}

void set_quote_array(JSONDocumentBuilder *builder, unsigned long* quotes)
{
  builder->quote_array = (unsigned long*)malloc(sizeof(unsigned long) * builder->column_count);
  memcpy(builder->quote_array, quotes, sizeof(unsigned long) * (builder->column_count));
}

void set_hashing_array(JSONDocumentBuilder *builder, unsigned long* do_not_hashes)
{
  builder->do_not_hash = (unsigned long*)malloc(sizeof(unsigned long) * builder->column_count);
  memcpy(builder->do_not_hash, do_not_hashes, sizeof(unsigned long) * (builder->column_count));
}

void set_depth_array(
  JSONDocumentBuilder *builder, 
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

void set_column_names_sizes(JSONDocumentBuilder *builder, char** column_names, unsigned long* column_string_sizes)
{
  builder->column_name_lengths = (unsigned long*)malloc(sizeof(unsigned long) * builder->column_count);
  builder->column_names = (char**)malloc(sizeof(char*) * builder->column_count);

  memcpy(builder->column_name_lengths, column_string_sizes, sizeof(unsigned long*) * (builder->column_count));
  memcpy(builder->column_names, column_names, sizeof(char*) * (builder->column_count));
}

void set_mapping_array(JSONDocumentBuilder *builder, char** internal_map)
{
  builder->mapping_array = (char**)malloc(sizeof(char*) * builder->column_count);
  memcpy(builder->mapping_array, internal_map, sizeof(char*) * (builder->column_count));

  unsigned int counter = 0;
  char* column_mapping;
  unsigned int sub_column_indentifiers[builder->column_count]; // Max possible size
  unsigned int sub_column_count = 0; // Actual count

  // unsigned long counter = 0;
  // while(counter < builder->column_count)
  // {
  //   printf("Mapping array value: %s\n", builder->mapping_array[counter]);
  //   counter++;
  // }
}

void consume_row(
  JSONDocumentBuilder *builder,
  char** row,
  unsigned long* string_sizes,
  unsigned long accessing_depth,
  unsigned long column_count,
  uint64_t parent_hash,
  unsigned long* visible_depth_array
  )
{
  uint64_t hash = FNV_OFFSET;
  unsigned long counter = 0;
  unsigned long inner_counter = 0;
  unsigned int dnh_counter = 0;
  HashListNode* found_node;
  HashListNode* parent_node;
  JSONObject* parent_object;

  while(counter < column_count)
  {
    inner_counter = 0;
    if ((builder->real_depth_array[counter] == accessing_depth) && (!builder->do_not_hash[counter]))
    {
      while (inner_counter < string_sizes[counter])
      {
        hash = fnv_hash_byte(hash, &row[counter][inner_counter]);
        inner_counter++;
      }
    }
    else if ((builder->real_depth_array[counter] == accessing_depth) && (builder->do_not_hash[counter]))
    {
      dnh_counter++;
    }
    else if(builder->real_depth_array[counter] > accessing_depth)
    {
      // Build out indicator arrays for next deepest definitions to send off later      
      // This new array will contain the second number after the "4.", which will make it easier to access which items go to which next-level row consume
      
      /*
        "read_identifier" is going to be used here, something like:
        variable = read_identifier(accessing_depth + 1, builder->mapping_array[counter]);
        check struct list - identifier = variable, set array at this counter to 1 
      */
    }
    counter++;
  }

  found_node = hl_find_node(builder->search_list, hash);
  if(found_node == NULL)
  {
    builder->json_char_count += 3; // Starting, ending brace and comma
    create_array_object(builder, hash, 0);
    set_key_values(builder, string_sizes, row, hash, builder->column_count);
  }

  counter = 0;
  // Find 3s, add those in
  if(dnh_counter)
  {
    while (counter < column_count)
    {
      if ((builder->real_depth_array[counter] == accessing_depth) && (builder->do_not_hash[counter]))
      {
        add_to_array(builder, hash, row[counter], string_sizes[counter], builder->column_names[counter], builder->column_name_lengths[counter], builder->repeating_array_columns[counter],builder->quote_array[counter]);
      }
      counter++;
    }
  }

  /*
    For each struct in the identifiers list:
    From the identifiers that were read, construct new json-building structs to be consumed

    Consume the structs
  */

}

char* read_identifier(char depth_start, char* mapped_value)
{
  char* return_string;
  unsigned char cursor = 0;
  unsigned char identifier_characters = 1;
  char depth_counter = 0;
  
  if (depth_start > 0)
  {
    while (depth_counter < depth_start)
    {
      if (cursor > strlen(mapped_value))
      {
        // Returns 0 if the mapped value doesn't go deep enough
        return 0;
      }
      else if (mapped_value[cursor] == '-')
      {
        depth_counter++;
      }
      cursor++;
    }
  }

  cursor += 2;

  while (mapped_value[cursor] != '-')
  {
    identifier_characters++;
  }

  return_string = (char*)malloc(identifier_characters);
  // There's probably a pointer problem with second argument here
  memcpy(return_string, mapped_value[cursor], identifier_characters);
  return return_string;
}

char* finalize_json(JSONDocumentBuilder *builder)
{
  builder->resulting_json = (char*)malloc(builder->json_char_count - 1); // Remove extra character from last comma

  unsigned long counter;
  counter = 0;

  counter = finalize_object_array(builder, builder->root, counter);

  printf("Length of buffer: %lu\n", builder->json_char_count);
  memcpy(builder->resulting_json + counter, end, 1);
  return builder->resulting_json;
}
