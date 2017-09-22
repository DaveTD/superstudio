#include "json_object_array.h"
#include "hash_linked_list.h"
#include "ss_alloc.h"

void create_array_object(JSONDocumentBuilder* builder, 
  uint64_t hash, 
  HashList* parent_list,
  JSONLevelBuilder* parent_level_definitions, 
  JSONLevelBuilder* empty_child_object, 
  JSONLevelBuilder* empty_child_array,
  int identifier_int
  )
{
  HashListNode* found_node = NULL;
  ArrayObjectJSON* parent_array_object = NULL;
  found_node = hl_find_node(parent_list, hash);
  
  if (found_node) {

  } else {
    parent_array_object = builder->root;

    if (!parent_array_object->value_list) {
      if (identifier_int > 0) {
        // Dev note:
        // These are all -1 because the array that gets passed starts at 0, but numbering starts at 1
        parent_array_object->name_characters = builder->array_object_key_lengths[identifier_int - 1];        
        parent_array_object->name = builder->array_object_key_names[identifier_int - 1];

        builder->json_char_count += 8; //quotes, colon, brackets, braces, comma maybe
        builder->json_char_count += builder->array_object_key_lengths[identifier_int - 1];
      } else {
        parent_array_object->name_characters = 0;
      }

      parent_array_object->value_list = (ArrayObjectListItem*)ss_alloc(builder->memory_stack, 1, sizeof(ArrayObjectListItem));
      parent_array_object->value_list->array_object = (JSONObject*)ss_alloc(builder->memory_stack, 1, sizeof(JSONObject));
      parent_array_object->value_list->array_object->array_values = (ArrayValueJSON*)ss_alloc(builder->memory_stack, 1, sizeof(ArrayValueJSON));
      parent_array_object->value_list->array_object->single_objects = (SingleObjectJSON*)ss_alloc(builder->memory_stack, 1, sizeof(SingleObjectJSON));
      parent_array_object->value_list->array_object->last_single_object = parent_array_object->value_list->array_object->single_objects;
      parent_array_object->value_list->array_object->single_values = (SingleValueJSON*)ss_alloc(builder->memory_stack, 1, sizeof(SingleValueJSON));
      parent_array_object->value_list->array_object->last_single_value = parent_array_object->value_list->array_object->single_values;
      parent_array_object->value_list->array_object->array_objects = (ArrayObjectJSON*)ss_alloc(builder->memory_stack, 1, sizeof(ArrayObjectJSON));
      parent_array_object->value_list->array_object->array_objects->next = (ArrayObjectJSON*)ss_alloc(builder->memory_stack, 1, sizeof(ArrayObjectJSON));
      parent_array_object->value_list->associated_hash = hash;

      parent_array_object->last_list_object = parent_array_object->value_list;
      parent_array_object->set_flag = 1;
      parent_array_object->last_list_object->set_flag = 1;

      hl_insert_or_find(parent_list, hash, parent_array_object->last_list_object->array_object, parent_level_definitions, empty_child_object, empty_child_array, builder->memory_stack);
    } else if (identifier_int == 0) {
      parent_array_object->last_list_object->next_item = (ArrayObjectListItem*)ss_alloc(builder->memory_stack, 1, sizeof(ArrayObjectListItem));
      parent_array_object->last_list_object->next_item->array_object = (JSONObject*)ss_alloc(builder->memory_stack, 1, sizeof(JSONObject));
      parent_array_object->last_list_object->next_item->array_object->array_values = (ArrayValueJSON*)ss_alloc(builder->memory_stack, 1, sizeof(ArrayValueJSON));
      parent_array_object->last_list_object->next_item->array_object->single_objects = (SingleObjectJSON*)ss_alloc(builder->memory_stack, 1, sizeof(SingleObjectJSON));
      parent_array_object->last_list_object->next_item->array_object->last_single_object = parent_array_object->last_list_object->next_item->array_object->single_objects;
      parent_array_object->last_list_object->next_item->array_object->single_values = (SingleValueJSON*)ss_alloc(builder->memory_stack, 1, sizeof(SingleValueJSON));
      parent_array_object->last_list_object->next_item->array_object->last_single_value = parent_array_object->last_list_object->next_item->array_object->single_values;
      parent_array_object->last_list_object->next_item->array_object->array_objects = (ArrayObjectJSON*)ss_alloc(builder->memory_stack, 1, sizeof(ArrayObjectJSON));
      parent_array_object->last_list_object->next_item->array_object->array_objects->next = (ArrayObjectJSON*)ss_alloc(builder->memory_stack, 1, sizeof(ArrayObjectJSON));
      parent_array_object->last_list_object->next_item->associated_hash = hash;      

      parent_array_object->set_flag = 1;

      parent_array_object->last_list_object = parent_array_object->last_list_object->next_item;      
      hl_insert_or_find(parent_list, hash, parent_array_object->last_list_object->array_object, parent_level_definitions, empty_child_object, empty_child_array, builder->memory_stack);
    }
  }
}

