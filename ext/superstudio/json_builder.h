#ifndef JSON_BUILDER_
#define JSON_BUILDER_

#include <stdint.h>

static const char* obr = "{";
static const char* ob = "[";
static const char* oq = "\"";
static const char* oqc = "\":";
static const char* cm = ",";
static const char* cbr = "}";
static const char* cb = "]";
static const char* end = "\0";

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
  unsigned char quoted;
  struct SingleValueJSON* next_value;
} SingleValueJSON;

typedef struct SingleObjectJSON
{
  char* name;
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
  char** mapping_array; //Contents of the internal map from ruby
  unsigned long* quote_array; //Array of 1s and 0s indicating if quotes are to be applied
  unsigned long* do_not_hash; //Array of 1s and 0s indicating if an item should or shouldn't be hashed (pretty much 3s for now)
  unsigned long* depth_array; //Array showing the depth of each mapping
  unsigned long* real_depth_array; //Array showing where calculation splits happen
  unsigned long* repeating_array_columns; //Array showing whether or not type 3s can have repeating values or not
  struct HashList* search_list; //How to find nodes and insert stuff
  unsigned long column_count;
  unsigned long max_depth;
  unsigned long max_real_depth;
  struct JSONLevelBuilder* child_levels;
  struct ArrayValueJSON* direct_array_pointers; //Instead of having to look this stuff up repeatedly

  unsigned long row_count;
  unsigned long json_char_count;
  char* resulting_json; //What's going to end up being returned to ruby
  struct ArrayObjectJSON* root; //Root node of the JSON object, important for building, not for searching

  unsigned long* column_name_lengths;
  char** column_names;
} JSONDocumentBuilder;

typedef struct JSONLevelBuilder
{
  int identifier; // The part from the mapping after the 4.
  char** mapping_array;
  unsigned long* quote_array;
  unsigned long* do_not_hash;
  unsigned long* depth_array;
  unsigned long* real_depth_array;
  unsigned long* repeading_array_columns;
  unsigned long column_count;
  unsigned long max_depth;
  unsigned long max_real_depth;
  struct HashList* search_list;
  struct ArrayValueJSON* direct_array_pointers;
  struct JSONLevelBuilder* child_levels;
} JSONLevelBuilder;

void json_builder_initialize(JSONDocumentBuilder *json_builder_initialize);
void set_row_count(JSONDocumentBuilder *builder, unsigned long row_count);
unsigned long get_row_count(JSONDocumentBuilder *builder);
void set_column_count(JSONDocumentBuilder *builder, unsigned long count);
unsigned long get_column_count(JSONDocumentBuilder *builder);
void set_quote_array(JSONDocumentBuilder *builder, unsigned long* quotes);
void set_hashing_array(JSONDocumentBuilder *builder, unsigned long* do_not_hashes);
void set_depth_array(JSONDocumentBuilder *builder, unsigned long* depths, unsigned long max, unsigned long* real_depths, unsigned long max_real);
void set_mapping_array(JSONDocumentBuilder *builder, char** internal_map);
void set_column_names_sizes(JSONDocumentBuilder *builder, char** column_names, unsigned long* column_string_sizes);
void set_repeating_array_columns(JSONDocumentBuilder *builder, unsigned long* repeating);

void consume_row(JSONDocumentBuilder *builder, char** row, unsigned long* sizes, unsigned long accessing_depth, unsigned long column_count, uint64_t parent_hash, unsigned long* visible_depth_array);
void create_array_object(JSONDocumentBuilder* builder, uint64_t hash, uint64_t parent_hash);
char* read_identifier(char depth_start, char* mapped_value);

char* finalize_json(JSONDocumentBuilder *builder);

#endif