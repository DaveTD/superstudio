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
  builder->root_level = (JSONLevelBuilder*)calloc(1, sizeof(JSONLevelBuilder)); 
  builder->root_level->next_child = (JSONLevelBuilder*)calloc(1, sizeof(JSONLevelBuilder));
  builder->root_level->next_child_array = (JSONLevelBuilder*)calloc(1, sizeof(JSONLevelBuilder));

  builder->root = (ArrayObjectJSON*)malloc(sizeof(ArrayObjectJSON));
  builder->resulting_json = NULL;

  builder->root->name = "root";
  builder->root->identifier = 0;
  builder->root->value_list = NULL;

  builder->root_level->search_list = (HashList*)malloc(sizeof(HashList));
  builder->row_count = 0;
  builder->json_char_count = 3; //Starts with "[]\0"
  builder->max_depth = 0;
  builder->max_real_depth = 0;
}

void set_row_count(JSONDocumentBuilder *builder, unsigned long count)
{
  builder->row_count = count;
  initialize_search_list(builder->root_level, count);
}

void initialize_search_list(JSONLevelBuilder* level, unsigned long count)
{
  hl_initialize(level->search_list, count);
}

unsigned long get_row_count(JSONDocumentBuilder *builder)
{
  return builder->row_count;
}

void set_repeating_array_columns(JSONDocumentBuilder *builder, unsigned long* repeating)
{
  builder->root_level->repeating_array_columns = (unsigned long*)malloc(sizeof(unsigned long) * builder->root_level->column_count);
  memcpy(builder->root_level->repeating_array_columns, repeating, sizeof(unsigned long) * builder->root_level->column_count);
}

void set_column_count(JSONDocumentBuilder *builder, unsigned long count)
{
  builder->root_level->column_count = count;
}

unsigned long get_column_count(JSONDocumentBuilder *builder)
{
  return builder->root_level->column_count;
}

void set_quote_array(JSONDocumentBuilder *builder, unsigned long* quotes)
{
  builder->root_level->quote_array = (unsigned long*)malloc(sizeof(unsigned long) * builder->root_level->column_count);
  memcpy(builder->root_level->quote_array, quotes, sizeof(unsigned long) * (builder->root_level->column_count));
}

void set_hashing_array(JSONDocumentBuilder *builder, unsigned long* do_not_hashes)
{
  builder->root_level->do_not_hash = (unsigned long*)malloc(sizeof(unsigned long) * builder->root_level->column_count);
  memcpy(builder->root_level->do_not_hash, do_not_hashes, sizeof(unsigned long) * (builder->root_level->column_count));
}

void set_depth_array(
  JSONDocumentBuilder *builder, 
  unsigned long* depths, 
  unsigned long max, 
  unsigned long* real_depths, 
  unsigned long max_real
  )
{
  builder->root_level->depth_array = (unsigned long*)malloc(sizeof(unsigned long) * builder->root_level->column_count);
  memcpy(builder->root_level->depth_array, depths, sizeof(unsigned long) * (builder->root_level->column_count));

  builder->max_depth = max;
  
  builder->root_level->real_depth_array = (unsigned long*)malloc(sizeof(unsigned long) * builder->root_level->column_count);
  memcpy(builder->root_level->real_depth_array, real_depths, sizeof(unsigned long) * (builder->root_level->column_count));
  
  builder->max_real_depth = max_real;
}

void set_single_node_key_names(JSONDocumentBuilder *builder, char** key_array, unsigned long* key_sizes)
{
  builder->single_object_key_lengths = (unsigned long*)malloc(sizeof(unsigned long) * builder->root_level->column_count);
  builder->single_object_key_names = (char**)malloc(sizeof(char*) * builder->root_level->column_count);

  memcpy(builder->single_object_key_lengths, key_sizes, sizeof(unsigned long*) * (builder->root_level->column_count));
  memcpy(builder->single_object_key_names, key_array, sizeof(char*) * (builder->root_level->column_count));
}


