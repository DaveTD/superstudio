#ifndef RUBY_JSONBROKER
#define RUBY_JSONBROKER

#include <superstudio.h>
#include <stdio.h>
#include <json_builder.h>

void Init_jsonbroker();
static VALUE json_broker_allocate();
static VALUE json_broker_set_mapper();
static VALUE json_broker_set_quotes();
static VALUE json_broker_set_depths();
static VALUE json_broker_set_row_count();
static VALUE json_broker_get_row_count();
static VALUE json_broker_get_mapper_length();
static VALUE json_broker_consume_row();
static VALUE json_broker_finalize_json();
static void deallocate_broker();

#endif

