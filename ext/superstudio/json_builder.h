#ifndef JSON_BUILDER_
#define JSON_BUILDER_

#include <stdint.h>

static const char* const obr = "{";
static const char* const ob = "[";
static const char* const oq = "\"";
static const char* const oqc = "\":";
static const char* const cm = ",";
static const char* const cbr = "}";
static const char* const cb = "]";
static const char* const end = "\0";

typedef struct JSONObject
{
  struct SingleValueJSON* single_values;
  struct SingleObjectJSON* single_objects;
  struct ArrayValueJSON* array_values;
  struct ArrayObjectJSON* array_objects;

  struct SingleValueJSON* last_single_value;
  struct SingleObjectJSON* last_single_object;

  unsigned int value_array_count;
} JSONObject;

typedef struct SingleValueJSON
{
  char* name;
  char* value;
  unsigned long value_characters;
  unsigned long name_characters;
  unsigned char set_flag;

  unsigned char quoted;
  struct SingleValueJSON* next_value;
} SingleValueJSON;

typedef struct SingleObjectJSON
{
  char* name;
  unsigned long name_characters;
  unsigned char set_flag;

  JSONObject* value;
  struct SingleObjectJSON* next_item;
} SingleObjectJSON;

typedef struct ArrayObjectJSON
{
  char* name;
  unsigned long name_characters;
  uint64_t identifier;
  struct ArrayObjectListItem* value_list;
  struct ArrayObjectListItem* last_list_object;
  struct ArrayObjectJSON* next;
} ArrayObjectJSON;

typedef struct ArrayObjectListItem
{
  struct JSONObject* array_object;
  struct ArrayObjectListItem* next_item;
} ArrayObjectListItem;

typedef struct ArrayValueJSON
{
  char* name;
  unsigned long name_characters;
  unsigned char quoted;
  unsigned char set_flag;
  struct ArrayValueListItem* value_list;
  struct ArrayValueListItem* last_list_value;
  struct ArrayValueJSON* next_value;
} ArrayValueJSON;

typedef struct ArrayValueListItem
{
  char* array_value;
  unsigned long value_characters;
  unsigned char set_flag;
  struct ArrayValueListItem* next_value;
  struct ArrayValueListItem* last_value;
} ArrayValueListItem;

typedef struct JSONDocumentBuilder
{
  unsigned long max_depth;
  unsigned long max_real_depth;
  unsigned long row_count;
  unsigned long json_char_count;
  char* resulting_json; //What's going to end up being returned to ruby
  struct ArrayObjectJSON* root; //Root node of the JSON object, important for building, not for searching
  struct JSONLevelBuilder* root_level;
  unsigned long* single_object_key_lengths;
  unsigned long* array_object_key_lengths;
  char** single_object_key_names;
  char** array_object_key_names;
} JSONDocumentBuilder;

typedef struct JSONLevelStrings
{
  unsigned long set_strings_count;
  unsigned long* string_lengths;
  char** row_strings;
} JSONLevelStrings;

typedef struct JSONLevelBuilder
{
  struct HashList* search_list; //How to find nodes and insert stuff

  char* identifier; // The part from the mapping after the 4. or 2.
  unsigned char identifier_length;
  unsigned int identifier_int; // Internal identifier number to go ask the builder for the name of this fork from
  uint64_t parent_hash;

  unsigned char set_flag;
  unsigned char defined_flag;
  unsigned char contains_array_value_flag;
  unsigned char level_type; // 2 or 4
  unsigned long assigned_count; // When building a new level, count the number of assigned columns

  char** mapping_array; //Contents of the internal map from ruby
  unsigned long* mapping_array_lengths;
  unsigned long* quote_array; //Array of 1s and 0s indicating if quotes are to be applied
  unsigned long* do_not_hash; //Array of 1s and 0s indicating if an item should or shouldn't be hashed (pretty much 3s for now)
  unsigned long* depth_array; //Array showing the depth of each mapping
  unsigned long* real_depth_array; //Array showing where calculation splits happen
  unsigned long* repeating_array_columns; //Array showing whether or not type 3s can have repeating values or not
  unsigned long column_count;
  unsigned long* column_name_lengths;
  char** column_names;

  struct JSONLevelStrings* active_row_strings;
  struct JSONLevelBuilder* next_child;
  struct JSONLevelBuilder* last_child;
} JSONLevelBuilder;

void json_builder_initialize(JSONDocumentBuilder *json_builder_initialize);
void set_row_count(JSONDocumentBuilder *builder, unsigned long row_count);
void initialize_search_list(JSONLevelBuilder* level, unsigned long count);
unsigned long get_row_count(JSONDocumentBuilder *builder);
void set_column_count(JSONDocumentBuilder *builder, unsigned long count);
unsigned long get_column_count(JSONDocumentBuilder *builder);
void set_quote_array(JSONDocumentBuilder *builder, unsigned long* quotes);
void set_hashing_array(JSONDocumentBuilder *builder, unsigned long* do_not_hashes);
void set_depth_array(JSONDocumentBuilder *builder, unsigned long* depths, unsigned long max, unsigned long* real_depths, unsigned long max_real);
void set_mapping_array(JSONDocumentBuilder *builder, char** internal_map, unsigned long* internal_map_sizes);
void set_column_names_sizes(JSONDocumentBuilder *builder, char** column_names, unsigned long* column_string_sizes);
void set_repeating_array_columns(JSONDocumentBuilder *builder, unsigned long* repeating);
void consume_row(JSONDocumentBuilder *builder, 
  char** row, 
  unsigned long* sizes, 
  unsigned long accessing_depth, 
  unsigned long visible_depth, 
  unsigned long column_count, 
  uint64_t parent_hash, 
  JSONObject* parent_object, 
  unsigned long* visible_depth_array, 
  JSONLevelBuilder* level_definitions, 
  unsigned char parent_type);
void create_array_object(JSONDocumentBuilder* builder, uint64_t hash, uint64_t parent_hash, JSONLevelBuilder* parent_level);
uint64_t calculate_run_hash(JSONLevelBuilder* level_definitions, unsigned long column_count, unsigned long* string_sizes, char** row_strings);
char read_type(char depth_start, char* mapped_value);
void read_identifier(char depth_start, char* mapped_value, int* cursor, int* identifier_characters);
void set_single_node_key_names(JSONDocumentBuilder *builder, char** key_array, unsigned long* key_sizes);
void set_array_node_key_names(JSONDocumentBuilder *builder, char** key_array, unsigned long* key_sizes);
void define_child_levels(JSONDocumentBuilder *builder, JSONLevelBuilder* level_definitions, JSONLevelBuilder* single_object_children, uint64_t hash, unsigned long column_count, unsigned long accessing_depth);
void consume_single_objects(JSONDocumentBuilder* builder, JSONLevelBuilder* single_object_children, JSONObject* found_object, unsigned long accessing_depth, uint64_t hash, unsigned long visible_depth);

char* finalize_json(JSONDocumentBuilder *builder);

#endif