void set_array_node_key_names(JSONDocumentBuilder *builder, char** key_array, unsigned long* key_sizes)
{
  builder->array_object_key_lengths = (unsigned long*)malloc(sizeof(unsigned long) * builder->root_level->column_count);
  builder->array_object_key_names = (char**)malloc(sizeof(char*) * builder->root_level->column_count);

  memcpy(builder->array_object_key_lengths, key_sizes, sizeof(unsigned long*) * (builder->root_level->column_count));
  memcpy(builder->array_object_key_names, key_array, sizeof(char*) * (builder->root_level->column_count));
}

void set_column_names_sizes(JSONDocumentBuilder *builder, char** column_names, unsigned long* column_string_sizes)
{
  builder->root_level->column_name_lengths = (unsigned long*)malloc(sizeof(unsigned long) * builder->root_level->column_count);
  builder->root_level->column_names = (char**)malloc(sizeof(char*) * builder->root_level->column_count);

  memcpy(builder->root_level->column_name_lengths, column_string_sizes, sizeof(unsigned long*) * (builder->root_level->column_count));
  memcpy(builder->root_level->column_names, column_names, sizeof(char*) * (builder->root_level->column_count));
}

void set_mapping_array(JSONDocumentBuilder *builder, char** internal_map, unsigned long* internal_map_sizes)
{
  builder->root_level->mapping_array_lengths = (unsigned long*)malloc(sizeof(unsigned long) * builder->root_level->column_count);;
  builder->root_level->mapping_array = (char**)malloc(sizeof(char*) * builder->root_level->column_count);

  memcpy(builder->root_level->mapping_array_lengths, internal_map_sizes, sizeof(unsigned long) * (builder->root_level->column_count));
  memcpy(builder->root_level->mapping_array, internal_map, sizeof(char*) * (builder->root_level->column_count));
}

void consume_row(
  JSONDocumentBuilder *builder,
  char** row_strings,
  unsigned long* string_sizes,
  unsigned long accessing_depth,
  unsigned long visible_depth,
  unsigned long column_count,
  uint64_t parent_hash,
  JSONObject* parent_object,
  unsigned long* visible_depth_array,
  JSONLevelBuilder* level_definitions,
  unsigned char parent_type,
  HashList* parent_search_list
  )
{
  unsigned long counter;
  uint64_t hash;
  HashListNode* found_node;
  JSONLevelBuilder* child_levels = (JSONLevelBuilder*)calloc(1, sizeof(JSONLevelBuilder));
  JSONLevelBuilder* child_array_start = (JSONLevelBuilder*)calloc(1, sizeof(JSONLevelBuilder));

  child_levels->next_child = (JSONLevelBuilder*)calloc(1, sizeof(JSONLevelBuilder));
  child_levels->next_child_array = (JSONLevelBuilder*)calloc(1, sizeof(JSONLevelBuilder));

  child_array_start->next_child = (JSONLevelBuilder*)calloc(1, sizeof(JSONLevelBuilder));
  child_array_start->next_child_array = (JSONLevelBuilder*)calloc(1, sizeof(JSONLevelBuilder));


  if (!level_definitions) {
    //This level definition is at the root level.
    level_definitions = builder->root_level;
    parent_object = builder->root;
  }


  hash = calculate_run_hash(level_definitions, column_count, string_sizes, row_strings, accessing_depth);

  found_node = find_or_create_node(
    builder, 
    level_definitions, 
    child_levels, 
    child_array_start,
    parent_object, 
    hash, 
    parent_hash, 
    parent_type, 
    row_strings, 
    string_sizes, 
    visible_depth,
    parent_search_list);

  // I don't know if these two lines are needed - this should be obvious if the pointers are pointing to the same place anyway. Test later.
  child_levels = found_node->single_object_info_list;
  child_array_start = found_node->array_object_info_list;

  define_child_levels(builder, level_definitions, child_levels, child_array_start, hash, column_count, accessing_depth, row_strings, string_sizes);
  initialize_child_levels(child_levels);
  set_single_object_child_level_definitions(level_definitions, child_levels, hash, column_count, accessing_depth);
  assign_single_object_data(level_definitions, child_levels, row_strings, string_sizes, column_count, accessing_depth);

  initialize_child_array_levels(child_array_start);
  set_array_object_child_level_definitions(level_definitions, child_array_start, hash, column_count, accessing_depth, row_strings, string_sizes);
  assign_array_object_data(level_definitions, child_array_start, row_strings, string_sizes, column_count, accessing_depth);

  // 2s
  consume_single_objects(builder, child_levels, found_node->related_JSON_object, accessing_depth, hash, visible_depth);
  // 3s
  set_value_arrays(builder, level_definitions, hash, column_count, row_strings, string_sizes, accessing_depth);
  // 4s
  consume_array_objects(builder, child_array_start, parent_object, accessing_depth, hash, visible_depth, level_definitions->search_list);

}

