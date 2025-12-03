/***
        ------------------------------------------------------------------------

        Copyright (c) 2025 German Aerospace Center DLR e.V. (GSOC)

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
        @file           dtn_item_test.c
        @author         Töpfer, Markus

        @date           2025-11-28


        ------------------------------------------------------------------------
*/
#include "dtn_item.c"
#include "../include/testrun.h"

/*
 *      ------------------------------------------------------------------------
 *
 *      TEST CASES                                                      #CASES
 *
 *      ------------------------------------------------------------------------
 */

/*----------------------------------------------------------------------------*/

int test_dtn_item_cast() {

  uint16_t magic_byte = 0;

  for (uint16_t i = 0; i < UINT16_MAX; i++) {

    magic_byte = i;

    if (i == dtn_ITEM_MAGIC_BYTES) {
      testrun(dtn_item_cast(&magic_byte));
    } else {
      testrun(!dtn_item_cast(&magic_byte))
    }
  }

  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_item_object() {

  dtn_item *item = dtn_item_object();
  testrun(dtn_item_is_object(item));
  testrun(dtn_dict_cast(item->config.data));
  testrun(NULL == dtn_item_free(item));

  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_item_array() {

  dtn_item *item = dtn_item_array();
  testrun(dtn_item_is_array(item));
  testrun(dtn_list_cast(item->config.data));
  testrun(NULL == dtn_item_free(item));

  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_item_string() {

  dtn_item *item = dtn_item_string("test");
  testrun(dtn_item_is_string(item));
  testrun(0 == dtn_string_compare(item->config.data, "test"));
  testrun(NULL == dtn_item_free(item));

  testrun(NULL == dtn_item_string(NULL));

  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_item_number() {

  dtn_item *item = dtn_item_number(0);
  testrun(dtn_item_is_number(item));
  testrun(0 == item->config.number);
  testrun(NULL == dtn_item_free(item));

  for (size_t i = 0; i < 100; i++) {

    dtn_item *item = dtn_item_number(i);
    testrun(item);
    testrun(dtn_item_is_number(item));
    testrun(i == item->config.number);
    testrun(NULL == dtn_item_free(item));
  }

  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_item_true() {

  dtn_item *item = dtn_item_true();
  testrun(dtn_item_is_true(item));
  testrun(NULL == dtn_item_free(item));

  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_item_false() {

  dtn_item *item = dtn_item_false();
  testrun(dtn_item_is_false(item));
  testrun(NULL == dtn_item_free(item));

  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_item_null() {

  dtn_item *item = dtn_item_null();
  testrun(dtn_item_is_null(item));
  testrun(NULL == dtn_item_free(item));

  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_item_free() {

  dtn_item *item = dtn_item_null();
  dtn_list *list = dtn_linked_list_create((dtn_list_config){0});

  testrun(list == dtn_item_free(list));
  testrun(NULL == dtn_list_free(list));

  testrun(dtn_item_is_null(item));
  testrun(NULL == dtn_item_free(item));

  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_item_clear() {

  dtn_item *item = dtn_item_null();
  dtn_list *list = dtn_linked_list_create((dtn_list_config){0});

  testrun(!dtn_item_clear(list));
  testrun(NULL == dtn_list_free(list));

  testrun(dtn_item_clear(item));
  testrun(dtn_item_is_null(item));
  testrun(NULL == item->config.data);
  testrun(NULL == dtn_item_free(item));

  item = dtn_item_true();
  testrun(dtn_item_clear(item));
  testrun(dtn_item_is_null(item));
  testrun(NULL == item->config.data);
  testrun(NULL == dtn_item_free(item));

  item = dtn_item_false();
  testrun(dtn_item_clear(item));
  testrun(dtn_item_is_null(item));
  testrun(NULL == item->config.data);
  testrun(NULL == dtn_item_free(item));

  item = dtn_item_number(100);
  testrun(dtn_item_clear(item));
  testrun(dtn_item_is_null(item));
  testrun(NULL == item->config.data);
  testrun(NULL == dtn_item_free(item));

  item = dtn_item_string("test");
  testrun(dtn_item_clear(item));
  testrun(dtn_item_is_null(item));
  testrun(NULL == item->config.data);
  testrun(NULL == dtn_item_free(item));

  item = dtn_item_object();
  testrun(dtn_item_clear(item));
  testrun(dtn_item_is_null(item));
  testrun(NULL == item->config.data);
  testrun(NULL == dtn_item_free(item));

  item = dtn_item_array();
  testrun(dtn_item_clear(item));
  testrun(dtn_item_is_null(item));
  testrun(NULL == item->config.data);
  testrun(NULL == dtn_item_free(item));

  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_item_copy() {

  dtn_item *item = dtn_item_object();
  dtn_item *copy = NULL;

  testrun(dtn_item_object_set(item, "1", dtn_item_null()));
  testrun(dtn_item_object_set(item, "2", dtn_item_true()));
  testrun(dtn_item_object_set(item, "3", dtn_item_false()));
  testrun(dtn_item_object_set(item, "4", dtn_item_string("test")));
  testrun(dtn_item_object_set(item, "5", dtn_item_array()));
  testrun(dtn_item_object_set(item, "6", dtn_item_object()));
  testrun(dtn_item_object_set(item, "7", dtn_item_number(7)));
  testrun(7 == dtn_item_count(item));

  testrun(!dtn_item_copy((void **)&copy, NULL));

  testrun(dtn_item_copy((void **)&copy, item));
  testrun(copy);
  testrun(7 == dtn_item_count(copy));
  testrun(copy != item);

  testrun(dtn_item_is_null((dtn_item *)dtn_item_object_get(copy, "1")));
  testrun(dtn_item_is_true((dtn_item *)dtn_item_object_get(copy, "2")));
  testrun(dtn_item_is_false((dtn_item *)dtn_item_object_get(copy, "3")));
  testrun(dtn_item_is_string((dtn_item *)dtn_item_object_get(copy, "4")));
  testrun(dtn_item_is_array((dtn_item *)dtn_item_object_get(copy, "5")));
  testrun(dtn_item_is_object((dtn_item *)dtn_item_object_get(copy, "6")));
  testrun(dtn_item_is_number((dtn_item *)dtn_item_object_get(copy, "7")));

  testrun(NULL == dtn_item_free(item));
  testrun(NULL == dtn_item_free(copy));

  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_item_dump() {

  dtn_item *item = dtn_item_object();

  testrun(dtn_item_object_set(item, "1", dtn_item_null()));
  testrun(dtn_item_object_set(item, "2", dtn_item_true()));
  testrun(dtn_item_object_set(item, "3", dtn_item_false()));
  testrun(dtn_item_object_set(item, "4", dtn_item_string("test")));
  testrun(dtn_item_object_set(item, "5", dtn_item_array()));
  testrun(dtn_item_object_set(item, "6", dtn_item_object()));
  testrun(dtn_item_object_set(item, "7", dtn_item_number(7)));
  testrun(7 == dtn_item_count(item));

  testrun(dtn_item_dump(stdout, item));
  testrun(NULL == dtn_item_free(item));

  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_item_count() {

  dtn_item *item = dtn_item_null();
  testrun(dtn_item_is_null(item));
  testrun(1 == dtn_item_count(item));
  testrun(NULL == dtn_item_free(item));

  item = dtn_item_true();
  testrun(1 == dtn_item_count(item));
  testrun(NULL == dtn_item_free(item));

  item = dtn_item_false();
  testrun(1 == dtn_item_count(item));
  testrun(NULL == dtn_item_free(item));

  item = dtn_item_number(100);
  testrun(1 == dtn_item_count(item));
  testrun(NULL == dtn_item_free(item));

  item = dtn_item_string("test");
  testrun(1 == dtn_item_count(item));
  testrun(NULL == dtn_item_free(item));

  item = dtn_item_object();
  testrun(0 == dtn_item_count(item));
  testrun(dtn_item_object_set(item, "1", dtn_item_null()));
  testrun(dtn_item_object_set(item, "2", dtn_item_true()));
  testrun(dtn_item_object_set(item, "3", dtn_item_false()));
  testrun(dtn_item_object_set(item, "4", dtn_item_string("test")));
  testrun(dtn_item_object_set(item, "5", dtn_item_array()));
  testrun(dtn_item_object_set(item, "6", dtn_item_object()));
  testrun(dtn_item_object_set(item, "7", dtn_item_number(7)));
  testrun(7 == dtn_item_count(item));
  testrun(NULL == dtn_item_free(item));

  item = dtn_item_array();
  testrun(0 == dtn_item_count(item));
  testrun(dtn_item_array_push(item, dtn_item_null()));
  testrun(dtn_item_array_push(item, dtn_item_true()));
  testrun(dtn_item_array_push(item, dtn_item_false()));
  testrun(dtn_item_array_push(item, dtn_item_string("test")));
  testrun(dtn_item_array_push(item, dtn_item_array()));
  testrun(dtn_item_array_push(item, dtn_item_object()));
  testrun(dtn_item_array_push(item, dtn_item_number(7)));
  testrun(7 == dtn_item_count(item));
  testrun(NULL == dtn_item_free(item));

  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_item_is_object() {

  dtn_item *n = dtn_item_null();
  dtn_item *f = dtn_item_false();
  dtn_item *t = dtn_item_true();
  dtn_item *nbr = dtn_item_number(0);
  dtn_item *str = dtn_item_string("test");
  dtn_item *arr = dtn_item_array();
  dtn_item *obj = dtn_item_object();

  testrun(!dtn_item_is_object(n));
  testrun(!dtn_item_is_object(f));
  testrun(!dtn_item_is_object(t));
  testrun(!dtn_item_is_object(nbr));
  testrun(!dtn_item_is_object(str));
  testrun(!dtn_item_is_object(arr));
  testrun(dtn_item_is_object(obj));

  testrun(NULL == dtn_item_free(n));
  testrun(NULL == dtn_item_free(f));
  testrun(NULL == dtn_item_free(t));
  testrun(NULL == dtn_item_free(nbr));
  testrun(NULL == dtn_item_free(str));
  testrun(NULL == dtn_item_free(arr));
  testrun(NULL == dtn_item_free(obj));

  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_item_is_array() {

  dtn_item *n = dtn_item_null();
  dtn_item *f = dtn_item_false();
  dtn_item *t = dtn_item_true();
  dtn_item *nbr = dtn_item_number(0);
  dtn_item *str = dtn_item_string("test");
  dtn_item *arr = dtn_item_array();
  dtn_item *obj = dtn_item_object();

  testrun(!dtn_item_is_array(n));
  testrun(!dtn_item_is_array(f));
  testrun(!dtn_item_is_array(t));
  testrun(!dtn_item_is_array(nbr));
  testrun(!dtn_item_is_array(str));
  testrun(dtn_item_is_array(arr));
  testrun(!dtn_item_is_array(obj));

  testrun(NULL == dtn_item_free(n));
  testrun(NULL == dtn_item_free(f));
  testrun(NULL == dtn_item_free(t));
  testrun(NULL == dtn_item_free(nbr));
  testrun(NULL == dtn_item_free(str));
  testrun(NULL == dtn_item_free(arr));
  testrun(NULL == dtn_item_free(obj));

  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_item_is_string() {

  dtn_item *n = dtn_item_null();
  dtn_item *f = dtn_item_false();
  dtn_item *t = dtn_item_true();
  dtn_item *nbr = dtn_item_number(0);
  dtn_item *str = dtn_item_string("test");
  dtn_item *arr = dtn_item_array();
  dtn_item *obj = dtn_item_object();

  testrun(!dtn_item_is_string(n));
  testrun(!dtn_item_is_string(f));
  testrun(!dtn_item_is_string(t));
  testrun(!dtn_item_is_string(nbr));
  testrun(dtn_item_is_string(str));
  testrun(!dtn_item_is_string(arr));
  testrun(!dtn_item_is_string(obj));

  testrun(NULL == dtn_item_free(n));
  testrun(NULL == dtn_item_free(f));
  testrun(NULL == dtn_item_free(t));
  testrun(NULL == dtn_item_free(nbr));
  testrun(NULL == dtn_item_free(str));
  testrun(NULL == dtn_item_free(arr));
  testrun(NULL == dtn_item_free(obj));

  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_item_is_number() {

  dtn_item *n = dtn_item_null();
  dtn_item *f = dtn_item_false();
  dtn_item *t = dtn_item_true();
  dtn_item *nbr = dtn_item_number(0);
  dtn_item *str = dtn_item_string("test");
  dtn_item *arr = dtn_item_array();
  dtn_item *obj = dtn_item_object();

  testrun(!dtn_item_is_number(n));
  testrun(!dtn_item_is_number(f));
  testrun(!dtn_item_is_number(t));
  testrun(dtn_item_is_number(nbr));
  testrun(!dtn_item_is_number(str));
  testrun(!dtn_item_is_number(arr));
  testrun(!dtn_item_is_number(obj));

  testrun(NULL == dtn_item_free(n));
  testrun(NULL == dtn_item_free(f));
  testrun(NULL == dtn_item_free(t));
  testrun(NULL == dtn_item_free(nbr));
  testrun(NULL == dtn_item_free(str));
  testrun(NULL == dtn_item_free(arr));
  testrun(NULL == dtn_item_free(obj));

  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_item_is_null() {

  dtn_item *n = dtn_item_null();
  dtn_item *f = dtn_item_false();
  dtn_item *t = dtn_item_true();
  dtn_item *nbr = dtn_item_number(0);
  dtn_item *str = dtn_item_string("test");
  dtn_item *arr = dtn_item_array();
  dtn_item *obj = dtn_item_object();

  testrun(dtn_item_is_null(n));
  testrun(!dtn_item_is_null(f));
  testrun(!dtn_item_is_null(t));
  testrun(!dtn_item_is_null(nbr));
  testrun(!dtn_item_is_null(str));
  testrun(!dtn_item_is_null(arr));
  testrun(!dtn_item_is_null(obj));

  testrun(NULL == dtn_item_free(n));
  testrun(NULL == dtn_item_free(f));
  testrun(NULL == dtn_item_free(t));
  testrun(NULL == dtn_item_free(nbr));
  testrun(NULL == dtn_item_free(str));
  testrun(NULL == dtn_item_free(arr));
  testrun(NULL == dtn_item_free(obj));

  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_item_is_false() {

  dtn_item *n = dtn_item_null();
  dtn_item *f = dtn_item_false();
  dtn_item *t = dtn_item_true();
  dtn_item *nbr = dtn_item_number(0);
  dtn_item *str = dtn_item_string("test");
  dtn_item *arr = dtn_item_array();
  dtn_item *obj = dtn_item_object();

  testrun(!dtn_item_is_false(n));
  testrun(dtn_item_is_false(f));
  testrun(!dtn_item_is_false(t));
  testrun(!dtn_item_is_false(nbr));
  testrun(!dtn_item_is_false(str));
  testrun(!dtn_item_is_false(arr));
  testrun(!dtn_item_is_false(obj));

  testrun(NULL == dtn_item_free(n));
  testrun(NULL == dtn_item_free(f));
  testrun(NULL == dtn_item_free(t));
  testrun(NULL == dtn_item_free(nbr));
  testrun(NULL == dtn_item_free(str));
  testrun(NULL == dtn_item_free(arr));
  testrun(NULL == dtn_item_free(obj));

  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_item_is_true() {

  dtn_item *n = dtn_item_null();
  dtn_item *f = dtn_item_false();
  dtn_item *t = dtn_item_true();
  dtn_item *nbr = dtn_item_number(0);
  dtn_item *str = dtn_item_string("test");
  dtn_item *arr = dtn_item_array();
  dtn_item *obj = dtn_item_object();

  testrun(!dtn_item_is_true(n));
  testrun(!dtn_item_is_true(f));
  testrun(dtn_item_is_true(t));
  testrun(!dtn_item_is_true(nbr));
  testrun(!dtn_item_is_true(str));
  testrun(!dtn_item_is_true(arr));
  testrun(!dtn_item_is_true(obj));

  testrun(NULL == dtn_item_free(n));
  testrun(NULL == dtn_item_free(f));
  testrun(NULL == dtn_item_free(t));
  testrun(NULL == dtn_item_free(nbr));
  testrun(NULL == dtn_item_free(str));
  testrun(NULL == dtn_item_free(arr));
  testrun(NULL == dtn_item_free(obj));

  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_item_object_set() {

  dtn_item *n = dtn_item_null();
  dtn_item *f = dtn_item_false();
  dtn_item *t = dtn_item_true();
  dtn_item *nbr = dtn_item_number(0);
  dtn_item *str = dtn_item_string("test");
  dtn_item *arr = dtn_item_array();
  dtn_item *obj = dtn_item_object();

  dtn_item *val = dtn_item_number(1234);

  testrun(!dtn_item_object_set(n, "key", val));
  testrun(!dtn_item_object_set(f, "key", val));
  testrun(!dtn_item_object_set(t, "key", val));
  testrun(!dtn_item_object_set(str, "key", val));
  testrun(!dtn_item_object_set(nbr, "key", val));
  testrun(!dtn_item_object_set(arr, "key", val));
  testrun(dtn_item_object_set(obj, "key", val));
  testrun(1 == dtn_item_count(obj));
  testrun(1234 ==
          dtn_item_get_number((dtn_item *)dtn_item_object_get(obj, "key")));

  testrun(NULL == dtn_item_free(n));
  testrun(NULL == dtn_item_free(f));
  testrun(NULL == dtn_item_free(t));
  testrun(NULL == dtn_item_free(nbr));
  testrun(NULL == dtn_item_free(str));
  testrun(NULL == dtn_item_free(arr));
  testrun(NULL == dtn_item_free(obj));

  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_item_object_delete() {

  dtn_item *item = dtn_item_object();
  dtn_item *obj = dtn_item_object();

  dtn_item *val = dtn_item_number(1234);

  testrun(dtn_item_object_set(item, "1", val));
  testrun(dtn_item_object_set(item, "2", obj));
  testrun(2 == dtn_item_count(item));

  testrun(dtn_item_object_delete(item, "2"));
  testrun(1 == dtn_item_count(item));

  testrun(dtn_item_object_delete(item, "1"));
  testrun(0 == dtn_item_count(item));

  testrun(NULL == dtn_item_free(item));

  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_item_object_remove() {

  dtn_item *item = dtn_item_object();
  dtn_item *obj = dtn_item_object();

  dtn_item *val = dtn_item_number(1234);

  testrun(dtn_item_object_set(item, "1", val));
  testrun(dtn_item_object_set(item, "2", obj));
  testrun(2 == dtn_item_count(item));

  testrun(obj == dtn_item_object_remove(item, "2"));
  testrun(1 == dtn_item_count(item));

  testrun(val == dtn_item_object_remove(item, "1"));
  testrun(0 == dtn_item_count(item));

  testrun(NULL == dtn_item_free(item));
  testrun(NULL == dtn_item_free(val));
  testrun(NULL == dtn_item_free(obj));

  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_item_object_get() {

  dtn_item *item = dtn_item_object();
  dtn_item *obj = dtn_item_object();

  dtn_item *val = dtn_item_number(1234);

  testrun(dtn_item_object_set(item, "1", val));
  testrun(dtn_item_object_set(item, "2", obj));
  testrun(2 == dtn_item_count(item));

  testrun(obj == dtn_item_object_get(item, "2"));
  testrun(2 == dtn_item_count(item));

  testrun(val == dtn_item_object_get(item, "1"));
  testrun(2 == dtn_item_count(item));

  testrun(NULL == dtn_item_free(item));

  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

static bool clear_subitem(const char *key, dtn_item const *val, void *userdata) {

  if (!key)
    return true;
  dtn_item_clear((dtn_item *)val);
  UNUSED(userdata);
  return true;
}

/*----------------------------------------------------------------------------*/

int test_dtn_item_object_for_each() {

  dtn_item *item = dtn_item_object();
  dtn_item *obj = dtn_item_object();
  dtn_item *val = dtn_item_number(1234);

  testrun(dtn_item_object_set(item, "1", val));
  testrun(dtn_item_object_set(item, "2", obj));

  testrun(2 == dtn_item_count(item));

  testrun(dtn_item_object_for_each(item, clear_subitem, item));

  testrun(2 == dtn_item_count(item));

  testrun(obj == dtn_item_object_get(item, "2"));
  testrun(2 == dtn_item_count(item));
  testrun(dtn_item_is_null(obj));

  testrun(val == dtn_item_object_get(item, "1"));
  testrun(2 == dtn_item_count(item));
  testrun(dtn_item_is_null(val));

  testrun(NULL == dtn_item_free(item));

  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_item_array_get() {

  dtn_item *item = dtn_item_array();
  dtn_item *obj = dtn_item_object();
  dtn_item *val = dtn_item_number(1234);

  testrun(dtn_item_array_push(item, obj));
  testrun(dtn_item_array_push(item, val));

  testrun(obj == dtn_item_array_get(item, 1));
  testrun(val == dtn_item_array_get(item, 2));
  testrun(NULL == dtn_item_array_get(item, 3));

  testrun(NULL == dtn_item_free(item));

  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_item_array_set() {

  dtn_item *item = dtn_item_array();
  dtn_item *obj = dtn_item_object();
  dtn_item *val = dtn_item_number(1234);

  testrun(dtn_item_array_push(item, obj));
  testrun(dtn_item_array_push(item, val));

  testrun(dtn_item_array_set(item, 1, dtn_item_null()));
  testrun(dtn_item_is_null(dtn_item_array_get(item, 1)));
  testrun(val == dtn_item_array_get(item, 2));
  testrun(NULL == dtn_item_array_get(item, 3));
  testrun(NULL == dtn_item_array_get(item, 4));

  testrun(dtn_item_array_set(item, 3, dtn_item_null()));
  testrun(dtn_item_is_null(dtn_item_array_get(item, 1)));
  testrun(val == dtn_item_array_get(item, 2));
  testrun(dtn_item_is_null(dtn_item_array_get(item, 3)));
  testrun(NULL == dtn_item_array_get(item, 4));
  testrun(NULL == dtn_item_array_get(item, 5));
  testrun(NULL == dtn_item_array_get(item, 6));
  testrun(NULL == dtn_item_array_get(item, 7));
  testrun(dtn_item_array_set(item, 6, dtn_item_true()));
  testrun(dtn_item_is_null(dtn_item_array_get(item, 1)));
  testrun(val == dtn_item_array_get(item, 2));
  testrun(dtn_item_is_null(dtn_item_array_get(item, 3)));
  testrun(NULL == dtn_item_array_get(item, 4));
  testrun(NULL == dtn_item_array_get(item, 5));
  testrun(dtn_item_is_true(dtn_item_array_get(item, 6)));
  testrun(NULL == dtn_item_array_get(item, 7));

  testrun(NULL == dtn_item_free(item));

  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_item_array_push() {

  dtn_item *item = dtn_item_array();
  dtn_item *obj = dtn_item_object();
  dtn_item *val = dtn_item_number(1234);

  testrun(dtn_item_array_push(item, obj));
  testrun(dtn_item_array_push(item, val));

  testrun(obj == dtn_item_array_get(item, 1));
  testrun(val == dtn_item_array_get(item, 2));
  testrun(NULL == dtn_item_array_get(item, 3));

  testrun(NULL == dtn_item_free(item));

  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_item_array_stack_pop() {

  dtn_item *item = dtn_item_array();
  dtn_item *obj = dtn_item_object();
  dtn_item *val = dtn_item_number(1234);

  testrun(dtn_item_array_push(item, obj));
  testrun(dtn_item_array_push(item, val));

  testrun(val == dtn_item_array_stack_pop(item));
  testrun(obj == dtn_item_array_stack_pop(item));
  testrun(NULL == dtn_item_array_stack_pop(item));

  testrun(NULL == dtn_item_free(item));
  testrun(NULL == dtn_item_free(val));
  testrun(NULL == dtn_item_free(obj));

  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_item_array_queue_pop() {

  dtn_item *item = dtn_item_array();
  dtn_item *obj = dtn_item_object();
  dtn_item *val = dtn_item_number(1234);

  testrun(dtn_item_array_push(item, obj));
  testrun(dtn_item_array_push(item, val));

  testrun(obj == dtn_item_array_queue_pop(item));
  testrun(val == dtn_item_array_queue_pop(item));
  testrun(NULL == dtn_item_array_queue_pop(item));

  testrun(NULL == dtn_item_free(item));
  testrun(NULL == dtn_item_free(val));
  testrun(NULL == dtn_item_free(obj));

  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_item_get_string() {

  dtn_item *item = dtn_item_string("test");
  testrun(0 == dtn_string_compare("test", dtn_item_get_string(item)));
  testrun(NULL == dtn_item_free(item));

  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_item_get_number() {

  dtn_item *item = dtn_item_number(1.001);
  testrun(1.001 == dtn_item_get_number(item));
  testrun(NULL == dtn_item_free(item));

  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_item_get_int() {

  dtn_item *item = dtn_item_number(1.001);
  testrun(1 == dtn_item_get_int(item));
  testrun(NULL == dtn_item_free(item));

  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int check_pointer_parse_token() {

  uint64_t size = 100;
  char buffer[size];
  memset(buffer, '\0', size);

  memcpy(buffer, "/abc", 4);

  char *token = NULL;
  size_t length = 0;

  testrun(!pointer_parse_token(NULL, 0, NULL, NULL));
  testrun(!pointer_parse_token(NULL, size, &token, &length));
  testrun(!pointer_parse_token(buffer, 0, &token, &length));
  testrun(!pointer_parse_token(buffer, size, NULL, &length));
  testrun(!pointer_parse_token(buffer, size, &token, NULL));
  testrun(pointer_parse_token(buffer, 1, &token, &length));

  // empty token
  testrun(pointer_parse_token(buffer, 1, &token, &length));
  testrun(length == 0);
  testrun(token[0] == '/');

  testrun(pointer_parse_token(buffer, 2, &token, &length));
  testrun(length == 1);
  testrun(token[0] == 'a');
  testrun(token[length - 1] == 'a');

  testrun(pointer_parse_token(buffer, 3, &token, &length));
  testrun(length == 2);
  testrun(token[0] == 'a');
  testrun(token[length - 1] == 'b');

  testrun(pointer_parse_token(buffer, 4, &token, &length));
  testrun(length == 3);
  testrun(token[0] == 'a');
  testrun(token[length - 1] == 'c');

  testrun(pointer_parse_token(buffer, 5, &token, &length));
  testrun(length == 3);
  testrun(token[0] == 'a');
  testrun(token[length - 1] == 'c');

  testrun(pointer_parse_token(buffer, 100, &token, &length));
  testrun(length == 3);
  testrun(token[0] == 'a');
  testrun(token[length - 1] == 'c');

  //              0123456789
  memcpy(buffer, "/123/45/6/", 10);

  testrun(pointer_parse_token(buffer, 2, &token, &length));
  testrun(length == 1);
  testrun(token[0] == '1');
  testrun(token[length - 1] == '1');

  testrun(pointer_parse_token(buffer, 3, &token, &length));
  testrun(length == 2);
  testrun(token[0] == '1');
  testrun(token[length - 1] == '2');

  testrun(pointer_parse_token(buffer, 4, &token, &length));
  testrun(length == 3);
  testrun(token[0] == '1');
  testrun(token[length - 1] == '3');

  testrun(pointer_parse_token(buffer, 5, &token, &length));
  testrun(length == 3);
  testrun(token[0] == '1');
  testrun(token[length - 1] == '3');

  testrun(pointer_parse_token(buffer, 100, &token, &length));
  testrun(length == 3);
  testrun(token[0] == '1');
  testrun(token[length - 1] == '3');

  // not starting with '/'
  testrun(!pointer_parse_token(buffer + 1, 2, &token, &length));
  testrun(!pointer_parse_token(buffer + 2, 2, &token, &length));
  testrun(!pointer_parse_token(buffer + 3, 2, &token, &length));

  // check second pointer
  testrun(pointer_parse_token(buffer + 4, 2, &token, &length));
  testrun(length == 1);
  testrun(token[0] == '4');
  testrun(token[length - 1] == '4');
  testrun(pointer_parse_token(buffer + 4, 3, &token, &length));
  testrun(length == 2);
  testrun(token[0] == '4');
  testrun(token[length - 1] == '5');
  testrun(pointer_parse_token(buffer + 4, 1000, &token, &length));
  testrun(length == 2);
  testrun(token[0] == '4');
  testrun(token[length - 1] == '5');

  // check third pointer
  testrun(!pointer_parse_token(buffer + 5, 2, &token, &length));
  testrun(!pointer_parse_token(buffer + 6, 2, &token, &length));
  testrun(pointer_parse_token(buffer + 7, 2, &token, &length));
  testrun(length == 1);
  testrun(token[0] == '6');
  testrun(token[length - 1] == '6');
  testrun(pointer_parse_token(buffer + 7, 3, &token, &length));
  testrun(length == 1);
  testrun(token[0] == '6');
  testrun(token[length - 1] == '6');
  testrun(pointer_parse_token(buffer + 7, 1000, &token, &length));
  testrun(length == 1);
  testrun(token[0] == '6');
  testrun(token[length - 1] == '6');

  // check empty followed by null pointer
  testrun(!pointer_parse_token(buffer + 10, 2, &token, &length));
  testrun(!pointer_parse_token(buffer + 11, 2, &token, &length));
  testrun(!pointer_parse_token(buffer + 12, 2, &token, &length));

  //              0123456789
  memcpy(buffer, "/1~0/45/6/", 10);
  testrun(pointer_parse_token(buffer, 2, &token, &length));
  testrun(length == 1);
  testrun(token[0] == '1');
  testrun(token[length - 1] == '1');

  testrun(pointer_parse_token(buffer, 3, &token, &length));
  testrun(length == 2);
  testrun(token[0] == '1');
  testrun(token[length - 1] == '~');

  testrun(pointer_parse_token(buffer, 4, &token, &length));
  testrun(length == 3);
  testrun(token[0] == '1');
  testrun(token[length - 1] == '0');

  testrun(pointer_parse_token(buffer, 5, &token, &length));
  testrun(length == 3);
  testrun(token[0] == '1');
  testrun(token[length - 1] == '0');

  testrun(pointer_parse_token(buffer, 100, &token, &length));
  testrun(length == 3);
  testrun(token[0] == '1');
  testrun(token[length - 1] == '0');

  //              0123456789
  memcpy(buffer, "/1~1/45/6/", 10);
  testrun(pointer_parse_token(buffer, 2, &token, &length));
  testrun(length == 1);
  testrun(token[0] == '1');
  testrun(token[length - 1] == '1');

  testrun(pointer_parse_token(buffer, 3, &token, &length));
  testrun(length == 2);
  testrun(token[0] == '1');
  testrun(token[length - 1] == '~');

  testrun(pointer_parse_token(buffer, 4, &token, &length));
  testrun(length == 3);
  testrun(token[0] == '1');
  testrun(token[length - 1] == '1');

  testrun(pointer_parse_token(buffer, 5, &token, &length));
  testrun(length == 3);
  testrun(token[0] == '1');
  testrun(token[length - 1] == '1');

  testrun(pointer_parse_token(buffer, 100, &token, &length));
  testrun(length == 3);
  testrun(token[0] == '1');
  testrun(token[length - 1] == '1');

  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int check_pointer_replace_special_encoding() {

  uint64_t size = 100;
  char buffer1[size];
  char buffer2[size];
  memset(buffer1, '\0', size);
  memset(buffer2, '\0', size);

  memcpy(buffer1, "123abcdefg", 10);

  char *result = buffer2;

  testrun(!pointer_replace_special_encoding(NULL, size, buffer1, "abc", "xyz"));

  testrun(!pointer_replace_special_encoding(result, size, NULL, "abc", "xyz"));

  testrun(
      !pointer_replace_special_encoding(result, size, buffer1, NULL, "xyz"));

  testrun(
      !pointer_replace_special_encoding(result, size, buffer1, "abc", NULL));

  testrun(!pointer_replace_special_encoding(result, 0, buffer1, "abc", "xyz"));

  testrun(strlen(buffer2) == 0);

  // replace with same length
  testrun(
      pointer_replace_special_encoding(result, size, buffer1, "abc", "xyz"));

  testrun(strlen(buffer2) == strlen(buffer1));
  testrun(strncmp("123xyzdefg", buffer2, strlen(buffer2)) == 0);
  memset(buffer2, '\0', size);

  // replace with shorter length
  memset(buffer2, '\0', size);
  testrun(pointer_replace_special_encoding(result, size, buffer1, "abc", "x"));

  testrun(strlen(buffer2) == strlen(buffer1) - 2);
  testrun(strncmp("123xdefg", buffer2, strlen(buffer2)) == 0);

  // replace first
  memset(buffer2, '\0', size);
  testrun(pointer_replace_special_encoding(result, size, buffer1, "1", "x"));

  testrun(strlen(buffer2) == strlen(buffer1));
  testrun(strncmp("x23abcdefg", buffer2, strlen(buffer2)) == 0);

  // replace last
  memset(buffer2, '\0', size);
  testrun(pointer_replace_special_encoding(result, size, buffer1, "g", "x"));

  testrun(strlen(buffer2) == strlen(buffer1));
  testrun(strncmp("123abcdefx", buffer2, strlen(buffer2)) == 0);

  // replace (not included)
  memset(buffer2, '\0', size);
  testrun(pointer_replace_special_encoding(result, size, buffer1, "vvv", "x"));

  testrun(strlen(buffer2) == strlen(buffer1));
  testrun(strncmp("123abcdefg", buffer2, strlen(buffer2)) == 0);

  // replace all
  memset(buffer2, '\0', size);
  testrun(pointer_replace_special_encoding(result, size, buffer1, "123abcdefg",
                                           "x"));

  testrun(strlen(buffer2) == 1);
  testrun(strncmp("x", buffer2, strlen(buffer2)) == 0);

  // replace up to length
  memset(buffer2, '\0', size);
  testrun(pointer_replace_special_encoding(result, 9, buffer1, "abc", "xyz"));
  testrun(strlen(buffer2) == 9);
  testrun(strncmp("123xyzdef", buffer2, strlen(buffer2)) == 0);

  memset(buffer2, '\0', size);
  testrun(pointer_replace_special_encoding(result, 8, buffer1, "abc", "xyz"));
  testrun(strlen(buffer2) == 8);
  testrun(strncmp("123xyzde", buffer2, strlen(buffer2)) == 0);

  memset(buffer2, '\0', size);
  testrun(pointer_replace_special_encoding(result, 7, buffer1, "abc", "xyz"));
  testrun(strlen(buffer2) == 7);
  testrun(strncmp("123xyzd", buffer2, strlen(buffer2)) == 0);

  memset(buffer2, '\0', size);
  testrun(pointer_replace_special_encoding(result, 6, buffer1, "abc", "xyz"));
  testrun(strlen(buffer2) == 6);
  testrun(strncmp("123xyz", buffer2, strlen(buffer2)) == 0);

  memset(buffer2, '\0', size);
  testrun(!pointer_replace_special_encoding(result, 5, buffer1, "abc", "xyz"));
  testrun(strlen(buffer2) == 0);

  memcpy(buffer1, "/123/abc/x", 10);

  memset(buffer2, '\0', size);
  testrun(pointer_replace_special_encoding(result, 10, buffer1, "~1", "/"));
  testrun(strlen(buffer2) == 10);
  testrun(strncmp("/123/abc/x", buffer2, strlen(buffer2)) == 0);

  memset(buffer2, '\0', size);
  testrun(pointer_replace_special_encoding(result, 4, buffer1, "~1", "/"));
  testrun(strlen(buffer2) == 4);
  testrun(strncmp("/123", buffer2, strlen(buffer2)) == 0);

  memcpy(buffer1, "12~1abc", 7);
  memset(buffer2, '\0', size);
  testrun(pointer_replace_special_encoding(result, 7, buffer1, "~1", "/"));
  testrun(strlen(buffer2) == 6);
  testrun(strncmp("12/abc", buffer2, strlen(buffer2)) == 0);

  memcpy(buffer1, "12~0abc", 7);
  memset(buffer2, '\0', size);
  testrun(pointer_replace_special_encoding(result, 7, buffer1, "~0", "~"));
  testrun(strlen(buffer2) == 6);
  testrun(strncmp("12~abc", buffer2, strlen(buffer2)) == 0);

  memcpy(buffer1, "12~3abc", 7);
  memset(buffer2, '\0', size);
  testrun(pointer_replace_special_encoding(result, 7, buffer1, "~0", "~"));
  testrun(strlen(buffer2) == 7);
  testrun(strncmp("12~3abc", buffer2, strlen(buffer2)) == 0);

  memcpy(buffer1, "12~~0ab", 7);
  memset(buffer2, '\0', size);
  testrun(pointer_replace_special_encoding(result, 7, buffer1, "~0", "~"));
  testrun(strlen(buffer2) == 6);
  testrun(strncmp("12~~ab", buffer2, strlen(buffer2)) == 0);

  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int check_pointer_escape_token() {

  uint64_t size = 100;
  char buffer1[size];
  char buffer2[size];
  memset(buffer1, '\0', size);
  memset(buffer2, '\0', size);

  memcpy(buffer1, "123abcdefg", 10);

  char *result = buffer2;

  testrun(!pointer_escape_token(NULL, size, result, size));

  testrun(!pointer_escape_token(buffer1, 0, result, size));

  testrun(!pointer_escape_token(buffer1, size, NULL, size));

  testrun(!pointer_escape_token(buffer1, size, result, size - 1));

  // tokens not contained
  testrun(pointer_escape_token(buffer1, size, result, size));

  // log("buffer1 |%s|\nbuffer2 |%s|", buffer1, buffer2);
  testrun(strlen(buffer2) == strlen(buffer1));
  testrun(strncmp("123abcdefg", buffer2, strlen(buffer2)) == 0);

  // ~0 contained
  memcpy(buffer1, "123~0cdefg", 10);
  memset(buffer2, '\0', size);

  testrun(pointer_escape_token(buffer1, 10, result, size));
  testrun(strlen(buffer2) == strlen(buffer1) - 1);
  testrun(strncmp("123~cdefg", buffer2, strlen(buffer2)) == 0);

  // ~1 contained
  memcpy(buffer1, "123~1cdefg", 10);
  memset(buffer2, '\0', size);

  testrun(pointer_escape_token(buffer1, size, result, size));
  testrun(strlen(buffer2) == strlen(buffer1) - 1);
  testrun(strncmp("123\\cdefg", buffer2, strlen(buffer2)) == 0);

  // replacement order
  memcpy(buffer1, "123~01defg", 10);
  memset(buffer2, '\0', size);
  testrun(pointer_escape_token(buffer1, size, result, size));
  testrun(strlen(buffer2) == strlen(buffer1) - 1);
  testrun(strncmp("123~1defg", buffer2, strlen(buffer2)) == 0);

  // multiple replacements
  memcpy(buffer1, "~0~0~01d~0g", 11);
  memset(buffer2, '\0', size);
  testrun(pointer_escape_token(buffer1, 11, result, size));
  testrun(strlen(buffer2) == strlen(buffer1) - 4);
  testrun(strncmp("~~~1d~g", buffer2, strlen(buffer2)) == 0);

  // multiple replacements
  memcpy(buffer1, "~0~1~01d~1g", 11);
  memset(buffer2, '\0', size);
  testrun(pointer_escape_token(buffer1, size, result, size));
  testrun(strlen(buffer2) == strlen(buffer1) - 4);
  testrun(strncmp("~\\~1d\\g", buffer2, strlen(buffer2)) == 0);

  // multiple replacements
  memcpy(buffer1, "array/5", 7);
  memset(buffer2, '\0', size);
  testrun(pointer_escape_token(buffer1, 5, result, size));
  testrun(strlen(buffer2) == 5);
  testrun(strncmp("array", buffer2, strlen(buffer2)) == 0);

  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int check_pointer_get_token_in_parent() {

  dtn_item *v1 = NULL;
  dtn_item *v2 = NULL;
  dtn_item *v3 = NULL;
  dtn_item *v4 = NULL;
  dtn_item *v0 = NULL;
  dtn_item *v5 = NULL;
  dtn_item *v6 = NULL;
  dtn_item *v7 = NULL;
  dtn_item *v8 = NULL;
  dtn_item *v9 = NULL;

  dtn_item *result = NULL;
  dtn_item *val = NULL;
  dtn_item *value = dtn_item_object();

  v1 = dtn_item_true();
  dtn_item_object_set(value, "key", v1);

  uint64_t size = 100;
  char buffer1[size];
  char buffer2[size];
  memset(buffer1, '\0', size);
  memset(buffer2, '\0', size);

  strcpy(buffer1, "key");

  testrun(!pointer_get_token_in_parent(NULL, NULL, 0));
  testrun(!pointer_get_token_in_parent(value, NULL, 3));

  // empty key is value
  testrun(value == pointer_get_token_in_parent(value, "key", 0));

  val = dtn_item_object_get(value, "key");
  result = pointer_get_token_in_parent(value, "key", 3);
  testrun(result);
  testrun(result->parent == value);
  testrun(result == val);

  // test empty object
  value = dtn_item_free(value);
  value = dtn_item_object();
  testrun(!pointer_get_token_in_parent(value, "key", 3));
  dtn_item_object_set(value, "key", dtn_item_true());
  val = dtn_item_object_get(value, "key");
  result = pointer_get_token_in_parent(value, "key", 3);
  testrun(result);
  testrun(result->parent == value);
  testrun(result == val);

  // check key match
  testrun(!pointer_get_token_in_parent(value, "key", 2));
  testrun(!pointer_get_token_in_parent(value, "Key ", 3));
  testrun(!pointer_get_token_in_parent(value, "key ", 4));

  // add some members
  v1 = dtn_item_true();
  testrun(dtn_item_object_set(value, "1", v1));
  v2 = dtn_item_true();
  testrun(dtn_item_object_set(value, "2", v2));
  v3 = dtn_item_true();
  testrun(dtn_item_object_set(value, "3", v3));
  v4 = dtn_item_true();
  testrun(dtn_item_object_set(value, "4", v4));

  result = pointer_get_token_in_parent(value, "1", 1);
  testrun(result);
  testrun(result->parent == value);
  testrun(result == v1);

  result = pointer_get_token_in_parent(value, "2", 1);
  testrun(result);
  testrun(result->parent == value);
  testrun(result == v2);

  result = pointer_get_token_in_parent(value, "3", 1);
  testrun(result);
  testrun(result->parent == value);
  testrun(result == v3);

  result = pointer_get_token_in_parent(value, "4", 1);
  testrun(result);
  testrun(result->parent == value);
  testrun(result == v4);
  value = dtn_item_free(value);

  value = dtn_item_array();
  testrun(0 == dtn_item_count(value));
  testrun(!pointer_get_token_in_parent(value, "1", 1));
  testrun(!pointer_get_token_in_parent(value, "2", 1));
  testrun(!pointer_get_token_in_parent(value, "3", 1));
  v0 = pointer_get_token_in_parent(value, "-", 1);
  testrun(1 == dtn_item_count(value));

  v1 = dtn_item_true();
  testrun(dtn_item_array_push(value, v1)) v2 = dtn_item_true();
  testrun(dtn_item_array_push(value, v2)) v3 = dtn_item_true();
  testrun(dtn_item_array_push(value, v3)) v4 = dtn_item_true();
  testrun(dtn_item_array_push(value, v4));

  result = pointer_get_token_in_parent(value, "0", 1);
  testrun(result == v0);
  result = pointer_get_token_in_parent(value, "1", 1);
  testrun(result == v1);
  result = pointer_get_token_in_parent(value, "2", 1);
  testrun(result == v2);
  result = pointer_get_token_in_parent(value, "3", 1);
  testrun(result == v3);
  result = pointer_get_token_in_parent(value, "4", 1);
  testrun(result == v4);
  testrun(5 == dtn_item_count(value));

  // add a new array item
  result = pointer_get_token_in_parent(value, "-", 1);
  testrun(6 == dtn_item_count(value));
  testrun(result->parent == value);

  // get with leading zero
  result = pointer_get_token_in_parent(value, "00", 2);
  testrun(result == v0);
  result = pointer_get_token_in_parent(value, "01", 2);
  testrun(result == v1);
  result = pointer_get_token_in_parent(value, "02", 2);
  testrun(result == v2);
  result = pointer_get_token_in_parent(value, "03", 2);
  testrun(result == v3);
  result = pointer_get_token_in_parent(value, "04", 2);
  testrun(result == v4);

  // not array or object
  testrun(!pointer_get_token_in_parent(v1, "3", 1));
  value = dtn_item_free(value);

  value = dtn_item_object();
  testrun(!pointer_get_token_in_parent(value, "foo", 3));
  testrun(dtn_item_object_set(value, "foo", dtn_item_true()));
  result = pointer_get_token_in_parent(value, "foo", 3);
  testrun(result);
  testrun(result->parent == value);
  testrun(result == dtn_item_object_get(value, "foo"));

  v1 = dtn_item_true();
  v2 = dtn_item_true();
  v3 = dtn_item_true();
  v4 = dtn_item_true();
  v5 = dtn_item_true();
  v6 = dtn_item_true();
  v7 = dtn_item_true();
  v8 = dtn_item_true();
  v9 = dtn_item_true();
  testrun(dtn_item_object_set(value, "0", v1));
  testrun(dtn_item_object_set(value, "a/b", v2));
  testrun(dtn_item_object_set(value, "c%d", v3));
  testrun(dtn_item_object_set(value, "e^f", v4));
  testrun(dtn_item_object_set(value, "g|h", v5));
  testrun(dtn_item_object_set(value, "i\\\\j", v6));
  testrun(dtn_item_object_set(value, "k\"l", v7));
  testrun(dtn_item_object_set(value, " ", v8));
  testrun(dtn_item_object_set(value, "m~n", v9));

  result = pointer_get_token_in_parent(value, "", 0);
  testrun(result == value);
  result = pointer_get_token_in_parent(value, "0", 1);
  testrun(result == v1);
  result = pointer_get_token_in_parent(value, "a/b", 3);
  testrun(result == v2);
  result = pointer_get_token_in_parent(value, "c%d", 3);
  testrun(result == v3);
  result = pointer_get_token_in_parent(value, "e^f", 3);
  testrun(result == v4);
  result = pointer_get_token_in_parent(value, "g|h", 3);
  testrun(result == v5);
  result = pointer_get_token_in_parent(value, "i\\\\j", 4);
  testrun(result == v6);
  result = pointer_get_token_in_parent(value, "k\"l", 3);
  testrun(result == v7);
  result = pointer_get_token_in_parent(value, " ", 1);
  testrun(result == v8);
  result = pointer_get_token_in_parent(value, "m~n", 3);
  testrun(result == v9);

  value = dtn_item_free(value);

  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_item_get() {

  dtn_item *array = NULL;
  dtn_item *object = NULL;
  dtn_item *v0 = NULL;
  dtn_item *v1 = NULL;
  dtn_item *v2 = NULL;
  dtn_item *v3 = NULL;
  dtn_item *v4 = NULL;
  dtn_item *v5 = NULL;

  dtn_item const *result = NULL;
  dtn_item *value = dtn_item_object();
  v0 = dtn_item_true();
  dtn_item_object_set(value, "key", v0);

  uint64_t size = 100;
  char buffer1[size];
  memset(buffer1, '\0', size);

  strcpy(buffer1, "key");

  testrun(!dtn_item_get(NULL, NULL));
  testrun(value == dtn_item_get(value, NULL));
  testrun(!dtn_item_get(NULL, "key"));

  // empty key is value
  testrun(value == dtn_item_get(value, ""));
  testrun(!dtn_item_get(value, "/"));
  testrun(!dtn_item_get(value, "/k"));
  testrun(!dtn_item_get(value, "/ke"));
  testrun(v0 == dtn_item_get(value, "/key"));

  // key not contained
  testrun(!dtn_item_get(value, "1"));

  // add some childs
  array = dtn_item_array();
  dtn_item_object_set(value, "array", array);
  object = dtn_item_object();
  dtn_item_object_set(value, "object", object);
  v1 = dtn_item_true();
  dtn_item_object_set(value, "1", v1);
  v2 = dtn_item_false();
  dtn_item_object_set(value, "2", v2);

  // add some child to the array
  v3 = dtn_item_true();
  v4 = dtn_item_true();
  v5 = dtn_item_object();
  dtn_item_array_push(array, v3);
  dtn_item_array_push(array, v4);
  dtn_item_array_push(array, v5);

  // add child to object in array
  dtn_item_object_set(v5, "5", dtn_item_array());
  dtn_item_object_set(v5, "key6", dtn_item_object());
  dtn_item_object_set(v5, "key7", dtn_item_object());

  // add child to object
  dtn_item_object_set(object, "8", dtn_item_object());

  // positive testing

  testrun(value == dtn_item_get(value, ""));
  testrun(v0 == dtn_item_get(value, "/key"));
  testrun(array == dtn_item_get(value, "/array"));
  testrun(object == dtn_item_get(value, "/object"));

  testrun(dtn_item_get(value, "/key"));
  testrun(dtn_item_get(value, "/array"));
  testrun(dtn_item_get(value, "/object"));
  testrun(!dtn_item_get(value, "/key "));
  testrun(!dtn_item_get(value, "/array "));
  testrun(!dtn_item_get(value, "/object "));

  testrun(v3 == dtn_item_get(value, "/array/0"));
  testrun(v4 == dtn_item_get(value, "/array/1"));
  testrun(v5 == dtn_item_get(value, "/array/2"));

  testrun(dtn_item_get(value, "/array/2/5"));
  testrun(dtn_item_get(value, "/array/2/key6"));
  testrun(dtn_item_get(value, "/array/2/key7"));

  testrun(object == dtn_item_get(value, "/object"));
  testrun(dtn_item_get(value, "/object/8"));

  testrun(v1 == dtn_item_get(value, "/1"));
  testrun(v2 == dtn_item_get(value, "/2"));

  // add new array member
  testrun(3 == dtn_item_count(array));
  result = dtn_item_get(value, "/array/-");
  testrun(result);
  testrun(4 == dtn_item_count(array));

  // try to add new object member
  testrun(!dtn_item_get(value, "/-"));

  // key not contained
  testrun(!dtn_item_get(value, "/objectX"));
  testrun(!dtn_item_get(value, "/obj"));
  testrun(!dtn_item_get(value, "/array/2/key1"));
  testrun(!dtn_item_get(value, "/array/2/k"));

  // out of index
  testrun(4 == dtn_item_count(array));
  testrun(dtn_item_get(value, "/array/0"));
  testrun(dtn_item_get(value, "/array/1"));
  testrun(dtn_item_get(value, "/array/2"));
  testrun(dtn_item_get(value, "/array/3"));
  testrun(!dtn_item_get(value, "/array/4"));
  testrun(!dtn_item_get(value, "/array/5"));
  testrun(!dtn_item_get(value, "/array/6"));
  testrun(!dtn_item_get(value, "/array/7"));

  // not an index
  testrun(array == dtn_item_get(value, "/array/"));
  testrun(array == dtn_item_get(value, "/array/"));
  testrun(array == dtn_item_get(value, "/array/"));

  // not an object
  testrun(object == dtn_item_get(value, "/object"));
  testrun(object == dtn_item_get(value, "/object/"));
  testrun(!dtn_item_get(value, "/object/ "));
  testrun(!dtn_item_get(value, "/objec "));

  // no value to return
  testrun(!dtn_item_get(value, "/"));

  // no starting pointer slash
  testrun(!dtn_item_get(value, "object"));
  testrun(!dtn_item_get(value, "object/"));

  dtn_item_free(value);

  return testrun_log_success();
}

/*
 *      ------------------------------------------------------------------------
 *
 *      TEST CLUSTER                                                    #CLUSTER
 *
 *      ------------------------------------------------------------------------
 */

int all_tests() {

  testrun_init();
  testrun_test(test_dtn_item_cast);

  testrun_test(test_dtn_item_object);
  testrun_test(test_dtn_item_array);
  testrun_test(test_dtn_item_string);
  testrun_test(test_dtn_item_number);
  testrun_test(test_dtn_item_true);
  testrun_test(test_dtn_item_false);
  testrun_test(test_dtn_item_null);

  testrun_test(test_dtn_item_free);
  testrun_test(test_dtn_item_clear);
  testrun_test(test_dtn_item_copy);
  testrun_test(test_dtn_item_dump);

  testrun_test(test_dtn_item_count);

  testrun_test(test_dtn_item_is_object);
  testrun_test(test_dtn_item_is_array);
  testrun_test(test_dtn_item_is_string);
  testrun_test(test_dtn_item_is_number);
  testrun_test(test_dtn_item_is_null);
  testrun_test(test_dtn_item_is_true);
  testrun_test(test_dtn_item_is_false);

  testrun_test(test_dtn_item_object_set);
  testrun_test(test_dtn_item_object_delete);
  testrun_test(test_dtn_item_object_remove);
  testrun_test(test_dtn_item_object_get);
  testrun_test(test_dtn_item_object_for_each);

  testrun_test(test_dtn_item_array_get);
  testrun_test(test_dtn_item_array_set);
  testrun_test(test_dtn_item_array_push);
  testrun_test(test_dtn_item_array_stack_pop);
  testrun_test(test_dtn_item_array_queue_pop);

  testrun_test(test_dtn_item_get_string);
  testrun_test(test_dtn_item_get_number);
  testrun_test(test_dtn_item_get_int);

  testrun_test(check_pointer_parse_token);
  testrun_test(check_pointer_replace_special_encoding);
  testrun_test(check_pointer_escape_token);
  testrun_test(check_pointer_get_token_in_parent);

  testrun_test(test_dtn_item_get);

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
