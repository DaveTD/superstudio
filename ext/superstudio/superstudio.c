#include <superstudio.h>

VALUE mSuperstudio;

void Init_superstudio()
{
  mSuperstudio = rb_define_module("Superstudio");

  Init_jsonbroker();
}

