#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "json_builder.h"
#include "json_value.h"
#include "json_value_array.h"
#include "json_single_object.h"
#include "json_object_array.h"
#include "hash_linked_list.h"
#include "ss_alloc.h"
#include "fnv_64.h"

void json_builder_initialize(JSONDocumentBuilder *builder)
{
  builder->memory_stack = calloc(1, sizeof(SSMemoryStack));
  builder->memory_stack->stack_top = calloc(1, sizeof(SSMemoryStackNode));

  builder->root_level = (JSONLevelBuilder*)ss_alloc(builder->memory_stack, 1, sizeof(JSONLevelBuilder)); 
  builder->root_level->next_child = (JSONLevelBuilder*)ss_alloc(builder->memory_stack, 1, sizeof(JSONLevelBuilder));
  builder->root_level->next_child_array = (JSONLevelBuilder*)ss_alloc(builder->memory_stack, 1, sizeof(JSONLevelBuilder));

  builder->root = (ArrayObjectJSON*)ss_alloc(builder->memory_stack, 1, sizeof(ArrayObjectJSON));

  builder->root->next = (ArrayObjectJSON*)ss_alloc(builder->memory_stack, 1, sizeof(ArrayObjectJSON));

  builder->resulting_json = NULL;
  builder->root->identifier_int = 0;
  builder->root->value_list = NULL;
  builder->root->set_flag = 0;

  builder->root_level->search_list = (HashList*)ss_alloc(builder->memory_stack, 1, sizeof(HashList));

  builder->row_count = 0;
  builder->json_char_count = 3; //Starts with "[]\0"
  builder->max_depth = 0;
  builder->max_real_depth = 0;
}

void set_row_count(JSONDocumentBuilder *builder, unsigned long count)
{
  builder->row_count = count;
  initialize_search_list(builder->root_level, builder->memory_stack, count);
}

void initialize_search_list(JSONLevelBuilder* level, SSMemoryStack* memory_stack, unsigned long count)
{
  hl_initialize(level->search_list, memory_stack, count);
}

unsigned long get_row_count(JSONDocumentBuilder *builder)
{
  return builder->row_count;
}

void set_repeating_array_columns(JSONDocumentBuilder *builder, unsigned long* repeating)
{
  builder->root_level->repeating_array_columns = (unsigned long*)ss_alloc(builder->memory_stack, 1, sizeof(unsigned long) * builder->root_level->column_count);
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
  builder->root_level->quote_array = (unsigned long*)ss_alloc(builder->memory_stack, 1, sizeof(unsigned long) * builder->root_level->column_count);
  memcpy(builder->root_level->quote_array, quotes, sizeof(unsigned long) * (builder->root_level->column_count));
}

void set_hashing_array(JSONDocumentBuilder *builder, unsigned long* do_not_hashes)
{
  builder->root_level->do_not_hash = (unsigned long*)ss_alloc(builder->memory_stack, 1, sizeof(unsigned long) * builder->root_level->column_count);
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
  builder->root_level->depth_array = (unsigned long*)ss_alloc(builder->memory_stack, 1, sizeof(unsigned long) * builder->root_level->column_count);
  memcpy(builder->root_level->depth_array, depths, sizeof(unsigned long) * (builder->root_level->column_count));

  builder->max_depth = max;
  
  builder->root_level->real_depth_array = (unsigned long*)ss_alloc(builder->memory_stack, 1, sizeof(unsigned long) * builder->root_level->column_count);
  memcpy(builder->root_level->real_depth_array, real_depths, sizeof(unsigned long) * (builder->root_level->column_count));
  
  builder->max_real_depth = max_real;
}

void set_single_node_key_names(JSONDocumentBuilder *builder, char** key_array, unsigned long* key_sizes)
{
  builder->single_object_key_lengths = (unsigned long*)ss_alloc(builder->memory_stack, 1, sizeof(unsigned long) * builder->root_level->column_count);
  builder->single_object_key_names = (char**)ss_alloc(builder->memory_stack, 1, sizeof(char*) * builder->root_level->column_count);

  memcpy(builder->single_object_key_lengths, key_sizes, sizeof(unsigned long*) * (builder->root_level->column_count));
  memcpy(builder->single_object_key_names, key_array, sizeof(char*) * (builder->root_level->column_count));
}


