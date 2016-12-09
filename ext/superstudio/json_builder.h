#include <stdint.h>

typedef struct JSONObject
{
  struct SingleValueJSON* single_values;
  struct SingleObjectJSON* single_objects;
  struct ArrayValueJSON* array_values;
  struct ArrayObjectJSON* array_objects;

  struct SingleValueJSON* last_single_value;
  struct SingleObjectJSON* last_single_object;
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

typedef struct ArrayValueJSON
{
  char* name;
  unsigned long name_characters;
  unsigned char quoted;
  struct ArrayValueListItem* value_list;
  struct ArrayValueListItem* last_list_value;

  struct ArrayValueJSON* next_value;
} ArrayValueJSON;

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

typedef struct ArrayValueListItem
{
  char* array_value;
  unsigned long value_characters;
  struct ArrayValueListItem* next_value;
  struct ArrayValueListItem* last_value;
} ArrayValueListItem;


typedef struct JSONBuilder
{
  char** mapping_array; //Contents of the internal map from ruby
  unsigned long* quote_array; //Array of 1s and 0s indicating if quotes are to be applied
  unsigned long* do_not_hash; //Array of 1s and 0s indicating if an item should or shouldn't be hashed (pretty much 3s for now)
  unsigned long* depth_array; //Array showing the depth of each mapping
  unsigned long* real_depth_array; //Array showing where calculation splits happen
  unsigned long* repeating_array_columns; //Array showing whether or not type 3s can have repeating values or not
  char** row_being_worked_on; //This is what gets set and worked on
  struct HashList* search_list; //How to find nodes and insert stuff
  unsigned long column_count;
  unsigned long max_depth;
  unsigned long max_real_depth;
  struct RowBuilder* sub_rows;
  struct ArrayValueJSON* direct_array_pointers; //Instead of having to look this stuff up repeatedly

  unsigned long row_count;
  unsigned long json_char_count;
  char* resulting_json; //What's going to end up being returned to ruby
  struct ArrayObjectJSON* root; //Root node of the JSON object, important for building, not for searching

  unsigned long* column_name_lengths;
  char** column_names;
} JSONBuilder;

typedef struct RowBuilder
{
  int identifier; // The part from the mapping after the 4.
  char** mapping_array;
  unsigned long* quote_array;
  unsigned long* do_not_hash;
  unsigned long* depth_array;
  unsigned long* real_depth_array;
  unsigned long column_count;
  unsigned long max_depth;
  unsigned long max_real_depth;
  struct HashList* search_list;
  struct ArrayValueJSON* direct_array_pointers;
  struct RowBuilder* sub_rows;
} RowBuilder;

void json_builder_initialize(JSONBuilder *json_builder_initialize);
void set_row_count(JSONBuilder *builder, unsigned long row_count);
unsigned long get_row_count(JSONBuilder *builder);
void set_column_count(JSONBuilder *builder, unsigned long count);
unsigned long get_column_count(JSONBuilder *builder);
void set_quote_array(JSONBuilder *builder, unsigned long* quotes);
void set_hashing_array(JSONBuilder *builder, unsigned long* do_not_hashes);
void set_depth_array(JSONBuilder *builder, unsigned long* depths, unsigned long max, unsigned long* real_depths, unsigned long max_real);
void set_mapping_array(JSONBuilder *builder, char** internal_map);
void set_column_names_sizes(JSONBuilder *builder, char** column_names, unsigned long* column_string_sizes);
void set_repeating_array_columns(JSONBuilder *builder, unsigned long* repeating);

void consume_row(JSONBuilder *builder, char** row, unsigned long* sizes, unsigned long accessing_depth, unsigned long column_count, uint64_t parent_hash, unsigned long* visible_depth_array);
void create_new_root_item(JSONBuilder* builder, uint64_t hash, char** row, unsigned long* string_sizes, uint64_t parent_hash);
char* read_value(char depth_start, char is_first, char* mapped_value);
char* append(const char *input, const char c);

void add_to_array(
  JSONBuilder* builder,
  uint64_t hash,
  char* string_value,
  unsigned long string_size,
  char* column_name,
  unsigned long column_name_length,
  unsigned long repeatable,
  unsigned long quoted
  );

void sub_build(
  JSONBuilder* builder, 
  char** sub_object_row, 
  unsigned long * sub_object_string_sizes, 
  unsigned int next_depth, 
  unsigned long sub_object_column_count, 
  unsigned int* sub_object_groups, 
  JSONObject* parent_object,
  unsigned long* visible_depth_array
  );

char* finalize_json(JSONBuilder *builder);
