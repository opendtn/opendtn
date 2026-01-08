/***
  ------------------------------------------------------------------------

  Copyright 2018 German Aerospace Center DLR e.V. (GSOC)

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
    @file           dtn_list_test.c
    @author         Markus Toepfer
    @author         Michael Beer

    @date           2018-07-02

    @ingroup        dtn_data_structures

    @brief


    ------------------------------------------------------------------------
    */

#include "../include/dtn_list_test_interface.h"
#include "../include/testrun.h"
#include "dtn_list.c"

/*----------------------------------------------------------------------------*/

int test_dtn_list_cast() {

    dtn_list *list = dtn_list_create((dtn_list_config){0});
    testrun(list);
    testrun(dtn_list_cast(list));
    testrun(list->magic_byte == MAGIC_BYTE);

    uint16_t type = list->type;

    for (int i = 0; i <= 0xFFFF; i++) {

        list->magic_byte = i;

        if (i != MAGIC_BYTE) {
            testrun(!dtn_list_cast(list));
        } else {
            testrun(dtn_list_cast(list));
        }

        list->magic_byte = MAGIC_BYTE;
        list->type = i;
        testrun(dtn_list_cast(list));
    }

    // reset to free all content
    list->magic_byte = MAGIC_BYTE;
    list->type = type;

    testrun(NULL == dtn_list_free(list));
    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_list_create() {

    dtn_list *list = dtn_list_create((dtn_list_config){0});

    testrun(list);
    testrun(dtn_list_cast(list));

    testrun(list->clear != NULL);
    testrun(list->free != NULL);
    testrun(list->copy != NULL);
    testrun(list->get != NULL);
    testrun(list->set != NULL);
    testrun(list->insert != NULL);
    testrun(list->remove != NULL);
    testrun(list->push != NULL);
    testrun(list->pop != NULL);
    testrun(list->count != NULL);
    testrun(list->for_each != NULL);

    testrun(list->config.item.free == NULL);
    testrun(list->config.item.clear == NULL);
    testrun(list->config.item.copy == NULL);
    testrun(list->config.item.dump == NULL);

    list = list->free(list);
    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_list_clear() {

    dtn_list *list = dtn_list_create((dtn_list_config){0});

    char *str1 = strdup("content1");
    char *str2 = strdup("content2");
    char *str3 = strdup("content3");

    testrun(!dtn_list_clear(NULL));
    testrun(0 == list->count(list));
    testrun(dtn_list_clear(list));
    testrun(0 == list->count(list));

    // not a list
    list->magic_byte = MAGIC_BYTE + 1;
    testrun(!dtn_list_clear(list));
    list->magic_byte = MAGIC_BYTE;
    testrun(dtn_list_clear(list));

    // with items
    list->push(list, str1);
    list->push(list, str2);
    list->push(list, str3);
    testrun(3 == list->count(list));
    testrun(dtn_list_clear(list));
    testrun(0 == list->count(list));

    // items untouched
    testrun(strcmp(str1, "content1") == 0);
    testrun(strcmp(str2, "content2") == 0);
    testrun(strcmp(str3, "content3") == 0);

    // clear with content free
    list->config.item = dtn_data_string_data_functions();
    testrun(list->config.item.free == dtn_data_string_free);
    list->push(list, str1);
    list->push(list, str2);
    list->push(list, str3);
    testrun(3 == list->count(list));
    testrun(dtn_list_clear(list));
    testrun(0 == list->count(list));

    // check valgrind run for strdup free

    list = list->free(list);
    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_list_free() {

    dtn_list *list = dtn_list_create((dtn_list_config){0});

    char *str1 = strdup("content1");
    char *str2 = strdup("content2");
    char *str3 = strdup("content3");

    testrun(!dtn_list_free(NULL));
    testrun(0 == list->count(list));
    testrun(NULL == dtn_list_free(list));

    list = dtn_list_create((dtn_list_config){0});
    // not a list
    list->magic_byte = MAGIC_BYTE + 1;
    testrun(list == dtn_list_free(list));
    list->magic_byte = MAGIC_BYTE;
    testrun(NULL == dtn_list_free(list));

    // with items
    list = dtn_list_create((dtn_list_config){0});
    list->push(list, str1);
    list->push(list, str2);
    list->push(list, str3);
    testrun(3 == list->count(list));
    testrun(NULL == dtn_list_free(list));

    // items untouched
    testrun(strcmp(str1, "content1") == 0);
    testrun(strcmp(str2, "content2") == 0);
    testrun(strcmp(str3, "content3") == 0);

    // clear with content free
    list = dtn_list_create((dtn_list_config){0});
    list->config.item = dtn_data_string_data_functions();
    testrun(list->config.item.free == dtn_data_string_free);
    list->push(list, str1);
    list->push(list, str2);
    list->push(list, str3);
    testrun(3 == list->count(list));
    testrun(NULL == dtn_list_free(list));

    // check valgrind run for strdup free

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_list_copy() {

    dtn_list *list = dtn_list_create((dtn_list_config){0});
    dtn_list *copy = NULL;

    char *str1 = strdup("content1");
    char *str2 = strdup("content2");
    char *str3 = strdup("content3");

    testrun(!dtn_list_copy(NULL, NULL));
    testrun(!dtn_list_copy((void **)&copy, NULL));
    testrun(!dtn_list_copy(NULL, list));

    // no item copy config (empty)
    testrun(!dtn_list_copy((void **)&copy, list));
    testrun(copy);
    testrun(0 == list->count(list));
    testrun(0 == copy->count(copy));

    // no item config (not empty)
    list->push(list, str1);
    list->push(list, str2);
    list->push(list, str3);
    testrun(3 == list->count(list));
    testrun(!dtn_list_copy((void **)&copy, list));

    // with items and items config
    list->config.item = dtn_data_string_data_functions();
    testrun(dtn_list_copy((void **)&copy, list));
    testrun(copy);
    testrun(3 == list->count(list));
    testrun(3 == copy->count(copy));

    // items copied
    testrun(strcmp(list->get(list, 1), copy->get(copy, 1)) == 0);
    testrun(list->get(list, 1) != copy->get(copy, 1));
    testrun(strcmp(list->get(list, 2), copy->get(copy, 2)) == 0);
    testrun(list->get(list, 2) != copy->get(copy, 2));
    testrun(strcmp(list->get(list, 3), copy->get(copy, 3)) == 0);
    testrun(list->get(list, 3) != copy->get(copy, 3));

    testrun(NULL == dtn_list_free(list));
    testrun(NULL == dtn_list_free(copy));

    // check valgrind run for strdup free

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_list_dump() {

    dtn_list *list = dtn_list_create((dtn_list_config){0});

    char *str1 = strdup("content1");
    char *str2 = strdup("content2");
    char *str3 = strdup("content3");

    testrun(!dtn_list_dump(NULL, NULL));
    testrun(!dtn_list_dump(stdout, NULL));
    testrun(!dtn_list_dump(NULL, list));

    // no item dump
    testrun(dtn_list_dump(stdout, list));

    // with items (no item dump)
    list->push(list, str1);
    list->push(list, str2);
    list->push(list, str3);
    testrun(3 == list->count(list));
    testrun(dtn_list_dump(stdout, list));

    // with items and items config
    list->config.item = dtn_data_string_data_functions();
    testrun(dtn_list_dump(stdout, list));

    testrun(NULL == dtn_list_free(list));
    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_list_data_functions() {

    dtn_data_function f = dtn_list_data_functions();

    testrun(f.clear == dtn_list_clear);
    testrun(f.free == dtn_list_free);
    testrun(f.copy == dtn_list_copy);
    testrun(f.dump == dtn_list_dump);

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_list_get_default_implementations() {

    dtn_list_default_implementations f = dtn_list_get_default_implementations();

    testrun(f.copy == list_copy);

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_list_set_magic_bytes() {

    dtn_list *list = calloc(1, sizeof(dtn_list));

    testrun(!dtn_list_set_magic_bytes(NULL));
    testrun(dtn_list_set_magic_bytes(list));

    testrun(list->magic_byte == MAGIC_BYTE);

    free(list);

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_list_remove_if_included() {

    dtn_list *list = calloc(1, sizeof(dtn_list));

    char *str1 = "content1";
    char *str2 = "content2";
    char *str3 = "content3";

    // not a list
    testrun(!dtn_list_remove_if_included(list, str1));
    free(list);
    list = dtn_list_create((dtn_list_config){0});

    testrun(!dtn_list_remove_if_included(NULL, NULL));
    testrun(!dtn_list_remove_if_included(NULL, str1));

    // no longer included (null)
    testrun(dtn_list_remove_if_included(list, NULL));

    // no longer included (content)
    testrun(dtn_list_remove_if_included(list, str1));
    testrun(dtn_list_remove_if_included(list, str2));
    testrun(dtn_list_remove_if_included(list, str3));

    testrun(list->count(list) == 0);
    list->push(list, str1);
    list->push(list, str2);
    list->push(list, str3);
    testrun(list->count(list) == 3);

    testrun(dtn_list_remove_if_included(list, str1));
    testrun(list->count(list) == 2);
    testrun(dtn_list_remove_if_included(list, str1));
    testrun(list->count(list) == 2);
    testrun(dtn_list_remove_if_included(list, str1));
    testrun(list->count(list) == 2);

    testrun(dtn_list_remove_if_included(list, str2));
    testrun(list->count(list) == 1);
    testrun(dtn_list_remove_if_included(list, str3));
    testrun(list->count(list) == 0);

    list->free(list);

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_list_is_empty() {

    dtn_list *list = dtn_list_create((dtn_list_config){0});
    void *function = list->is_empty;

    testrun(list);
    testrun(list->is_empty);
    testrun(list->is_empty(list));
    testrun(dtn_list_is_empty(list));

    list->is_empty = NULL;
    testrun(!dtn_list_is_empty(list));
    list->is_empty = function;
    testrun(dtn_list_is_empty(list));

    testrun(list->push(list, "test"));
    testrun(!list->is_empty(list));
    testrun(!dtn_list_is_empty(list));

    dtn_list_free(list);
    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_list_get_pos() {

    dtn_list *list = dtn_list_create((dtn_list_config){0});
    void *function = list->get_pos;

    void *a = "1";
    void *b = "2";
    void *c = "3";
    void *d = "4";

    testrun(list->push(list, a));
    testrun(list->push(list, b));
    testrun(list->push(list, c));

    testrun(list);
    testrun(list->get_pos);
    testrun(1 == list->get_pos(list, a));
    testrun(2 == list->get_pos(list, b));
    testrun(3 == list->get_pos(list, c));
    testrun(0 == list->get_pos(list, d));
    testrun(1 == dtn_list_get_pos(list, a));
    testrun(2 == dtn_list_get_pos(list, b));
    testrun(3 == dtn_list_get_pos(list, c));
    testrun(0 == dtn_list_get_pos(list, d));

    list->get_pos = NULL;
    testrun(0 == dtn_list_get_pos(list, a));
    list->get_pos = function;
    testrun(1 == dtn_list_get_pos(list, a));

    dtn_list_free(list);
    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_list_get() {

    dtn_list *list = dtn_list_create((dtn_list_config){0});
    void *function = list->get;

    void *a = "1";
    void *b = "2";
    void *c = "3";

    testrun(list->push(list, a));
    testrun(list->push(list, b));
    testrun(list->push(list, c));

    testrun(list);
    testrun(list->get);
    testrun(a == list->get(list, 1));
    testrun(b == list->get(list, 2));
    testrun(c == list->get(list, 3));
    testrun(0 == list->get(list, 4));
    testrun(a == dtn_list_get(list, 1));
    testrun(b == dtn_list_get(list, 2));
    testrun(c == dtn_list_get(list, 3));
    testrun(0 == dtn_list_get(list, 4));

    list->get = NULL;
    testrun(0 == dtn_list_get(list, 1));
    list->get = function;
    testrun(a == dtn_list_get(list, 1));

    dtn_list_free(list);
    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_list_set() {

    dtn_list *list = dtn_list_create((dtn_list_config){0});
    void *function = list->set;

    void *a = "1";
    void *b = "2";
    void *c = "3";
    void *d = "4";

    testrun(dtn_list_set(list, 1, a, NULL));
    testrun(dtn_list_set(list, 2, b, NULL));
    testrun(dtn_list_set(list, 3, c, NULL));
    testrun(a == dtn_list_get(list, 1));
    testrun(b == dtn_list_get(list, 2));
    testrun(c == dtn_list_get(list, 3));

    list->set = NULL;
    testrun(!dtn_list_set(list, 3, d, NULL));
    list->set = function;
    testrun(dtn_list_set(list, 3, d, NULL));
    testrun(d == dtn_list_get(list, 3));

    dtn_list_free(list);
    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_list_insert() {

    dtn_list *list = dtn_list_create((dtn_list_config){0});
    void *function = list->insert;

    void *a = "1";
    void *b = "2";
    void *c = "3";
    void *d = "4";

    testrun(dtn_list_insert(list, 1, a));
    testrun(dtn_list_insert(list, 1, b));
    testrun(dtn_list_insert(list, 1, c));
    testrun(c == dtn_list_get(list, 1));
    testrun(b == dtn_list_get(list, 2));
    testrun(a == dtn_list_get(list, 3));

    list->insert = NULL;
    testrun(!dtn_list_insert(list, 1, d));
    list->insert = function;
    testrun(dtn_list_insert(list, 1, d));
    testrun(d == dtn_list_get(list, 1));

    dtn_list_free(list);
    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_list_remove() {

    dtn_list *list = dtn_list_create((dtn_list_config){0});
    void *function = list->remove;

    void *a = "1";
    void *b = "2";
    void *c = "3";

    testrun(dtn_list_set(list, 1, a, NULL));
    testrun(dtn_list_set(list, 2, b, NULL));
    testrun(dtn_list_set(list, 3, c, NULL));
    testrun(a == dtn_list_remove(list, 1));
    testrun(b == dtn_list_remove(list, 1));

    list->remove = NULL;
    testrun(!dtn_list_remove(list, 1));
    list->remove = function;
    testrun(c == dtn_list_remove(list, 1));

    dtn_list_free(list);
    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

static void *dummy_free(void *data) {

    if (data) { /* ignore */
    };
    return NULL;
}

/*----------------------------------------------------------------------------*/

int test_dtn_list_delete() {

    dtn_list *list = dtn_list_create((dtn_list_config){0});

    void *a = "1";
    void *b = "2";
    void *c = "3";

    testrun(dtn_list_set(list, 1, a, NULL));
    testrun(dtn_list_set(list, 2, b, NULL));
    testrun(dtn_list_set(list, 3, c, NULL));

    testrun(3 == dtn_list_count(list));
    testrun(!list->config.item.free);
    testrun(!dtn_list_delete(NULL, 0));
    testrun(!dtn_list_delete(list, 0));
    testrun(3 == dtn_list_count(list));

    list->config.item.free = dummy_free;
    testrun(dtn_list_delete(list, 2));
    testrun(2 == dtn_list_count(list));

    testrun(a == dtn_list_get(list, 1));
    testrun(c == dtn_list_get(list, 2));

    testrun(dtn_list_delete(list, 1));
    testrun(1 == dtn_list_count(list));
    testrun(c == dtn_list_get(list, 1));

    // pos > items
    testrun(dtn_list_delete(list, 3));
    testrun(dtn_list_delete(list, 2));
    testrun(1 == dtn_list_count(list));
    testrun(c == dtn_list_get(list, 1));
    testrun(dtn_list_delete(list, 1));
    testrun(0 == dtn_list_count(list));

    dtn_list_free(list);
    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_list_push() {

    dtn_list *list = dtn_list_create((dtn_list_config){0});
    void *function = list->push;

    void *a = "1";
    void *b = "2";
    void *c = "3";

    testrun(dtn_list_push(list, a));
    testrun(dtn_list_push(list, b));
    testrun(a == dtn_list_get(list, 1));
    testrun(b == dtn_list_get(list, 2));

    list->push = NULL;
    testrun(!dtn_list_push(list, c));
    list->push = function;
    testrun(dtn_list_push(list, c));
    testrun(c == dtn_list_get(list, 3));

    dtn_list_free(list);
    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_list_pop() {

    dtn_list *list = dtn_list_create((dtn_list_config){0});
    void *function = list->pop;

    void *a = "1";
    void *b = "2";
    void *c = "3";

    testrun(dtn_list_push(list, a));
    testrun(dtn_list_push(list, b));
    testrun(dtn_list_push(list, c));
    testrun(c == dtn_list_pop(list));
    testrun(b == dtn_list_pop(list));

    list->pop = NULL;
    testrun(!dtn_list_pop(list));
    list->pop = function;
    testrun(a == dtn_list_pop(list));
    testrun(NULL == dtn_list_pop(list));

    dtn_list_free(list);
    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_list_count() {

    dtn_list *list = dtn_list_create((dtn_list_config){0});
    void *function = list->count;

    void *a = "1";
    void *b = "2";
    void *c = "3";

    testrun(dtn_list_push(list, a));
    testrun(1 == dtn_list_count(list));
    testrun(dtn_list_push(list, b));
    testrun(2 == dtn_list_count(list));
    testrun(dtn_list_push(list, c));
    testrun(3 == dtn_list_count(list));

    list->count = NULL;
    testrun(0 == dtn_list_count(list));
    list->count = function;
    testrun(3 == dtn_list_count(list));

    dtn_list_free(list);
    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

bool for_each_dummy(void *item, void *data) {

    if (item || data)
        return true;

    return true;
}

/*----------------------------------------------------------------------------*/

int test_dtn_list_for_each() {

    dtn_list *list = dtn_list_create((dtn_list_config){0});
    void *function = list->for_each;

    void *a = "1";
    void *b = "2";
    void *c = "3";

    testrun(dtn_list_push(list, a));
    testrun(dtn_list_push(list, b));
    testrun(dtn_list_push(list, c));

    testrun(dtn_list_for_each(list, NULL, for_each_dummy));

    list->for_each = NULL;
    testrun(!dtn_list_for_each(list, NULL, for_each_dummy));
    list->for_each = function;
    testrun(dtn_list_for_each(list, NULL, for_each_dummy));

    dtn_list_free(list);
    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_list_queue_push() {

    dtn_list *list = dtn_list_create((dtn_list_config){0});

    void *a = "1";
    void *b = "2";
    void *c = "3";

    testrun(!dtn_list_queue_push(NULL, NULL));
    testrun(!dtn_list_queue_push(list, NULL));
    testrun(!dtn_list_queue_push(NULL, a));

    testrun(dtn_list_queue_push(list, a));
    testrun(a == dtn_list_get(list, 1));

    testrun(dtn_list_queue_push(list, b));
    testrun(b == dtn_list_get(list, 1));
    testrun(a == dtn_list_get(list, 2));

    testrun(dtn_list_queue_push(list, c));
    testrun(c == dtn_list_get(list, 1));
    testrun(b == dtn_list_get(list, 2));
    testrun(a == dtn_list_get(list, 3));

    testrun(a == dtn_list_queue_pop(list));
    testrun(b == dtn_list_queue_pop(list));
    testrun(c == dtn_list_queue_pop(list));

    testrun(NULL == dtn_list_queue_pop(list));

    dtn_list_free(list);
    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_list_queue_pop() {

    dtn_list *list = dtn_list_create((dtn_list_config){0});

    void *a = "1";
    void *b = "2";
    void *c = "3";

    testrun(!dtn_list_queue_push(NULL, NULL));
    testrun(!dtn_list_queue_push(list, NULL));
    testrun(!dtn_list_queue_push(NULL, a));

    testrun(dtn_list_queue_push(list, a));
    testrun(a == dtn_list_get(list, 1));

    testrun(dtn_list_queue_push(list, b));
    testrun(b == dtn_list_get(list, 1));
    testrun(a == dtn_list_get(list, 2));

    testrun(dtn_list_queue_push(list, c));
    testrun(c == dtn_list_get(list, 1));
    testrun(b == dtn_list_get(list, 2));
    testrun(a == dtn_list_get(list, 3));

    testrun(a == dtn_list_queue_pop(list));
    testrun(b == dtn_list_queue_pop(list));
    testrun(c == dtn_list_queue_pop(list));

    testrun(NULL == dtn_list_queue_pop(list));

    dtn_list_free(list);
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

    testrun_test(test_dtn_list_create);

    DTN_LIST_PERFORM_INTERFACE_TESTS(dtn_list_create);

    testrun_test(test_dtn_list_cast);

    testrun_test(test_dtn_list_clear);
    testrun_test(test_dtn_list_free);
    testrun_test(test_dtn_list_copy);
    testrun_test(test_dtn_list_dump);

    testrun_test(test_dtn_list_data_functions);
    testrun_test(test_dtn_list_get_default_implementations);
    testrun_test(test_dtn_list_set_magic_bytes);

    testrun_test(test_dtn_list_remove_if_included);

    testrun_test(test_dtn_list_is_empty);
    testrun_test(test_dtn_list_get_pos);
    testrun_test(test_dtn_list_get);
    testrun_test(test_dtn_list_set);
    testrun_test(test_dtn_list_insert);
    testrun_test(test_dtn_list_remove);
    testrun_test(test_dtn_list_delete);
    testrun_test(test_dtn_list_push);
    testrun_test(test_dtn_list_pop);
    testrun_test(test_dtn_list_count);
    testrun_test(test_dtn_list_for_each);

    testrun_test(test_dtn_list_queue_push);
    testrun_test(test_dtn_list_queue_pop);

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