void set_array_node_key_names(JSONDocumentBuilder *builder, char** key_array, unsigned long* key_sizes)
{
  builder->array_object_key_lengths = (unsigned long*)ss_alloc(builder->memory_stack, 1, sizeof(unsigned long) * builder->root_level->column_count);
  builder->array_object_key_names = (char**)ss_alloc(builder->memory_stack, 1, sizeof(char*) * builder->root_level->column_count);

  memcpy(builder->array_object_key_lengths, key_sizes, sizeof(unsigned long*) * (builder->root_level->column_count));
  memcpy(builder->array_object_key_names, key_array, sizeof(char*) * (builder->root_level->column_count));
}

void set_column_names_sizes(JSONDocumentBuilder *builder, char** column_names, unsigned long* column_string_sizes)
{
  builder->root_level->column_name_lengths = (unsigned long*)ss_alloc(builder->memory_stack, 1, sizeof(unsigned long) * builder->root_level->column_count);
  builder->root_level->column_names = (char**)ss_alloc(builder->memory_stack, 1, sizeof(char*) * builder->root_level->column_count);

  memcpy(builder->root_level->column_name_lengths, column_string_sizes, sizeof(unsigned long*) * (builder->root_level->column_count));
  memcpy(builder->root_level->column_names, column_names, sizeof(char*) * (builder->root_level->column_count));
}

void set_mapping_array(JSONDocumentBuilder *builder, char** internal_map, unsigned long* internal_map_sizes)
{
  builder->root_level->mapping_array_lengths = (unsigned long*)ss_alloc(builder->memory_stack, 1, sizeof(unsigned long) * builder->root_level->column_count);;
  builder->root_level->mapping_array = (char**)ss_alloc(builder->memory_stack, 1, sizeof(char*) * builder->root_level->column_count);

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
  JSONObject* parent_object,
  JSONLevelBuilder* level_definitions,
  unsigned char parent_type,
  int parent_identifier,
  HashList* parent_search_list
  )
{
  uint64_t hash;
  JSONObject* found_object;
  JSONLevelBuilder* child_levels = (JSONLevelBuilder*)ss_alloc(builder->memory_stack, 1, sizeof(JSONLevelBuilder));
  JSONLevelBuilder* child_array_start = (JSONLevelBuilder*)ss_alloc(builder->memory_stack, 1, sizeof(JSONLevelBuilder));

  child_levels->next_child = (JSONLevelBuilder*)ss_alloc(builder->memory_stack, 1, sizeof(JSONLevelBuilder));
  child_levels->next_child_array = (JSONLevelBuilder*)ss_alloc(builder->memory_stack, 1, sizeof(JSONLevelBuilder));

  child_array_start->next_child = (JSONLevelBuilder*)ss_alloc(builder->memory_stack, 1, sizeof(JSONLevelBuilder));
  child_array_start->next_child_array = (JSONLevelBuilder*)ss_alloc(builder->memory_stack, 1, sizeof(JSONLevelBuilder));

  if (accessing_depth == 0 && visible_depth == 0) {
    level_definitions = builder->root_level;
  }

  hash = calculate_run_hash(level_definitions, column_count, string_sizes, row_strings, accessing_depth, (accessing_depth + visible_depth));

  found_object = find_or_create_node(
    builder, 
    level_definitions, 
    child_levels, 
    child_array_start,
    parent_object, 
    hash, 
    parent_type, 
    row_strings, 
    string_sizes, 
    (visible_depth + accessing_depth),
    parent_search_list);

  define_child_levels(builder, level_definitions, child_levels, hash, column_count, (accessing_depth + visible_depth));

  initialize_child_levels(child_levels, builder->memory_stack);
  set_single_object_child_level_definitions(level_definitions, child_levels, hash, column_count, (accessing_depth + visible_depth));
  assign_single_object_data(level_definitions, child_levels, row_strings, string_sizes, column_count, (accessing_depth + visible_depth));

  define_child_array_levels(builder, level_definitions, child_array_start, hash, column_count, accessing_depth, visible_depth, row_strings, string_sizes);
  initialize_child_array_levels(child_array_start, builder->memory_stack);
  set_array_object_child_level_definitions(level_definitions, child_array_start, hash, column_count, (accessing_depth + visible_depth));
  assign_array_object_data(level_definitions, child_array_start, row_strings, string_sizes, column_count, (accessing_depth + visible_depth));

  if (found_object) {
    add_or_find_single_object_child_hashes(builder,
      child_levels,
      found_object,
      parent_object,
      hash,
      parent_type,
      parent_identifier);

    add_or_find_array_object_child_hashes(builder, 
      level_definitions, 
      child_array_start, 
      accessing_depth,
      visible_depth, 
      column_count, 
      string_sizes, 
      row_strings, 
      found_object
      );

    consume_single_objects(
      builder, 
      child_levels, 
      found_object,
      parent_search_list,
      accessing_depth, 
      visible_depth
      );

    consume_array_objects(
      builder, 
      child_array_start, 
      found_object,
      accessing_depth, 
      visible_depth 
      );
  }
}