void consume_single_objects(JSONDocumentBuilder* builder, JSONLevelBuilder* child_levels, JSONObject* found_object, unsigned long accessing_depth, uint64_t hash, unsigned long visible_depth)
{
  while(child_levels->set_flag) {
    consume_row(builder,
      child_levels->active_row_strings->row_strings,
      child_levels->active_row_strings->string_lengths,
      accessing_depth,
      (visible_depth + 1),
      child_levels->column_count,
      hash,
      found_object,
      child_levels->depth_array,
      child_levels,
      2,
      child_levels->search_list
      );
    child_levels = child_levels->next_child;
  }
}

void consume_array_objects(JSONDocumentBuilder* builder, 
  JSONLevelBuilder* child_array_levels, 
  JSONObject* found_object, 
  unsigned long accessing_depth, 
  uint64_t hash, 
  unsigned long visible_depth,
  HashList* parent_search_list
  )
{
  unsigned long counter;
  while(child_array_levels->set_flag) {
    counter = 0;
    while (counter < child_array_levels->column_count) {
      counter++;
    }

    consume_row(builder,
      child_array_levels->active_row_strings->row_strings,
      child_array_levels->active_row_strings->string_lengths,
      (accessing_depth + 1),
      (visible_depth + 1),
      child_array_levels->column_count,
      hash,
      found_object,
      child_array_levels->depth_array,
      child_array_levels,
      4,
      parent_search_list
      );
    child_array_levels = child_array_levels->next_child_array;
  }
}

uint64_t calculate_run_hash(JSONLevelBuilder* level_definitions, unsigned long column_count, unsigned long* string_sizes, char** row_strings, unsigned long accessing_depth)
{
  uint64_t hash = FNV_OFFSET;
  int counter = 0;
  unsigned long inner_counter = 0;

  while(counter < column_count) {
    inner_counter = 0;
    if ((!level_definitions->do_not_hash[counter]) && (accessing_depth == level_definitions->real_depth_array[counter])) {
      while (inner_counter < string_sizes[counter]) {
        hash_byte(&hash, &row_strings[counter][inner_counter]);
        inner_counter++;
      }
    }
    counter++;
  }

  return hash;
}

uint64_t calculate_identified_hash(
  JSONLevelBuilder* level_definitions,
  unsigned long column_count,
  unsigned long* string_sizes,
  char** row_strings,
  unsigned long accessing_depth,
  int test_identifier_length,
  char* test_identifier)
{
  uint64_t hash = FNV_OFFSET;
  int counter = 0;
  unsigned long inner_counter = 0;
  unsigned char cursor;
  int inner_identifier_length;
  char* inner_test_identifier;

  while(counter < column_count) {
    if (level_definitions->real_depth_array[counter] >= accessing_depth) {
      cursor = 0;
      read_identifier(accessing_depth, level_definitions->mapping_array[counter], &cursor, &inner_identifier_length, level_definitions->mapping_array_lengths[counter]);
      inner_test_identifier = malloc(test_identifier_length + 1);
      memcpy(inner_test_identifier, level_definitions->mapping_array[counter] + cursor, test_identifier_length);
      inner_test_identifier[test_identifier_length] = '\0';

      inner_counter = 0;
      if ((!level_definitions->do_not_hash[counter]) && (accessing_depth == level_definitions->real_depth_array[counter]) && (inner_identifier_length == test_identifier_length)) {
        if (!memcmp(test_identifier, inner_test_identifier, test_identifier_length)) {
          while (inner_counter < string_sizes[counter]) {
            hash_byte(&hash, &row_strings[counter][inner_counter]);
            inner_counter++;
          }
        }
      }
    }
    counter++;
  }

  return hash;
}

