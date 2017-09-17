#include "json_single_object.h"
#include "hash_linked_list.h"

void create_single_object(JSONDocumentBuilder* builder, 
  uint64_t hash, 
  uint64_t parent_hash, 
  int identifier_int, 
  JSONLevelBuilder* empty_child_object, 
  JSONLevelBuilder* empty_child_array, 
  JSONLevelBuilder* parent_level, 
  HashList* parent_search_list,
  JSONObject* parent_object)
{
  SingleObjectJSON* adding_object = NULL;
  SingleObjectJSON* single_parent_object;

  int found = 0;

  adding_object = parent_object->single_objects;
  while (adding_object->set_flag && !found) {
    single_parent_object = adding_object;
    if (single_parent_object->identifier_int == identifier_int) {
      found = 1;
    } else {
      single_parent_object = single_parent_object->value->single_objects;
      while (single_parent_object->set_flag && !found) {
        if (single_parent_object->identifier_int == identifier_int) {
          found = 1;
        } else {
          single_parent_object = single_parent_object->next_item;
        }
      }
    }
    adding_object = adding_object->next_item;
  }

  if (!found) {
    adding_object->next_item = (SingleObjectJSON*)calloc(1, sizeof(SingleObjectJSON));
    adding_object->name_characters = builder->single_object_key_lengths[identifier_int];
    adding_object->name = (char*)calloc(1, builder->single_object_key_lengths[identifier_int]);
    memcpy(adding_object->name, builder->single_object_key_names[identifier_int], builder->single_object_key_lengths[identifier_int]);

    adding_object->value = (JSONObject*)calloc(1, sizeof(JSONObject));
    adding_object->value->single_values = (SingleValueJSON*)calloc(1, sizeof(SingleValueJSON));
    adding_object->value->single_objects = (SingleObjectJSON*)calloc(1, sizeof(SingleObjectJSON));
    adding_object->value->array_values = (ArrayValueJSON*)calloc(1, sizeof(ArrayValueJSON));
    adding_object->value->array_objects = (ArrayObjectJSON*)calloc(1, sizeof(ArrayObjectJSON));
    adding_object->value->array_objects->next = (ArrayObjectJSON*)calloc(1, sizeof(ArrayObjectJSON));
    
    adding_object->next_item->value = (JSONObject*)calloc(1, sizeof(JSONObject));
    adding_object->identifier_int = identifier_int;
    adding_object->associated_hash = hash;

    adding_object->value->last_single_value = adding_object->value->single_values;
    adding_object->value->last_single_object = adding_object->value->single_objects;

    adding_object->set_flag = 1;

    builder->json_char_count += adding_object->name_characters;
  }
}

void count_increment_or_create_json_level_child(JSONDocumentBuilder* builder, JSONLevelBuilder* child_levels, int identifier_length, char* identifier, int identifier_int, uint64_t parent_hash)
{
  unsigned char found = 0;

  while(child_levels->set_flag && !found) {
    if (child_levels->identifier_length == identifier_length) {
      if (!memcmp(child_levels->identifier, identifier, identifier_length)) {
        found = 1;
        child_levels->column_count++;
      }
    }
    if (!found) {
      child_levels = child_levels->next_child;
    }
  }

  if (!child_levels->set_flag && !found) {
    child_levels->parent_hash = parent_hash;
    child_levels->identifier_int = identifier_int;
    child_levels->identifier_length = identifier_length;
    child_levels->identifier = calloc(1, identifier_length);
    memcpy(child_levels->identifier, identifier, identifier_length);
    child_levels->column_count++;
    child_levels->set_flag = 1;
    child_levels->next_child = (JSONLevelBuilder*)calloc(1, sizeof(JSONLevelBuilder));
    child_levels->next_child_array = (JSONLevelBuilder*)calloc(1, sizeof(JSONLevelBuilder));

    child_levels->search_list = (HashList*)calloc(1, sizeof(HashList));
    initialize_search_list(child_levels, builder->row_count);

    found = 1;
  }
}

void initialize_child_levels(JSONLevelBuilder* single_object_children, unsigned long visible_depth)
{
  while(single_object_children->set_flag) {
    single_object_children->active_row_strings = (JSONLevelStrings*)calloc(1, sizeof(JSONLevelStrings));
    single_object_children->active_row_strings->set_strings_count = 0;

    single_object_children->active_row_strings->string_lengths = (unsigned long*)calloc(single_object_children->column_count, sizeof(unsigned long));
    single_object_children->active_row_strings->row_strings = (char**)calloc(single_object_children->column_count, sizeof(char*));

    single_object_children->mapping_array_lengths = (unsigned long*)calloc(1, sizeof(unsigned long) * single_object_children->column_count);
    single_object_children->mapping_array = (char**)calloc(1, sizeof(char*) * single_object_children->column_count);
    single_object_children->quote_array = (unsigned long*)calloc(1, sizeof(unsigned long) * single_object_children->column_count);
    single_object_children->do_not_hash = (unsigned long*)calloc(1, sizeof(unsigned long) * single_object_children->column_count);
    single_object_children->depth_array = (unsigned long*)calloc(1, sizeof(unsigned long) * single_object_children->column_count);
    single_object_children->real_depth_array = (unsigned long*)calloc(1, sizeof(unsigned long) * single_object_children->column_count);
    single_object_children->repeating_array_columns = (unsigned long*)calloc(1, sizeof(unsigned long) * single_object_children->column_count);

    single_object_children->column_name_lengths = (unsigned long*)calloc(1, sizeof(unsigned long) * single_object_children->column_count);
    single_object_children->column_names = (char**)calloc(1, sizeof(char*) * single_object_children->column_count);

    single_object_children->defined_flag = 1;
    single_object_children = single_object_children->next_child;
  }
}