void count_increment_or_create_json_level_child_array(
  JSONDocumentBuilder* builder,
  JSONLevelBuilder* child_array_levels,
  int identifier_length,
  char* identifier,
  int identifier_int,
  uint64_t parent_hash,
  uint64_t child_hash)
{
  unsigned char found = 0;

  while(child_array_levels->set_flag && !found) {
    if (child_array_levels->identifier_length == identifier_length) {
      if (!memcmp(child_array_levels->identifier, identifier, identifier_length)) {
        if (child_hash == child_array_levels->hash) {
          found = 1;
          child_array_levels->column_count++;
        }
      }
    }
    if (!found) {
      child_array_levels = child_array_levels->next_child_array;
    }
  }

  if (!child_array_levels->set_flag && !found) {
    child_array_levels->parent_hash = parent_hash;
    child_array_levels->hash = child_hash;
    child_array_levels->identifier_int = identifier_int;
    child_array_levels->identifier_length = identifier_length;
    child_array_levels->identifier = identifier;

    child_array_levels->column_count++;
    child_array_levels->set_flag = 1;
    child_array_levels->next_child = (JSONLevelBuilder*)ss_alloc(builder->memory_stack, 1, sizeof(JSONLevelBuilder));
    child_array_levels->next_child_array = (JSONLevelBuilder*)ss_alloc(builder->memory_stack, 1, sizeof(JSONLevelBuilder));

    child_array_levels->search_list = (HashList*)ss_alloc(builder->memory_stack, 1, sizeof(HashList));
    initialize_search_list(child_array_levels, builder->memory_stack, builder->row_count);
    found = 1;
  }
}

void initialize_child_array_levels(JSONLevelBuilder* child_array_levels, SSMemoryStack* memory_stack)
{
  while(child_array_levels->set_flag && child_array_levels->identifier_int) {
    if (!child_array_levels->defined_flag) {
      child_array_levels->active_row_strings = (JSONLevelStrings*)ss_alloc(memory_stack, 1, sizeof(JSONLevelStrings));
      child_array_levels->active_row_strings->set_strings_count = 0;
      child_array_levels->active_row_strings->string_lengths = (unsigned long*)ss_alloc(memory_stack, child_array_levels->column_count, sizeof(unsigned long));
      child_array_levels->active_row_strings->row_strings = (char**)ss_alloc(memory_stack, child_array_levels->column_count, sizeof(char*));
      child_array_levels->mapping_array_lengths = (unsigned long*)ss_alloc(memory_stack, 1, sizeof(unsigned long) * child_array_levels->column_count);
      child_array_levels->mapping_array = (char**)ss_alloc(memory_stack, 1, sizeof(char*) * child_array_levels->column_count);
      child_array_levels->quote_array = (unsigned long*)ss_alloc(memory_stack, 1, sizeof(unsigned long) * child_array_levels->column_count);
      child_array_levels->do_not_hash = (unsigned long*)ss_alloc(memory_stack, 1, sizeof(unsigned long) * child_array_levels->column_count);
      child_array_levels->depth_array = (unsigned long*)ss_alloc(memory_stack, 1, sizeof(unsigned long) * child_array_levels->column_count);
      child_array_levels->real_depth_array = (unsigned long*)ss_alloc(memory_stack, 1, sizeof(unsigned long) * child_array_levels->column_count);
      child_array_levels->repeating_array_columns = (unsigned long*)ss_alloc(memory_stack, 1, sizeof(unsigned long) * child_array_levels->column_count);
      child_array_levels->column_name_lengths = (unsigned long*)ss_alloc(memory_stack, 1, sizeof(unsigned long) * child_array_levels->column_count);
      child_array_levels->column_names = (char**)ss_alloc(memory_stack, 1, sizeof(char*) * child_array_levels->column_count);
      child_array_levels->defined_flag = 1;
    }
    child_array_levels = child_array_levels->next_child_array;
  }
}

