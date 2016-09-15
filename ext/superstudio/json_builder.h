#include <stdint.h>

typedef struct JSONObject
{
  struct SingleValueJSON* single_values;
  struct SingleObjectJSON* single_objects;
  struct ArrayValueJSON** array_values;
  struct ArrayObjectJSON** array_objects;
} JSONObject;

typedef struct SingleValueJSON
{
  char* name;
  char* value;
} SingleValueJSON;

typedef struct SingleObjectJSON
{
  char* name;
  JSONObject* value;
} SingleObjectJSON;

typedef struct ArrayValueJSON
{
  char* name;
  char** value_array;
} ArrayValueJSON;

typedef struct ArrayObjectJSON
{
  char* name;
  uint64_t identifier;
  JSONObject** value_array;
} ArrayObjectJSON;

typedef struct JSONBuilder
{
  char** mapping_array; //Contents of the internal map from ruby
  unsigned long* quote_array; //Array of 1s and 0s indicating if quotes are to be applied
  unsigned long* do_not_hash; //Array of 1s and 0s indicating if an item should or shouldn't be hashed (pretty much 3s for now)
  unsigned long* depth_array; //Array showing the depth of each mapping
  unsigned long* real_depth_array; //Array showing where calculation splits happen
  char** row_being_worked_on; //This is what gets set and worked on
  char* resulting_json; //What's going to end up being returned to ruby
  struct ArrayObjectJSON* root; //Root node of the JSON object, important for building, not for searching
  struct HashList* search_list; //How to find nodes and insert stuff
  unsigned long row_count;
  unsigned long column_count;
  unsigned long json_char_count;
  unsigned long max_depth;
  unsigned long max_real_depth;
} JSONBuilder;

void json_builder_initialize(JSONBuilder *json_builder_initialize);
void set_row_count(JSONBuilder *builder, unsigned long row_count);
unsigned long get_row_count(JSONBuilder *builder);
void set_column_count(JSONBuilder *builder, unsigned long count);
unsigned long get_column_count(JSONBuilder *builder);
void set_quote_array(JSONBuilder *builder, unsigned long* quotes);
void set_hashing_array(JSONBuilder *builder, unsigned long* do_not_hashes);
void set_depth_array(JSONBuilder *builder, unsigned long* depths, unsigned long max, unsigned long* real_depths, unsigned long max_real);
void set_mapping_array(JSONBuilder *builder, char** internal_map);

void consume_row(JSONBuilder *builder, char** row, unsigned long* sizes, unsigned long accessing_depth, unsigned long column_count, uint64_t parent_hash);
void create_new_root_item(JSONBuilder* builder, uint64_t hash, char** row, unsigned long* string_sizes);
char* read_value(char depth_start, char is_first, char* mapped_value);
char* append(const char *input, const char c);

char* finalize_json(JSONBuilder *builder);