void consume_single_objects (
  JSONDocumentBuilder* builder, 
  JSONLevelBuilder* child_levels, 
  JSONObject* found_object, 
  HashList* parent_search_list,
  unsigned long accessing_depth, 
  unsigned long visible_depth
  )
{
  while(child_levels->set_flag && parent_search_list->length) {
    if (found_object->single_objects->set_flag) {
      consume_row(builder,
        child_levels->active_row_strings->row_strings,
        child_levels->active_row_strings->string_lengths,
        accessing_depth,
        (visible_depth + 1),
        child_levels->column_count,
        found_object,
        child_levels,
        2,
        child_levels->identifier_int,
        parent_search_list
        );
    } else {
      consume_row(builder,
        child_levels->active_row_strings->row_strings,
        child_levels->active_row_strings->string_lengths,
        accessing_depth,
        (visible_depth + 1),
        child_levels->column_count,
        found_object,
        child_levels,
        2,
        child_levels->identifier_int,
        parent_search_list
        );      
    }
    child_levels = child_levels->next_child;
  }
}

void consume_array_objects(JSONDocumentBuilder* builder, 
  JSONLevelBuilder* child_array_levels, 
  JSONObject* found_object, 
  unsigned long accessing_depth, 
  unsigned long visible_depth)
{
  while(child_array_levels->set_flag) {
    consume_row(builder,
      child_array_levels->active_row_strings->row_strings,
      child_array_levels->active_row_strings->string_lengths,
      (accessing_depth + 1),
      visible_depth,
      child_array_levels->column_count,
      found_object,
      child_array_levels,
      4,
      child_array_levels->identifier_int,
      child_array_levels->search_list
      );

    child_array_levels = child_array_levels->next_child_array;
  }
}