void set_array_object_child_level_definitions(
  JSONLevelBuilder* level_definitions,
  JSONLevelBuilder* child_array_levels,
  uint64_t hash,
  unsigned long column_count,
  unsigned long accessing_depth
  )
{
  unsigned long counter = 0;
  int identifier_int;
  int cursor;
  unsigned char found = 0;
  int test_identifier_length;
  char* test_identifier;
  JSONLevelBuilder* child_level_first_node = child_array_levels;
  
  while(counter < column_count) {
    if ((read_type(accessing_depth, level_definitions->mapping_array[counter], level_definitions->mapping_array_lengths[counter]) == '4')) {
      found = 0;
      while(child_array_levels->set_flag && !found) {
        read_identifier(accessing_depth, level_definitions->mapping_array[counter], &cursor, &test_identifier_length, level_definitions->mapping_array_lengths[counter]);
        test_identifier = level_definitions->mapping_array[counter] + cursor;

        identifier_int = atoi(test_identifier);

        if (
          (child_array_levels->identifier_int == identifier_int) 
          && (child_array_levels->parent_hash == hash) 
          && (child_array_levels->assigned_count < child_array_levels->column_count)
          ) {
  
          child_array_levels->mapping_array_lengths[child_array_levels->assigned_count] = level_definitions->mapping_array_lengths[counter];
          child_array_levels->mapping_array[child_array_levels->assigned_count] = level_definitions->mapping_array[counter];

          child_array_levels->quote_array[child_array_levels->assigned_count] = level_definitions->quote_array[counter];
          child_array_levels->do_not_hash[child_array_levels->assigned_count] = level_definitions->do_not_hash[counter];
          child_array_levels->depth_array[child_array_levels->assigned_count] = level_definitions->depth_array[counter];
          child_array_levels->real_depth_array[child_array_levels->assigned_count] = level_definitions->real_depth_array[counter];
          child_array_levels->repeating_array_columns[child_array_levels->assigned_count] = level_definitions->repeating_array_columns[counter];
          child_array_levels->column_name_lengths[child_array_levels->assigned_count] = level_definitions->column_name_lengths[counter];
          child_array_levels->column_names[child_array_levels->assigned_count] = level_definitions->column_names[counter];

          child_array_levels->assigned_count += 1;
        }
        child_array_levels = child_array_levels->next_child_array;
      }
    }
    child_array_levels = child_level_first_node;
    counter++;
  }
}

