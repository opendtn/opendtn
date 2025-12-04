/***
        ------------------------------------------------------------------------

        Copyright (c) 2021 German Aerospace Center DLR e.V. (GSOC)

        Licensed under the Apache License, Version 2.0 (the "License");
        you may not use this file except in compliance with the License.
        You may obtain a copy of the License at

                http://www.apache.org/licenses/LICENSE-2.0

        Unless required by applicable law or agreed to in writing, software
        distributed under the License is distributed on an "AS IS" BASIS,
        WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
        See the License for the specific language governing permissions and
        limitations under the License.

        This file is part of the openvocs project. https://openvocs.org

        ------------------------------------------------------------------------
*//**
        @file           dtn_json_io_buffer_test.c
        @author         Markus Töpfer

        @date           2021-03-29


        ------------------------------------------------------------------------
*/
#include "dtn_item_json_io_buffer.c"
#include "../include/testrun.h"

#include "../include/dtn_random.h"

/*
 *      ------------------------------------------------------------------------
 *
 *      TEST CASES                                                      #CASES
 *
 *      ------------------------------------------------------------------------
 */

/*----------------------------------------------------------------------------*/

struct dummy_userdata {

  dtn_json_io_buffer *self;
  dtn_list *list;
  int socket;

  bool error;
};

/*----------------------------------------------------------------------------*/

static void dummy_receive_no_drop(void *userdata, int socket,
                                  dtn_item *value) {

  struct dummy_userdata *data = (struct dummy_userdata *)userdata;
  dtn_list_push(data->list, value);
  data->socket = socket;
  data->error = false;
  return;
}

/*----------------------------------------------------------------------------*/

static void dummy_receive_drop(void *userdata, int socket,
                               dtn_item *value) {

  struct dummy_userdata *data = (struct dummy_userdata *)userdata;
  dtn_list_push(data->list, value);
  data->socket = socket;
  dtn_json_io_buffer_drop(data->self, socket);
  data->error = false;
  return;
}

/*----------------------------------------------------------------------------*/

static void dummy_error(void *userdata, int socket) {

  struct dummy_userdata *data = (struct dummy_userdata *)userdata;

  data->socket = socket;
  data->error = true;
  return;
}

/*----------------------------------------------------------------------------*/

static bool dummy_init(struct dummy_userdata *dummy) {

  if (!dummy)
    return false;

  dummy->socket = 0;

  dummy->list =
      dtn_list_create((dtn_list_config){.item.free = dtn_item_free});
  return true;
}

/*----------------------------------------------------------------------------*/

static bool dummy_deinit(struct dummy_userdata *dummy) {

  if (!dummy)
    return false;

  dummy->self = NULL;
  dummy->list = dtn_list_free(dummy->list);
  return true;
}

/*----------------------------------------------------------------------------*/