char read_type(unsigned long depth_start, char* mapped_value, unsigned long mapped_value_length)
{
  unsigned int cursor = 0;
  unsigned long depth_counter = 0;

  if (depth_start > 0) {
    while (depth_counter < depth_start) {
      if (cursor >= mapped_value_length) {
        // Returns 0 if the mapped value doesn't go deep enough
        return 0;
      }
      else if (mapped_value[cursor] == '-') {
        depth_counter++;
      }
      cursor++;
    }
  }
  return mapped_value[cursor];
}

void read_identifier(char depth_start, char* mapped_value, int* cursor, int* identifier_characters, unsigned long mapped_value_length)
{
  *cursor = 0;
  unsigned char end_cursor = 0;
  *identifier_characters = 0;
  char depth_counter = 0;
  
  if (depth_start > 0) {
    while (depth_counter < depth_start) {
      if (mapped_value[*cursor] == '-') {
        depth_counter++;
      }
      *cursor += 1;
    }
  }

  *cursor += 2;
  end_cursor = *cursor;

  while (mapped_value[end_cursor] != '-' && end_cursor < mapped_value_length) {
    *identifier_characters += 1;
    end_cursor++;
  }
}

void define_child_levels(
  JSONDocumentBuilder *builder, 
  JSONLevelBuilder* level_definitions, 
  JSONLevelBuilder* child_levels, 
  JSONLevelBuilder* child_array_start, 
  uint64_t hash, 
  unsigned long column_count, 
  unsigned long accessing_depth, 
  char** row_strings, 
  unsigned long* string_sizes)
{
  unsigned long counter = 0;
  int test_identifier_length;
  int identifier_int;
  int cursor;
  char* test_identifier;
  char type;
  uint64_t child_hash;


  while(counter < column_count) {
    if ((!level_definitions->do_not_hash[counter]) && !level_definitions->defined_flag)
    {
      type = read_type(accessing_depth, level_definitions->mapping_array[counter], level_definitions->mapping_array_lengths[counter]);
      read_identifier(accessing_depth, level_definitions->mapping_array[counter], &cursor, &test_identifier_length, level_definitions->mapping_array_lengths[counter]);
      test_identifier = malloc(test_identifier_length + 1);
      memcpy(test_identifier, level_definitions->mapping_array[counter] + cursor, test_identifier_length);
      test_identifier[test_identifier_length] = '\0';
      identifier_int = atoi(test_identifier);

      if (type == '2') {
        count_increment_or_create_json_level_child(builder, child_levels, test_identifier_length, test_identifier, identifier_int, hash);
      }
      if (type == '4') {
        child_hash = calculate_identified_hash(level_definitions, column_count, string_sizes, row_strings, (accessing_depth + 1), test_identifier_length, test_identifier);
        count_increment_or_create_json_level_child_array(builder, child_array_start, test_identifier_length, test_identifier, identifier_int, hash, child_hash);
      }
    }
    else if ((level_definitions->real_depth_array[counter] == accessing_depth) && (level_definitions->do_not_hash[counter]) && (!level_definitions->defined_flag)) { 
      level_definitions->contains_array_value_flag = 1; //Has a type 3
    }

    counter++;
  }
}

char* finalize_json(JSONDocumentBuilder *builder)
{
  unsigned long counter;

  builder->json_char_count += 100000;

  builder->resulting_json = malloc(builder->json_char_count - 1); // Remove extra character from last comma
  counter = 0;
  counter = finalize_object_array(builder, builder->root, counter);

  printf("Length of buffer: %lu\n", builder->json_char_count - 1);
  memcpy(builder->resulting_json + counter, end, 1);
  return builder->resulting_json;
}