void assign_array_object_data(
  JSONLevelBuilder* level_definitions,
  JSONLevelBuilder* child_array_levels,
  char** row_strings,
  unsigned long* string_sizes,
  unsigned long column_count,
  unsigned long accessing_depth
  )
{
  unsigned long counter = 0;
  int cursor;
  int tmp_total_assigned = 0;
  unsigned char found = 0;
  unsigned long inner_counter = 0;
  int test_identifier_length;
  char* test_identifier;
  JSONLevelBuilder* child_level_first_node = child_array_levels;

  while(counter < column_count) {
    if (read_type(accessing_depth, level_definitions->mapping_array[counter], level_definitions->mapping_array_lengths[counter]) == '4') {
      found = 0;
      read_identifier(accessing_depth, level_definitions->mapping_array[counter], &cursor, &test_identifier_length, level_definitions->mapping_array_lengths[counter]);      
      test_identifier = level_definitions->mapping_array[counter] + cursor;

      while(child_array_levels->set_flag && !found) {
        if (!memcmp(child_array_levels->identifier, test_identifier, test_identifier_length) && (child_array_levels->active_row_strings->set_strings_count < child_array_levels->column_count)) {
          child_array_levels->active_row_strings->string_lengths[child_array_levels->active_row_strings->set_strings_count] = string_sizes[counter];          
          child_array_levels->active_row_strings->row_strings[child_array_levels->active_row_strings->set_strings_count] = row_strings[counter];

          inner_counter = 0;
          while(inner_counter < child_array_levels->active_row_strings->string_lengths[child_array_levels->active_row_strings->set_strings_count]) {
            inner_counter++;
          }

          child_array_levels->active_row_strings->set_strings_count += 1;

          tmp_total_assigned++;
          found = 1;
        }
        child_array_levels = child_array_levels->next_child_array;
      }
    }
    child_array_levels = child_level_first_node;
    counter++;
  }
}

