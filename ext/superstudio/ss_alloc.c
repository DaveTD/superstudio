#include "ss_alloc.h"

void *ss_alloc(SSMemoryStack* memory_stack, size_t multiple, size_t size) {
  SSMemoryStackNode* new_node = calloc(1, sizeof(SSMemoryStackNode));
  new_node->memory_location = calloc(multiple, size);
  new_node->previous_node = memory_stack->stack_top;

  memory_stack->stack_top = new_node;

  return memory_stack->stack_top->memory_location; 
}

void collapse_stack(SSMemoryStack* memory_stack)
{
  SSMemoryStackNode* temp_previous;
  while (memory_stack->stack_top->memory_location)
  {
    temp_previous = memory_stack->stack_top->previous_node;

    free(memory_stack->stack_top->memory_location);
    memory_stack->stack_top->memory_location = NULL;
    free(memory_stack->stack_top);
    memory_stack->stack_top = NULL;

    memory_stack->stack_top = temp_previous;
  }
}
