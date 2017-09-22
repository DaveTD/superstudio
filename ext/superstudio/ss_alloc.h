#ifndef SS_ALLOC_
#define SS_ALLOC_

#include <stddef.h>
#include <stdlib.h>
#include "json_builder.h"

void *ss_alloc(SSMemoryStack* memory_stack, size_t multiple, size_t size);
void collapse_stack(SSMemoryStack* memory_stack);

#endif // SS_ALLOC_