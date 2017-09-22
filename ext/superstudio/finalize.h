#ifndef SS_FINALIZE_

#include "json_builder.h"

unsigned long finalize_key_values(JSONDocumentBuilder* builder, SingleValueJSON* single_values, JSONObject* parent_json, unsigned long counter);
unsigned long finalize_single_objects(JSONDocumentBuilder* builder, SingleObjectJSON* level_single_objects, unsigned long counter, int comma_finish);
unsigned long finalize_value_array(JSONDocumentBuilder* builder, ArrayValueJSON* value_arrays, unsigned long counter);
unsigned long finalize_object_array(JSONDocumentBuilder* builder, ArrayObjectJSON* object_array, unsigned long counter);


#endif //SS_FINALIZE_