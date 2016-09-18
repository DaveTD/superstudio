#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "fnv_64.h"

/*
*  0 = char* string
*  1 = short
*  2 = int
*  3 = long
*  4 = float
*  6 = double
*/

uint64_t fnv_hash(uint64_t hash, void *data, char type)
{
  unsigned int i;

  union {
    short source_short;
    int source_int;
    long source_long;
    float source_float;
    double source_double;
    char* source_string;
    char short_bytes[sizeof(short)];
    char int_bytes[sizeof(int)];
    char long_bytes[sizeof(long)];
    char float_bytes[sizeof(float)];
    char double_bytes[sizeof(double)];
  } byte_struct;

  switch(type) {
    case 1:
      byte_struct.source_short = *(short*)data;
      for (i=0; i<sizeof(short); i++)
      {
        char* byte = &byte_struct.short_bytes[sizeof(short) - 1 - i];
        hash = fnv_hash_byte(hash, byte);
      }
      return hash;
    break;
    case 2:
      byte_struct.source_int = *(int*)data;
      for (i=0; i<sizeof(int); i++)
      {
        char* byte = &byte_struct.int_bytes[sizeof(int) - 1 - i];
        hash = fnv_hash_byte(hash, byte);
      }
      return hash;
    break;
    case 3:
      byte_struct.source_long = *(long*)data;
      for (i=0; i<sizeof(long); i++)
      {
        char* byte = &byte_struct.long_bytes[sizeof(long) - 1 - i];
        hash = fnv_hash_byte(hash, byte);
      }
      return hash;
    break;
    case 4:
      byte_struct.source_float = *(float*)data;
      for (i=0; i<sizeof(float); i++)
      {
        char* byte = &byte_struct.float_bytes[sizeof(float) - 1 - i];
        hash = fnv_hash_byte(hash, byte);
      }
      return hash;
    break;
    case 5:
      byte_struct.source_double = *(double*)data;
      for (i=0; i<sizeof(double); i++)
      {
        char* byte = &byte_struct.double_bytes[sizeof(double) - 1 - i];
        hash = fnv_hash_byte(hash, byte);
      }
      return hash;
    break;
    case 0:
      byte_struct.source_string = (char*)data;
      while(*byte_struct.source_string != '\0')
      {
         hash = fnv_hash_byte(hash, byte_struct.source_string);
         *byte_struct.source_string = *byte_struct.source_string + 1;
      }
      return hash;
    break;
  }
  return 0;
}

uint64_t fnv_hash_byte(uint64_t hash, char *data)
{
  hash ^= (uint64_t)(*data);
  hash *= (FNV_PRIME % UINT64_MAX);
  return hash;
}