void set_single_object_child_level_definitions(JSONLevelBuilder* level_definitions, JSONLevelBuilder* single_object_info_list, uint64_t hash, unsigned long column_count, unsigned long accessing_depth)
{
  int counter = 0;
  int identifier_int;
  int cursor;
  unsigned char found = 0;
  int test_identifier_length;
  char* test_identifier;
  JSONLevelBuilder* single_object_children;

  while(counter < column_count) {
    single_object_children = single_object_info_list;
    if ((read_type(accessing_depth, level_definitions->mapping_array[counter], level_definitions->mapping_array_lengths[counter], 1) == '2')) {
      found = 0;
      while(single_object_children->set_flag && !found) {
        read_identifier(accessing_depth, level_definitions->mapping_array[counter], &cursor, &test_identifier_length, level_definitions->mapping_array_lengths[counter], 1);
        test_identifier = calloc(1, test_identifier_length);
        memcpy(test_identifier, level_definitions->mapping_array[counter] + cursor, test_identifier_length);
        identifier_int = atoi(test_identifier);

        if ((single_object_children->identifier_int == identifier_int) && (single_object_children->parent_hash == hash) && (single_object_children->assigned_count < single_object_children->column_count)) {
          single_object_children->mapping_array_lengths[single_object_children->assigned_count] = level_definitions->mapping_array_lengths[counter];
          single_object_children->mapping_array[single_object_children->assigned_count] = calloc(1, level_definitions->mapping_array_lengths[counter]);
          memcpy(single_object_children->mapping_array[single_object_children->assigned_count], level_definitions->mapping_array[counter], level_definitions->mapping_array_lengths[counter]);
          single_object_children->quote_array[single_object_children->assigned_count] = level_definitions->quote_array[counter];
          single_object_children->do_not_hash[single_object_children->assigned_count] = level_definitions->do_not_hash[counter];
          single_object_children->depth_array[single_object_children->assigned_count] = level_definitions->depth_array[counter];
          single_object_children->real_depth_array[single_object_children->assigned_count] = level_definitions->real_depth_array[counter];
          single_object_children->repeating_array_columns[single_object_children->assigned_count] = level_definitions->repeating_array_columns[counter];
          single_object_children->column_name_lengths[single_object_children->assigned_count] = level_definitions->column_name_lengths[counter];  
          single_object_children->column_names[single_object_children->assigned_count] = level_definitions->column_names[counter];

          single_object_children->assigned_count += 1;
        }
        single_object_children = single_object_children->next_child;
      }
    }
    counter++;
  }
}

void assign_single_object_data(
  JSONLevelBuilder* level_definitions, 
  JSONLevelBuilder* single_object_info_list, 
  char** row_strings, 
  unsigned long* string_sizes, 
  unsigned long column_count, 
  unsigned long accessing_depth)
{
  int counter = 0;
  int cursor;
  int tmp_total_assigned = 0;
  unsigned char found = 0;
  int test_identifier_length;
  char* test_identifier;
  JSONLevelBuilder* single_object_children;

  while(counter < column_count) {  
    single_object_children = single_object_info_list;
    if (read_type(accessing_depth, level_definitions->mapping_array[counter], level_definitions->mapping_array_lengths[counter], 1) == '2') {
      found = 0;
      read_identifier(accessing_depth, level_definitions->mapping_array[counter], &cursor, &test_identifier_length, level_definitions->mapping_array_lengths[counter], 1);
      test_identifier = calloc(1, test_identifier_length);
      memcpy(test_identifier, level_definitions->mapping_array[counter] + cursor, test_identifier_length);
      
      while(single_object_children->set_flag && !found) {
        if (!memcmp(single_object_children->identifier, test_identifier, test_identifier_length) && (single_object_children->active_row_strings->set_strings_count < single_object_children->column_count)) {
          single_object_children->active_row_strings->string_lengths[single_object_children->active_row_strings->set_strings_count] = string_sizes[counter];
          single_object_children->active_row_strings->row_strings[single_object_children->active_row_strings->set_strings_count] = calloc(1, string_sizes[counter]);
          memcpy(single_object_children->active_row_strings->row_strings[single_object_children->active_row_strings->set_strings_count], row_strings[counter], string_sizes[counter]);
          
          single_object_children->active_row_strings->set_strings_count += 1;

          tmp_total_assigned++;
          found = 1;
        }
        single_object_children = single_object_children->next_child;
      }
    }
    counter++;
  }
}

unsigned long finalize_single_objects(JSONDocumentBuilder* builder, SingleObjectJSON* level_single_objects, unsigned long counter, int comma_finish)
{
  while(level_single_objects->set_flag) {
    memcpy(builder->resulting_json + counter, oq, 1);
    counter++;
    memcpy(builder->resulting_json + counter, level_single_objects->name, level_single_objects->name_characters);
    counter += level_single_objects->name_characters;
    memcpy(builder->resulting_json + counter, oqc, 2);
    counter += 2;

    counter = finalize_key_values(builder, level_single_objects->value->single_values, level_single_objects->value, counter);
    counter = finalize_single_objects(builder, level_single_objects->value->single_objects, counter, (level_single_objects->value->array_values->set_flag ));
    counter = finalize_value_array(builder, level_single_objects->value->array_values, counter);
    counter = finalize_object_array(builder, level_single_objects->value->array_objects, counter);

    memcpy(builder->resulting_json + counter, cbr, 1);
    counter++;

    if ((level_single_objects->next_item->set_flag) || comma_finish) {
      memcpy(builder->resulting_json + counter, cm, 1);
      counter++;
    }

    level_single_objects = level_single_objects->next_item;
  }

  return counter;
}