int test_dtn_json_io_buffer_create() {

  struct dummy_userdata dummy;
  testrun(dummy_init(&dummy));

  dtn_json_io_buffer_config config = (dtn_json_io_buffer_config){0};

  testrun(!dtn_json_io_buffer_create(config));

  config.callback.userdata = &dummy;
  config.callback.success = dummy_receive_no_drop;

  dtn_json_io_buffer *self = dtn_json_io_buffer_create(config);

  testrun(self);
  testrun(self->dict);
  testrun(self->config.debug == false);

  testrun(NULL == dtn_json_io_buffer_free(self));

  testrun(dummy_deinit(&dummy));
  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_json_io_buffer_free() {

  struct dummy_userdata dummy;
  testrun(dummy_init(&dummy));

  dtn_json_io_buffer_config config = (dtn_json_io_buffer_config){
      .callback.userdata = &dummy, .callback.success = dummy_receive_no_drop};

  dtn_json_io_buffer *self = dtn_json_io_buffer_create(config);

  testrun(NULL == dtn_json_io_buffer_free(NULL));
  testrun(NULL == dtn_json_io_buffer_free(self));

  /* check with content */

  self = dtn_json_io_buffer_create(config);

  char *valid_json = "{\"key\":";

  dtn_memory_pointer ptr = (dtn_memory_pointer){.start = (uint8_t *)valid_json,
                                              .length = strlen(valid_json)};

  dummy.socket = 0;
  testrun(0 == dtn_list_count(dummy.list));

  testrun(dtn_json_io_buffer_push(self, 1, ptr));
  testrun(dtn_json_io_buffer_push(self, 2, ptr));
  testrun(dtn_json_io_buffer_push(self, 3, ptr));

  testrun(0 == dummy.socket);
  testrun(0 == dtn_list_count(dummy.list));

  testrun(dtn_dict_get(self->dict, (void *)(intptr_t)1));
  testrun(dtn_dict_get(self->dict, (void *)(intptr_t)2));
  testrun(dtn_dict_get(self->dict, (void *)(intptr_t)3));

  testrun(NULL == dtn_json_io_buffer_free(self));

  testrun(dummy_deinit(&dummy));
  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_json_io_buffer_push() {

  dtn_item *val = NULL;
  dtn_buffer *buffer = NULL;

  struct dummy_userdata dummy;
  testrun(dummy_init(&dummy));

  dtn_json_io_buffer_config config =
      (dtn_json_io_buffer_config){.callback.userdata = &dummy,
                                 .callback.success = dummy_receive_no_drop,
                                 .callback.failure = dummy_error};

  dtn_json_io_buffer *self = dtn_json_io_buffer_create(config);
  testrun(self);
  dummy.self = self;

  char *str = "{\"key\":";
  intptr_t key = 1;

  testrun(!dtn_json_io_buffer_push(
      NULL, key,
      (dtn_memory_pointer){.start = (uint8_t *)str, .length = strlen(str)}));

  testrun(!dtn_json_io_buffer_push(
      self, key, (dtn_memory_pointer){.start = NULL, .length = strlen(str)}));

  testrun(!dtn_json_io_buffer_push(
      self, key, (dtn_memory_pointer){.start = (uint8_t *)str, .length = 0}));

  testrun(key == dummy.socket);
  testrun(true == dummy.error);
  testrun(0 == dtn_list_count(dummy.list));

  // check negative indicies (allowed as ID based content from -INT to + INT)

  testrun(dtn_json_io_buffer_push(
      self, -1,
      (dtn_memory_pointer){.start = (uint8_t *)str, .length = strlen(str)}));

  key = -1;

  // no callback yet
  testrun(1 == dummy.socket);
  testrun(true == dummy.error);
  // content added
  buffer = dtn_dict_get(self->dict, (void *)key);
  testrun(buffer);
  testrun(0 == memcmp(str, buffer->start, buffer->length));
  testrun(dtn_json_io_buffer_drop(self, key));

  // check socket id
  key = 1;

  testrun(dtn_json_io_buffer_push(
      self, key,
      (dtn_memory_pointer){.start = (uint8_t *)str, .length = strlen(str)}));

  buffer = dtn_dict_get(self->dict, (void *)key);
  testrun(buffer);
  testrun(0 == memcmp(str, buffer->start, buffer->length));

  str = "\"val\"";
  testrun(dtn_json_io_buffer_push(
      self, key,
      (dtn_memory_pointer){.start = (uint8_t *)str, .length = strlen(str)}));

  // no callback yet
  testrun(1 == dummy.socket);
  testrun(true == dummy.error);
  // content added
  buffer = dtn_dict_get(self->dict, (void *)key);
  testrun(buffer);
  testrun(0 == memcmp("{\"key\":\"val\"", buffer->start, buffer->length));

  str = "}{\"next\":";
  testrun(dtn_json_io_buffer_push(
      self, key,
      (dtn_memory_pointer){.start = (uint8_t *)str, .length = strlen(str)}));

  testrun(1 == dummy.socket);
  testrun(1 == dtn_list_count(dummy.list));

  buffer = dtn_dict_get(self->dict, (void *)key);
  testrun(buffer);
  testrun(0 == memcmp("{\"next\":", buffer->start, buffer->length));

  val = dtn_list_pop(dummy.list);
  testrun(dtn_item_is_object(val));
  val = dtn_item_free(val);

  // expect a drop of all content
  str = "invalid content in terms of json";
  testrun(!dtn_json_io_buffer_push(
      self, key,
      (dtn_memory_pointer){.start = (uint8_t *)str, .length = strlen(str)}));

  testrun(!dtn_dict_get(self->dict, (void *)key));
  testrun(0 == dtn_list_count(dummy.list));

  // try to add invalid content (nothing added yet at socket id)
  testrun(dtn_dict_is_empty(self->dict));
  testrun(!dtn_json_io_buffer_push(
      self, key,
      (dtn_memory_pointer){.start = (uint8_t *)str, .length = strlen(str)}));
  testrun(!dtn_dict_get(self->dict, (void *)key));
  testrun(dtn_dict_is_empty(self->dict));

  str = "{:";
  testrun(!dtn_json_io_buffer_push(
      self, key,
      (dtn_memory_pointer){.start = (uint8_t *)str, .length = strlen(str)}));
  testrun(dtn_dict_is_empty(self->dict));

  str = "[\"some valid array\"]";
  testrun(dtn_json_io_buffer_push(
      self, key,
      (dtn_memory_pointer){.start = (uint8_t *)str, .length = strlen(str)}));
  testrun(!dtn_dict_is_empty(self->dict));
  buffer = dtn_dict_get(self->dict, (void *)key);
  testrun(buffer);
  testrun(0 == buffer->length);
  testrun(1 == dummy.socket);
  testrun(1 == dtn_list_count(dummy.list));
  val = dtn_list_pop(dummy.list);
  testrun(dtn_item_is_array(val));
  val = dtn_item_free(val);

  str = "[\"some incomplete array ";
  testrun(dtn_json_io_buffer_push(
      self, key,
      (dtn_memory_pointer){.start = (uint8_t *)str, .length = strlen(str)}));
  testrun(!dtn_dict_is_empty(self->dict));
  buffer = dtn_dict_get(self->dict, (void *)key);
  testrun(buffer);
  testrun(0 ==
          memcmp("[\"some incomplete array ", buffer->start, buffer->length));
  testrun(dtn_json_io_buffer_drop(self, key));

  str = "\"some valid string\"";
  testrun(dtn_json_io_buffer_push(
      self, key,
      (dtn_memory_pointer){.start = (uint8_t *)str, .length = strlen(str)}));
  testrun(!dtn_dict_is_empty(self->dict));
  buffer = dtn_dict_get(self->dict, (void *)key);
  testrun(buffer);
  testrun(0 == buffer->length);
  testrun(1 == dummy.socket);
  testrun(1 == dtn_list_count(dummy.list));
  val = dtn_list_pop(dummy.list);
  testrun(dtn_item_is_string(val));
  val = dtn_item_free(val);
  testrun(dtn_json_io_buffer_drop(self, key));

  str = "\"some incomplete string";
  testrun(dtn_json_io_buffer_push(
      self, key,
      (dtn_memory_pointer){.start = (uint8_t *)str, .length = strlen(str)}));
  testrun(!dtn_dict_is_empty(self->dict));
  buffer = dtn_dict_get(self->dict, (void *)key);
  testrun(buffer);
  testrun(0 ==
          memcmp("\"some incomplete string", buffer->start, buffer->length));
  testrun(0 == dtn_list_count(dummy.list));
  testrun(dtn_json_io_buffer_drop(self, key));

  str = "null";
  testrun(dtn_json_io_buffer_push(
      self, key,
      (dtn_memory_pointer){.start = (uint8_t *)str, .length = strlen(str)}));
  testrun(!dtn_dict_is_empty(self->dict));
  buffer = dtn_dict_get(self->dict, (void *)key);
  testrun(buffer);
  testrun(0 == buffer->length);
  testrun(1 == dummy.socket);
  testrun(1 == dtn_list_count(dummy.list));
  val = dtn_list_pop(dummy.list);
  testrun(dtn_item_is_null(val));
  val = dtn_item_free(val);
  testrun(dtn_json_io_buffer_drop(self, key));

  str = "true";
  testrun(dtn_json_io_buffer_push(
      self, key,
      (dtn_memory_pointer){.start = (uint8_t *)str, .length = strlen(str)}));
  testrun(!dtn_dict_is_empty(self->dict));
  buffer = dtn_dict_get(self->dict, (void *)key);
  testrun(buffer);
  testrun(0 == buffer->length);
  testrun(1 == dummy.socket);
  testrun(1 == dtn_list_count(dummy.list));
  val = dtn_list_pop(dummy.list);
  testrun(dtn_item_is_true(val));
  val = dtn_item_free(val);
  testrun(dtn_json_io_buffer_drop(self, key));

  str = "false";
  testrun(dtn_json_io_buffer_push(
      self, key,
      (dtn_memory_pointer){.start = (uint8_t *)str, .length = strlen(str)}));
  testrun(!dtn_dict_is_empty(self->dict));
  buffer = dtn_dict_get(self->dict, (void *)key);
  testrun(buffer);
  testrun(0 == buffer->length);
  testrun(1 == dummy.socket);
  testrun(1 == dtn_list_count(dummy.list));
  val = dtn_list_pop(dummy.list);
  testrun(dtn_item_is_false(val));
  val = dtn_item_free(val);
  testrun(dtn_json_io_buffer_drop(self, key));

  str = "tr";
  testrun(dtn_json_io_buffer_push(
      self, key,
      (dtn_memory_pointer){.start = (uint8_t *)str, .length = strlen(str)}));
  testrun(!dtn_dict_is_empty(self->dict));
  buffer = dtn_dict_get(self->dict, (void *)key);
  testrun(buffer);
  testrun(0 == memcmp("tr", buffer->start, buffer->length));
  testrun(dtn_json_io_buffer_drop(self, key));

  str = "fal";
  testrun(dtn_json_io_buffer_push(
      self, key,
      (dtn_memory_pointer){.start = (uint8_t *)str, .length = strlen(str)}));
  testrun(!dtn_dict_is_empty(self->dict));
  buffer = dtn_dict_get(self->dict, (void *)key);
  testrun(buffer);
  testrun(0 == memcmp("fal", buffer->start, buffer->length));
  testrun(dtn_json_io_buffer_drop(self, key));

  str = "n";
  testrun(dtn_json_io_buffer_push(
      self, key,
      (dtn_memory_pointer){.start = (uint8_t *)str, .length = strlen(str)}));
  testrun(!dtn_dict_is_empty(self->dict));
  buffer = dtn_dict_get(self->dict, (void *)key);
  testrun(buffer);
  testrun(0 == memcmp("n", buffer->start, buffer->length));
  testrun(dtn_json_io_buffer_drop(self, key));

  str = "{\"some incomplete object";
  testrun(dtn_json_io_buffer_push(
      self, key,
      (dtn_memory_pointer){.start = (uint8_t *)str, .length = strlen(str)}));
  testrun(!dtn_dict_is_empty(self->dict));
  buffer = dtn_dict_get(self->dict, (void *)key);
  testrun(buffer);
  testrun(0 ==
          memcmp("{\"some incomplete object", buffer->start, buffer->length));
  testrun(dtn_json_io_buffer_drop(self, key));

  testrun(0 == dtn_list_count(dummy.list));
  str = "{\"key\":\"1\"} {";
  testrun(dtn_json_io_buffer_push(
      self, key,
      (dtn_memory_pointer){.start = (uint8_t *)str, .length = strlen(str)}));
  testrun(!dtn_dict_is_empty(self->dict));
  testrun(1 == dtn_list_count(dummy.list));
  testrun(dtn_list_clear(dummy.list));
  buffer = dtn_dict_get(self->dict, (void *)key);
  testrun(buffer);
  testrun(0 == memcmp(" {", buffer->start, buffer->length));
  testrun(dtn_json_io_buffer_drop(self, key));

  str = "{\"key\":\"1\"} {    ";
  testrun(dtn_json_io_buffer_push(
      self, key,
      (dtn_memory_pointer){.start = (uint8_t *)str, .length = strlen(str)}));
  testrun(!dtn_dict_is_empty(self->dict));
  testrun(1 == dtn_list_count(dummy.list));
  testrun(dtn_list_clear(dummy.list));
  buffer = dtn_dict_get(self->dict, (void *)key);
  testrun(buffer);
  testrun(0 == memcmp(" {    ", buffer->start, buffer->length));
  testrun(dtn_json_io_buffer_drop(self, key));

  // nothing added, as the whole buffer does not match
  testrun(0 == dtn_list_count(dummy.list));
  testrun(dummy.error == false);
  str = "{\"key\":\"1\"} {   : ";
  testrun(!dtn_json_io_buffer_push(
      self, key,
      (dtn_memory_pointer){.start = (uint8_t *)str, .length = strlen(str)}));
  testrun(0 == dtn_list_count(dummy.list));
  testrun(dtn_dict_is_empty(self->dict));
  testrun(dummy.error == true);

  // nothing added, as the whole buffer does not match
  dummy.error = false;
  str = "{\"key\":\"1\"} {[";
  testrun(!dtn_json_io_buffer_push(
      self, key,
      (dtn_memory_pointer){.start = (uint8_t *)str, .length = strlen(str)}));
  testrun(0 == dtn_list_count(dummy.list));
  testrun(dtn_dict_is_empty(self->dict));
  testrun(dummy.error == true);

  str = "{} {\"key\":\"1\"} {\"key\":\"2\"} {\"key\":\"3\"} {";
  testrun(dtn_json_io_buffer_push(
      self, key,
      (dtn_memory_pointer){.start = (uint8_t *)str, .length = strlen(str)}));
  testrun(!dtn_dict_is_empty(self->dict));
  buffer = dtn_dict_get(self->dict, (void *)key);
  testrun(buffer);
  testrun(0 == memcmp(" {", buffer->start, buffer->length));
  testrun(4 == dtn_list_count(dummy.list));
  val = dtn_list_pop(dummy.list);
  testrun(dtn_item_is_object(val));
  val = dtn_item_free(val);
  val = dtn_list_pop(dummy.list);
  testrun(dtn_item_is_object(val));
  val = dtn_item_free(val);
  val = dtn_list_pop(dummy.list);
  testrun(dtn_item_is_object(val));
  val = dtn_item_free(val);
  val = dtn_list_pop(dummy.list);
  testrun(dtn_item_is_object(val));
  val = dtn_item_free(val);
  testrun(dtn_json_io_buffer_drop(self, key));
  testrun(dtn_list_clear(dummy.list));

  str = "null null";
  testrun(dtn_json_io_buffer_push(
      self, key,
      (dtn_memory_pointer){.start = (uint8_t *)str, .length = strlen(str)}));
  testrun(!dtn_dict_is_empty(self->dict));
  testrun(2 == dtn_list_count(dummy.list));
  val = dtn_list_pop(dummy.list);
  testrun(dtn_item_is_null(val));
  val = dtn_item_free(val);
  val = dtn_list_pop(dummy.list);
  testrun(dtn_item_is_null(val));
  val = dtn_item_free(val);
  testrun(dtn_json_io_buffer_drop(self, key));
  testrun(dtn_list_clear(dummy.list));

  str = "true true";
  testrun(dtn_json_io_buffer_push(
      self, key,
      (dtn_memory_pointer){.start = (uint8_t *)str, .length = strlen(str)}));
  testrun(!dtn_dict_is_empty(self->dict));
  testrun(2 == dtn_list_count(dummy.list));
  val = dtn_list_pop(dummy.list);
  testrun(dtn_item_is_true(val));
  val = dtn_item_free(val);
  val = dtn_list_pop(dummy.list);
  testrun(dtn_item_is_true(val));
  val = dtn_item_free(val);
  testrun(dtn_json_io_buffer_drop(self, key));
  testrun(dtn_list_clear(dummy.list));

  str = "false false";
  testrun(dtn_json_io_buffer_push(
      self, key,
      (dtn_memory_pointer){.start = (uint8_t *)str, .length = strlen(str)}));
  testrun(!dtn_dict_is_empty(self->dict));
  testrun(2 == dtn_list_count(dummy.list));
  val = dtn_list_pop(dummy.list);
  testrun(dtn_item_is_false(val));
  val = dtn_item_free(val);
  val = dtn_list_pop(dummy.list);
  testrun(dtn_item_is_false(val));
  val = dtn_item_free(val);
  testrun(dtn_json_io_buffer_drop(self, key));
  testrun(dtn_list_clear(dummy.list));

  str = "false true false true false";
  testrun(dtn_json_io_buffer_push(
      self, key,
      (dtn_memory_pointer){.start = (uint8_t *)str, .length = strlen(str)}));
  testrun(!dtn_dict_is_empty(self->dict));
  testrun(5 == dtn_list_count(dummy.list));
  val = dtn_list_pop(dummy.list);
  testrun(dtn_item_is_false(val));
  val = dtn_item_free(val);
  val = dtn_list_pop(dummy.list);
  testrun(dtn_item_is_true(val));
  val = dtn_item_free(val);
  val = dtn_list_pop(dummy.list);
  testrun(dtn_item_is_false(val));
  val = dtn_item_free(val);
  val = dtn_list_pop(dummy.list);
  testrun(dtn_item_is_true(val));
  val = dtn_item_free(val);
  val = dtn_list_pop(dummy.list);
  testrun(dtn_item_is_false(val));
  val = dtn_item_free(val);
  testrun(dtn_json_io_buffer_drop(self, key));
  testrun(dtn_list_clear(dummy.list));

  str = "false true false true null";
  testrun(dtn_json_io_buffer_push(
      self, key,
      (dtn_memory_pointer){.start = (uint8_t *)str, .length = strlen(str)}));
  testrun(!dtn_dict_is_empty(self->dict));
  testrun(5 == dtn_list_count(dummy.list));
  val = dtn_list_get(dummy.list, 1);
  testrun(dtn_item_is_false(val));
  val = dtn_list_get(dummy.list, 2);
  testrun(dtn_item_is_true(val));
  val = dtn_list_get(dummy.list, 3);
  testrun(dtn_item_is_false(val));
  val = dtn_list_get(dummy.list, 4);
  testrun(dtn_item_is_true(val));
  val = dtn_list_get(dummy.list, 5);
  testrun(dtn_item_is_null(val));
  testrun(dtn_json_io_buffer_drop(self, key));
  testrun(dtn_list_clear(dummy.list));

  str = "null true false";
  testrun(dtn_json_io_buffer_push(
      self, key,
      (dtn_memory_pointer){.start = (uint8_t *)str, .length = strlen(str)}));
  testrun(!dtn_dict_is_empty(self->dict));
  testrun(3 == dtn_list_count(dummy.list));
  val = dtn_list_get(dummy.list, 1);
  testrun(dtn_item_is_null(val));
  val = dtn_list_get(dummy.list, 2);
  testrun(dtn_item_is_true(val));
  val = dtn_list_get(dummy.list, 3);
  testrun(dtn_item_is_false(val));
  testrun(dtn_json_io_buffer_drop(self, key));
  testrun(dtn_list_clear(dummy.list));

  // we leave out number, as the implementation of
  // parsing currently requires a closing element ,}]
  // after optional whitespace
  str = "{} [] \"string\" true false null";
  testrun(dtn_json_io_buffer_push(
      self, key,
      (dtn_memory_pointer){.start = (uint8_t *)str, .length = strlen(str)}));
  testrun(!dtn_dict_is_empty(self->dict));
  testrun(6 == dtn_list_count(dummy.list));
  val = dtn_list_get(dummy.list, 1);
  testrun(dtn_item_is_object(val));
  val = dtn_list_get(dummy.list, 2);
  testrun(dtn_item_is_array(val));
  val = dtn_list_get(dummy.list, 3);
  testrun(dtn_item_is_string(val));
  val = dtn_list_get(dummy.list, 4);
  testrun(dtn_item_is_true(val));
  val = dtn_list_get(dummy.list, 5);
  testrun(dtn_item_is_false(val));
  val = dtn_list_get(dummy.list, 6);
  testrun(dtn_item_is_null(val));
  testrun(dtn_json_io_buffer_drop(self, key));
  testrun(dtn_list_clear(dummy.list));

  str = "{}[]\"string\"truefalsenull";
  testrun(dtn_json_io_buffer_push(
      self, key,
      (dtn_memory_pointer){.start = (uint8_t *)str, .length = strlen(str)}));
  testrun(!dtn_dict_is_empty(self->dict));
  testrun(6 == dtn_list_count(dummy.list));
  val = dtn_list_get(dummy.list, 1);
  testrun(dtn_item_is_object(val));
  val = dtn_list_get(dummy.list, 2);
  testrun(dtn_item_is_array(val));
  val = dtn_list_get(dummy.list, 3);
  testrun(dtn_item_is_string(val));
  val = dtn_list_get(dummy.list, 4);
  testrun(dtn_item_is_true(val));
  val = dtn_list_get(dummy.list, 5);
  testrun(dtn_item_is_false(val));
  val = dtn_list_get(dummy.list, 6);
  testrun(dtn_item_is_null(val));
  testrun(dtn_json_io_buffer_drop(self, key));
  testrun(dtn_list_clear(dummy.list));

  str = "{}[]\"string\"truefalsenull";
  testrun(dtn_json_io_buffer_push(
      self, key,
      (dtn_memory_pointer){.start = (uint8_t *)str, .length = strlen(str)}));
  testrun(!dtn_dict_is_empty(self->dict));
  testrun(6 == dtn_list_count(dummy.list));
  val = dtn_list_get(dummy.list, 1);
  testrun(dtn_item_is_object(val));
  val = dtn_list_get(dummy.list, 2);
  testrun(dtn_item_is_array(val));
  val = dtn_list_get(dummy.list, 3);
  testrun(dtn_item_is_string(val));
  val = dtn_list_get(dummy.list, 4);
  testrun(dtn_item_is_true(val));
  val = dtn_list_get(dummy.list, 5);
  testrun(dtn_item_is_false(val));
  val = dtn_list_get(dummy.list, 6);
  testrun(dtn_item_is_null(val));
  testrun(dtn_json_io_buffer_drop(self, key));
  testrun(dtn_list_clear(dummy.list));

  str = "{} {";

  testrun(dtn_json_io_buffer_push(
      self, key,
      (dtn_memory_pointer){.start = (uint8_t *)str, .length = strlen(str)}));

  testrun(!dtn_dict_is_empty(self->dict));
  buffer = dtn_dict_get(self->dict, (void *)key);
  testrun(buffer);
  testrun(0 == memcmp(" {", buffer->start, buffer->length));
  testrun(1 == dtn_list_count(dummy.list));
  testrun(dtn_list_clear(dummy.list));

  str = "} {}{}";
  testrun(0 == dtn_list_count(dummy.list));

  testrun(dtn_json_io_buffer_push(
      self, key,
      (dtn_memory_pointer){.start = (uint8_t *)str, .length = strlen(str)}));

  testrun(!dtn_dict_is_empty(self->dict));
  buffer = dtn_dict_get(self->dict, (void *)key);
  testrun(buffer);
  testrun(0 == buffer->length);
  testrun(3 == dtn_list_count(dummy.list));
  testrun(dtn_list_clear(dummy.list));

  // check drop during receive
  self->config.callback.success = dummy_receive_drop;
  str = "{}{}{}";

  testrun(0 == dtn_list_count(dummy.list));

  testrun(!dtn_json_io_buffer_push(
      self, key,
      (dtn_memory_pointer){.start = (uint8_t *)str, .length = strlen(str)}));

  // check we received the first JSON and dropped
  testrun(dtn_dict_is_empty(self->dict));
  testrun(1 == dtn_list_count(dummy.list));
  testrun(dtn_list_clear(dummy.list));

  testrun(NULL == dtn_json_io_buffer_free(self));

  testrun(dummy_deinit(&dummy));
  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_json_io_buffer_drop() {

  dtn_buffer *buffer = NULL;

  struct dummy_userdata dummy;
  testrun(dummy_init(&dummy));

  dtn_json_io_buffer_config config =
      (dtn_json_io_buffer_config){.callback.userdata = &dummy,
                                 .callback.success = dummy_receive_no_drop,
                                 .callback.failure = dummy_error};

  dtn_json_io_buffer *self = dtn_json_io_buffer_create(config);
  testrun(self);
  dummy.self = self;

  char *str = "{\"key\":";
  intptr_t key = 1;

  for (size_t i = 1; i < 10; i++) {

    key = i;
    testrun(dtn_json_io_buffer_push(
        self, key,
        (dtn_memory_pointer){.start = (uint8_t *)str, .length = strlen(str)}));

    buffer = dtn_dict_get(self->dict, (void *)key);
    testrun(buffer);
    testrun(0 == memcmp(str, buffer->start, buffer->length));
  }

  testrun(9 == dtn_dict_count(self->dict));
  testrun(!dtn_json_io_buffer_drop(NULL, 1));
  testrun(dtn_dict_get(self->dict, (void *)(intptr_t)1));

  testrun(dtn_json_io_buffer_drop(self, 1));
  testrun(8 == dtn_dict_count(self->dict));
  testrun(!dtn_dict_get(self->dict, (void *)(intptr_t)1));

  testrun(dtn_json_io_buffer_drop(self, 1));
  testrun(8 == dtn_dict_count(self->dict));
  testrun(!dtn_dict_get(self->dict, (void *)(intptr_t)1));

  testrun(dtn_json_io_buffer_drop(self, 2));
  testrun(7 == dtn_dict_count(self->dict));
  testrun(!dtn_dict_get(self->dict, (void *)(intptr_t)1));
  testrun(!dtn_dict_get(self->dict, (void *)(intptr_t)2));

  testrun(dtn_json_io_buffer_drop(self, 3));
  testrun(6 == dtn_dict_count(self->dict));
  testrun(!dtn_dict_get(self->dict, (void *)(intptr_t)1));
  testrun(!dtn_dict_get(self->dict, (void *)(intptr_t)2));
  testrun(!dtn_dict_get(self->dict, (void *)(intptr_t)3));

  testrun(NULL == dtn_json_io_buffer_free(self));

  testrun(dummy_deinit(&dummy));

  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

/*
 *      ------------------------------------------------------------------------
 *
 *      TEST CLUSTER                                                    #CLUSTER
 *
 *      ------------------------------------------------------------------------
 */

int all_tests() {

  testrun_init();

  testrun_test(test_dtn_json_io_buffer_create);
  testrun_test(test_dtn_json_io_buffer_free);

  testrun_test(test_dtn_json_io_buffer_push);
  testrun_test(test_dtn_json_io_buffer_drop);
  return testrun_counter;
}

/*
 *      ------------------------------------------------------------------------
 *
 *      TEST EXECUTION                                                  #EXEC
 *
 *      ------------------------------------------------------------------------
 */

testrun_run(all_tests);