uint64_t calculate_run_hash(JSONLevelBuilder* level_definitions, unsigned long column_count, unsigned long* string_sizes, char** row_strings, unsigned long accessing_depth, unsigned long hashing_depth)
{
  uint64_t hash = FNV_OFFSET;
  unsigned long counter = 0;
  unsigned long inner_counter = 0;

  while(counter < column_count) {
    inner_counter = 0;

    if ((!level_definitions->do_not_hash[counter]) && (level_definitions->real_depth_array[counter] == accessing_depth)
      && (hashing_depth <= level_definitions->depth_array[counter]))
    {  
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
  unsigned long hashing_depth,
  int test_identifier_length,
  char* test_identifier)
{
  uint64_t hash = FNV_OFFSET;
  unsigned long counter = 0;
  unsigned long inner_counter = 0;
  int cursor;
  int inner_identifier_length;
  char* inner_test_identifier;

  while(counter < column_count) {
    if (level_definitions->real_depth_array[counter] == accessing_depth) {
      cursor = 0;

      read_identifier((accessing_depth - 1), level_definitions->mapping_array[counter], &cursor, &inner_identifier_length, level_definitions->mapping_array_lengths[counter]);
      inner_test_identifier = level_definitions->mapping_array[counter] + cursor;

      // TODO: check to see if we're looking at a 4, only hash on 4s
      inner_counter = 0;
      if ((!level_definitions->do_not_hash[counter]) 
        && (hashing_depth <= level_definitions->depth_array[counter]) 
        && (inner_identifier_length == test_identifier_length)) {
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

char read_type(unsigned long depth_start, 
  char* mapped_value, 
  unsigned long mapped_value_length)
{
  unsigned long cursor = 0;
  unsigned long depth_counter = 0;

  if (depth_start > 0) {
    while (depth_counter < depth_start) {
      if (cursor >= mapped_value_length) {
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

void read_identifier(char depth_start, 
  char* mapped_value, 
  int* cursor, 
  int* identifier_characters, 
  unsigned long mapped_value_length)
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
  uint64_t hash, 
  unsigned long column_count, 
  unsigned long visible_depth
  )
{
  unsigned long counter = 0;
  int test_identifier_length;
  int identifier_int;
  int cursor;
  char* test_identifier;
  char type;

  while(counter < column_count) {
    type = read_type(visible_depth, level_definitions->mapping_array[counter], level_definitions->mapping_array_lengths[counter]);
    read_identifier(visible_depth, level_definitions->mapping_array[counter], &cursor, &test_identifier_length, level_definitions->mapping_array_lengths[counter]);
    
    test_identifier = ss_alloc(builder->memory_stack, 1, test_identifier_length + 1);
    memcpy(test_identifier, level_definitions->mapping_array[counter] + cursor, test_identifier_length);
    test_identifier[test_identifier_length] = '\0';
    identifier_int = atoi(test_identifier);

    if (type == '2') {
      count_increment_or_create_json_level_child(builder, child_levels, test_identifier_length, test_identifier, identifier_int, hash);
    }
    if ((level_definitions->real_depth_array[counter] == visible_depth) && (level_definitions->do_not_hash[counter]) && (!level_definitions->defined_flag)) { 
      level_definitions->contains_array_value_flag = 1;
    }
    counter++;
  }
}

void define_child_array_levels(
  JSONDocumentBuilder *builder, 
  JSONLevelBuilder* level_definitions, 
  JSONLevelBuilder* child_array_start, 
  uint64_t hash, 
  unsigned long column_count, 
  unsigned long accessing_depth, 
  unsigned long visible_depth, 
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
      type = read_type(accessing_depth + visible_depth, level_definitions->mapping_array[counter], level_definitions->mapping_array_lengths[counter]);
      read_identifier(accessing_depth + visible_depth, level_definitions->mapping_array[counter], &cursor, &test_identifier_length, level_definitions->mapping_array_lengths[counter]);

      test_identifier = ss_alloc(builder->memory_stack, 1, test_identifier_length + 1);
      memcpy(test_identifier, level_definitions->mapping_array[counter] + cursor, test_identifier_length);
      test_identifier[test_identifier_length] = '\0';
      identifier_int = atoi(test_identifier);

      if (type == '4') {
        child_hash = calculate_identified_hash(level_definitions, column_count, string_sizes, row_strings, (accessing_depth + 1), (accessing_depth + visible_depth + 1), test_identifier_length, test_identifier);
        count_increment_or_create_json_level_child_array(builder, child_array_start, test_identifier_length, test_identifier, identifier_int, hash, child_hash);
      }

    if ((level_definitions->real_depth_array[counter] == accessing_depth) && (level_definitions->do_not_hash[counter]) && (!level_definitions->defined_flag)) { 
      level_definitions->contains_array_value_flag = 1;
    }
    counter++;
  }
}

char* finalize_json(JSONDocumentBuilder *builder)
{
  unsigned long counter;

  builder->resulting_json = ss_alloc(builder->memory_stack, 1, builder->json_char_count - 1); // Remove extra character from last comma
  counter = 0;
  memcpy(builder->resulting_json + counter, ob, 1);
  counter++;
  counter = finalize_object_array(builder, builder->root, counter);

  memcpy(builder->resulting_json + counter, end, 1);
  return builder->resulting_json;
}