void add_or_find_array_object_child_hashes(
  JSONDocumentBuilder* builder,
  JSONLevelBuilder* level_definitions,
  JSONLevelBuilder* child_array_levels, 
  unsigned long accessing_depth,
  unsigned long visible_depth,
  unsigned long column_count,
  unsigned long* string_sizes,
  char** row_strings,
  JSONObject* related_object)
{
  uint64_t child_hash_to_add;

  ArrayObjectJSON* related_array_objects;

  JSONLevelBuilder* empty_child_object;
  JSONLevelBuilder* empty_child_array;
  ArrayObjectListItem* hash_tester;

  int found;

  while (child_array_levels->set_flag && child_array_levels->identifier_int) {
    found = 0;

    child_hash_to_add = calculate_identified_hash(level_definitions, column_count, string_sizes, row_strings, accessing_depth + 1, (accessing_depth + visible_depth + 1), child_array_levels->identifier_length, child_array_levels->identifier);

    related_array_objects = related_object->array_objects;

    while(related_array_objects->set_flag && !found) {
      empty_child_object = (JSONLevelBuilder*)ss_alloc(builder->memory_stack, 1, sizeof(JSONLevelBuilder));
      empty_child_array = (JSONLevelBuilder*)ss_alloc(builder->memory_stack, 1, sizeof(JSONLevelBuilder));
      empty_child_object->next_child = (JSONLevelBuilder*)ss_alloc(builder->memory_stack, 1, sizeof(JSONLevelBuilder));
      empty_child_object->next_child_array = (JSONLevelBuilder*)ss_alloc(builder->memory_stack, 1, sizeof(JSONLevelBuilder));
      empty_child_array->next_child = (JSONLevelBuilder*)ss_alloc(builder->memory_stack, 1, sizeof(JSONLevelBuilder));
      empty_child_array->next_child_array = (JSONLevelBuilder*)ss_alloc(builder->memory_stack, 1, sizeof(JSONLevelBuilder));

      if (related_array_objects->identifier_int == child_array_levels->identifier_int) {
        if (related_array_objects->value_list) {
          hash_tester = related_array_objects->value_list;
          while (hash_tester) {
            if (hash_tester->associated_hash == child_hash_to_add) {
              found = 1;
              hl_insert_or_find(child_array_levels->search_list, child_hash_to_add, related_array_objects->last_list_object->array_object, child_array_levels, empty_child_object, empty_child_array, builder->memory_stack);
              break;
            }
            hash_tester = hash_tester->next_item;
          }
        }

        if (!related_array_objects->value_list && !found) {
          found = 1;

          related_array_objects->value_list = (ArrayObjectListItem*)ss_alloc(builder->memory_stack, 1, sizeof(ArrayObjectListItem));
          related_array_objects->value_list->array_object = (JSONObject*)ss_alloc(builder->memory_stack, 1, sizeof(JSONObject));
          related_array_objects->value_list->array_object->array_values = (ArrayValueJSON*)ss_alloc(builder->memory_stack, 1, sizeof(ArrayValueJSON));
          related_array_objects->value_list->array_object->single_objects = (SingleObjectJSON*)ss_alloc(builder->memory_stack, 1, sizeof(SingleObjectJSON));
          related_array_objects->value_list->array_object->last_single_object = related_array_objects->value_list->array_object->single_objects;
          related_array_objects->value_list->array_object->single_values = (SingleValueJSON*)ss_alloc(builder->memory_stack, 1, sizeof(SingleValueJSON));
          related_array_objects->value_list->array_object->last_single_value = related_array_objects->value_list->array_object->single_values;
          related_array_objects->value_list->array_object->array_objects = (ArrayObjectJSON*)ss_alloc(builder->memory_stack, 1, sizeof(ArrayObjectJSON));
          related_array_objects->value_list->array_object->array_objects->set_flag = 1;
          related_array_objects->value_list->array_object->array_objects->next = (ArrayObjectJSON*)ss_alloc(builder->memory_stack, 1, sizeof(ArrayObjectJSON));
          related_array_objects->value_list->associated_hash = child_hash_to_add;

          related_array_objects->last_list_object = related_array_objects->value_list;
          related_array_objects->set_flag = 1;
          related_array_objects->last_list_object->set_flag = 1;
          hl_insert_or_find(child_array_levels->search_list, child_hash_to_add, related_array_objects->last_list_object->array_object, child_array_levels, empty_child_object, empty_child_array, builder->memory_stack);
        } else if (!found) {
          found = 1;
          related_array_objects->last_list_object->next_item = (ArrayObjectListItem*)ss_alloc(builder->memory_stack, 1, sizeof(ArrayObjectListItem));
          related_array_objects->last_list_object->next_item->array_object = (JSONObject*)ss_alloc(builder->memory_stack, 1, sizeof(JSONObject));
          related_array_objects->last_list_object->next_item->array_object->array_values = (ArrayValueJSON*)ss_alloc(builder->memory_stack, 1, sizeof(ArrayValueJSON));
          related_array_objects->last_list_object->next_item->array_object->single_objects = (SingleObjectJSON*)ss_alloc(builder->memory_stack, 1, sizeof(SingleObjectJSON));
          related_array_objects->last_list_object->next_item->array_object->last_single_object = related_array_objects->last_list_object->next_item->array_object->single_objects;
          related_array_objects->last_list_object->next_item->array_object->single_values = (SingleValueJSON*)ss_alloc(builder->memory_stack, 1, sizeof(SingleValueJSON));
          related_array_objects->last_list_object->next_item->array_object->last_single_value = related_array_objects->last_list_object->next_item->array_object->single_values;
          related_array_objects->last_list_object->next_item->array_object->array_objects = (ArrayObjectJSON*)ss_alloc(builder->memory_stack, 1, sizeof(ArrayObjectJSON));
          related_array_objects->last_list_object->next_item->array_object->array_objects->set_flag = 1;
          related_array_objects->last_list_object->next_item->array_object->array_objects->next = (ArrayObjectJSON*)ss_alloc(builder->memory_stack, 1, sizeof(ArrayObjectJSON));
          related_array_objects->last_list_object->next_item->associated_hash = child_hash_to_add;

          related_array_objects->set_flag = 1;
          related_array_objects->last_list_object->set_flag = 1;
          related_array_objects->last_list_object = related_array_objects->last_list_object->next_item;

          hl_insert_or_find(child_array_levels->search_list, child_hash_to_add, related_array_objects->last_list_object->array_object, child_array_levels, empty_child_object, empty_child_array, builder->memory_stack);
        }
      }

      if (!related_array_objects->next) {
        break;
      }
      related_array_objects = related_array_objects->next;
    }
    if (!found) {
      related_array_objects->name_characters = builder->array_object_key_lengths[child_array_levels->identifier_int - 1];
      related_array_objects->name = builder->array_object_key_names[child_array_levels->identifier_int - 1];

      builder->json_char_count += 8;
      builder->json_char_count += builder->array_object_key_lengths[child_array_levels->identifier_int - 1];

      related_array_objects->value_list = (ArrayObjectListItem*)ss_alloc(builder->memory_stack, 1, sizeof(ArrayObjectListItem));
      related_array_objects->value_list->array_object = (JSONObject*)ss_alloc(builder->memory_stack, 1, sizeof(JSONObject));
      related_array_objects->value_list->array_object->array_values = (ArrayValueJSON*)ss_alloc(builder->memory_stack, 1, sizeof(ArrayValueJSON));
      related_array_objects->value_list->array_object->single_objects = (SingleObjectJSON*)ss_alloc(builder->memory_stack, 1, sizeof(SingleObjectJSON));
      related_array_objects->value_list->array_object->last_single_object = related_array_objects->value_list->array_object->single_objects;
      related_array_objects->value_list->array_object->single_values = (SingleValueJSON*)ss_alloc(builder->memory_stack, 1, sizeof(SingleValueJSON));
      related_array_objects->value_list->array_object->last_single_value = related_array_objects->value_list->array_object->single_values;
      related_array_objects->value_list->array_object->array_objects = (ArrayObjectJSON*)ss_alloc(builder->memory_stack, 1, sizeof(ArrayObjectJSON));
      related_array_objects->value_list->array_object->array_objects->set_flag = 1;
      related_array_objects->value_list->array_object->array_objects->next = (ArrayObjectJSON*)ss_alloc(builder->memory_stack, 1, sizeof(ArrayObjectJSON));

      related_array_objects->value_list->associated_hash = child_hash_to_add;
      related_array_objects->identifier_int = child_array_levels->identifier_int;
      related_array_objects->last_list_object = related_array_objects->value_list;
      related_array_objects->set_flag = 1;
      related_array_objects->last_list_object->set_flag = 1;

      hl_insert_or_find(child_array_levels->search_list, child_hash_to_add, related_array_objects->last_list_object->array_object, child_array_levels, empty_child_object, empty_child_array, builder->memory_stack);
    }

    child_array_levels = child_array_levels->next_child_array;
    related_array_objects = related_object->array_objects;
  }
}



unsigned long finalize_object_array(JSONDocumentBuilder* builder, ArrayObjectJSON* object_array, unsigned long counter)
{
  ArrayObjectListItem* target_object = NULL;
  
  while(object_array) {
    if (object_array->set_flag) {
      target_object = NULL;  
      if (object_array->name_characters > 0) {
        memcpy(builder->resulting_json + counter, cm, 1);
        counter++;
        memcpy(builder->resulting_json + counter, oq, 1);
        counter++;
        memcpy(builder->resulting_json + counter, object_array->name, object_array->name_characters);
        counter += object_array->name_characters;
        memcpy(builder->resulting_json + counter, oqc, 2);
        counter += 2;
        memcpy(builder->resulting_json + counter, ob, 1);
        counter++;
      }

      target_object = object_array->value_list;
      while(target_object) {
        counter = finalize_key_values(builder, target_object->array_object->single_values, target_object->array_object, counter);
        counter = finalize_single_objects(builder, target_object->array_object->single_objects, counter, (target_object->array_object->array_values->set_flag ));
        counter = finalize_value_array(builder, target_object->array_object->array_values, counter);
        counter = finalize_object_array(builder, target_object->array_object->array_objects, counter);

        memcpy(builder->resulting_json + counter, cbr, 1);
        counter++;

        if (target_object->next_item) {
          memcpy(builder->resulting_json + counter, cm, 1);
          counter++;
        } else {
          memcpy(builder->resulting_json + counter, cb, 1);
          counter++;
        }
        target_object = target_object->next_item; 
      }
    }
    object_array = object_array->next;
  }
  return counter;
}