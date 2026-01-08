/***
        ------------------------------------------------------------------------

        Copyright (c) 2019 German Aerospace Center DLR e.V. (GSOC)

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
        @file           dtn_buffer_test.c
        @author         Markus Toepfer
        @author         Michael Beer

        @date           2019-01-25

        @ingroup        dtn_buffer

        @brief          Unit tests of


        ------------------------------------------------------------------------
*/
#include "../include/testrun.h"
#include "dtn_buffer.c"
#include "dtn_string.h"

/*----------------------------------------------------------------------------*/

int test_dtn_buffer_create() {

    dtn_buffer *buffer = NULL;

    buffer = dtn_buffer_create(0);
    testrun(buffer);
    testrun(dtn_buffer_cast(buffer));
    testrun(NULL == buffer->start);
    testrun(0 == buffer->length);
    testrun(0 == buffer->capacity);
    testrun(NULL == dtn_buffer_free(buffer));

    buffer = dtn_buffer_create(100);
    testrun(buffer);
    testrun(dtn_buffer_cast(buffer));
    testrun(NULL != buffer->start);
    testrun(0 == buffer->length);
    testrun(100 == buffer->capacity);
    testrun(NULL == dtn_buffer_free(buffer));

    buffer = dtn_buffer_create(1000);
    testrun(buffer);
    testrun(dtn_buffer_cast(buffer));
    testrun(NULL != buffer->start);
    testrun(0 == buffer->length);
    testrun(1000 == buffer->capacity);
    testrun(NULL == dtn_buffer_free(buffer));

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_buffer_from_string() {

    dtn_buffer *buffer = NULL;

    testrun(NULL == dtn_buffer_from_string(NULL));

    buffer = dtn_buffer_from_string("ab");
    testrun(buffer);
    testrun(dtn_buffer_cast(buffer));
    testrun(NULL != buffer->start);
    testrun(2 == buffer->length);
    testrun(3 == buffer->capacity);
    testrun(0 == strncmp("ab", (char *)buffer->start, buffer->length));

    // The content of the buffer should still be 0 - terminated
    testrun(0 == buffer->start[buffer->length]);

    testrun(NULL == dtn_buffer_free(buffer));

    buffer = dtn_buffer_from_string("abcd");
    testrun(buffer);
    testrun(dtn_buffer_cast(buffer));
    testrun(NULL != buffer->start);
    testrun(4 == buffer->length);
    testrun(5 == buffer->capacity);
    testrun(0 == strncmp("abcd", (char *)buffer->start, buffer->length));
    // The content of the buffer should still be 0 - terminated
    testrun(0 == buffer->start[buffer->length]);

    testrun(NULL == dtn_buffer_free(buffer));

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

static int test_dtn_buffer_concat() {

    testrun(0 == dtn_buffer_concat(0, 0));

    dtn_buffer *b1 = dtn_buffer_from_string("123");
    testrun(3 == b1->length);

    testrun(b1 == dtn_buffer_concat(b1, 0));
    dtn_buffer *result = dtn_buffer_concat(0, b1);

    testrun(result != b1);

    testrun(0 != result);
    testrun(result->length = b1->length);
    testrun(0 == memcmp(result->start, b1->start, b1->length));

    result = dtn_buffer_free(result);

    dtn_buffer *empty = dtn_buffer_create(1);
    empty->length = 0;

    testrun(b1 == dtn_buffer_concat(b1, empty));
    testrun(3 == b1->length);

    empty = dtn_buffer_free(empty);

    dtn_buffer *b2 = dtn_buffer_from_string("4567");
    testrun(4 == b2->length);

    // Force target buffer too small
    b1->capacity = 4;

    b1 = dtn_buffer_concat(b1, b2);
    testrun(7 == b1->length);

    testrun(0 == dtn_string_compare((char *)b1->start, "1234567"));

    b1 = dtn_buffer_free(b1);

    // Target buffer with sufficient mem
    b1 = dtn_buffer_create(7);
    strcpy((char *)b1->start, "ab");
    b1->length = 2;

    result = dtn_buffer_concat(b1, b2);
    testrun(b1 == result);
    result = 0;
    testrun(6 == b1->length);

    testrun(0 == dtn_string_compare((char *)b1->start, "ab4567"));

    result = dtn_buffer_free(result);

    b2 = dtn_buffer_free(b2);
    b1 = dtn_buffer_free(b1);

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_buffer_cast() {

    dtn_buffer *buffer = dtn_buffer_create(0);

    testrun(buffer);
    testrun(dtn_buffer_cast(buffer));

    for (size_t i = 0; i < 0xffff; i++) {

        buffer->magic_byte = i;

        if (i == dtn_BUFFER_MAGIC_BYTE) {
            testrun(dtn_buffer_cast(buffer));
        } else {
            testrun(!dtn_buffer_cast(buffer));
        }
    }

    buffer->magic_byte = dtn_BUFFER_MAGIC_BYTE;
    testrun(NULL == dtn_buffer_free(buffer));

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_buffer_set() {

    dtn_buffer *buffer = dtn_buffer_create(0);

    testrun(buffer);
    testrun(dtn_buffer_cast(buffer));

    char *data = "test";
    testrun(!dtn_buffer_set(NULL, NULL, 0));
    testrun(!dtn_buffer_set(NULL, data, strlen(data)));
    testrun(!dtn_buffer_set(buffer, NULL, strlen(data)));
    testrun(!dtn_buffer_set(buffer, data, 0));

    testrun(dtn_buffer_set(buffer, data, strlen(data)));
    testrun(NULL != buffer->start);
    testrun(4 == buffer->length);
    testrun(4 == buffer->capacity);
    testrun(0 == strncmp(data, (char *)buffer->start, buffer->length));
    // check independent copy
    testrun(data != (char *)buffer->start);
    // check additional zero termination byte
    testrun(4 == strlen((char *)buffer->start));

    // set some longer data
    data = "abcd1234";
    testrun(dtn_buffer_set(buffer, data, strlen(data)));
    testrun(NULL != buffer->start);
    testrun(8 == buffer->length);
    testrun(8 == buffer->capacity);
    testrun(0 == strncmp(data, (char *)buffer->start, buffer->length));

    // set some smaller data
    data = "1234";
    testrun(dtn_buffer_set(buffer, data, strlen(data)));
    testrun(NULL != buffer->start);
    testrun(4 == buffer->length);
    testrun(8 == buffer->capacity);
    testrun(0 == strncmp(data, (char *)buffer->start, buffer->length));

    testrun(NULL == dtn_buffer_free(buffer));

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_buffer_clear() {

    dtn_buffer *buffer = dtn_buffer_create(0);

    testrun(buffer);
    testrun(dtn_buffer_cast(buffer));

    testrun(!dtn_buffer_clear(NULL));

    char *data = "test";

    // clear pointer buffer (empty)
    testrun(NULL == buffer->start);
    testrun(0 == buffer->length);
    testrun(0 == buffer->capacity);
    testrun(dtn_buffer_clear(buffer));
    testrun(NULL == buffer->start);
    testrun(0 == buffer->length);
    testrun(0 == buffer->capacity);

    // clear pointer buffer (set)
    buffer->start = (uint8_t *)data;
    buffer->length = 3;
    testrun(dtn_buffer_clear(buffer));
    testrun(NULL == buffer->start);
    testrun(0 == buffer->length);
    testrun(0 == buffer->capacity);

    // clear allocated buffer (set)
    testrun(dtn_buffer_set(buffer, data, strlen(data)));
    testrun(dtn_buffer_clear(buffer));
    testrun(NULL != buffer->start);
    testrun(0 == buffer->length);
    testrun(4 == buffer->capacity);
    for (size_t i = 0; i <= buffer->capacity; i++) {
        testrun(buffer->start[i] == 0);
    }

    // clear allocate buffer (empty)
    testrun(dtn_buffer_clear(buffer));
    testrun(NULL != buffer->start);
    testrun(0 == buffer->length);
    testrun(4 == buffer->capacity);
    for (size_t i = 0; i <= buffer->capacity; i++) {
        testrun(buffer->start[i] == 0);
    }

    testrun(NULL == dtn_buffer_free(buffer));

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_buffer_free() {

    dtn_buffer *buffer = dtn_buffer_create(0);

    testrun(NULL == dtn_buffer_free(NULL));
    testrun(NULL == dtn_buffer_free(buffer));

    char *data = "test";

    // free pointer buffer (empty)
    buffer = dtn_buffer_create(0);
    testrun(NULL == dtn_buffer_free(buffer));

    // clear pointer buffer (set)
    buffer = dtn_buffer_create(0);
    testrun(0 != buffer);

    buffer->start = (uint8_t *)data;
    buffer->length = 3;
    testrun(NULL == dtn_buffer_free(buffer));

    // clear allocated buffer (set)
    buffer = dtn_buffer_from_string(data);
    testrun(NULL == dtn_buffer_free(buffer));

    // clear allocate buffer (empty)
    buffer = dtn_buffer_from_string(data);
    testrun(dtn_buffer_clear(buffer));
    testrun(NULL == dtn_buffer_free(buffer));

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_buffer_copy() {

    dtn_buffer *buffer = dtn_buffer_create(0);
    dtn_buffer *copy = NULL;

    testrun(NULL == dtn_buffer_copy(NULL, NULL));

    copy = dtn_buffer_copy(NULL, buffer);
    testrun(copy != NULL);
    testrun(NULL == copy->start);
    testrun(0 == copy->length);
    testrun(0 == copy->capacity);
    copy = dtn_buffer_free(copy);
    testrun(0 == copy);

    testrun(NULL == dtn_buffer_copy((void **)&copy, NULL));
    testrun(copy == NULL);

    // copy empty pointer buffer
    testrun(dtn_buffer_copy((void **)&copy, buffer));
    testrun(copy != NULL);
    testrun(NULL == copy->start);
    testrun(0 == copy->length);
    testrun(0 == copy->capacity);

    copy = dtn_buffer_free(copy);
    testrun(0 == copy);

    char *data = "test";
    buffer->start = (uint8_t *)data;
    buffer->length = 3;

    copy = dtn_buffer_copy(0, buffer);
    testrun(NULL != copy->start);
    testrun(3 == copy->length);
    testrun(3 == copy->capacity);
    testrun(0 == strncmp(data, (char *)copy->start, copy->length));

    copy = dtn_buffer_free(copy);
    testrun(0 == copy);

    // copy non empty pointer buffer
    testrun(NULL != dtn_buffer_copy((void **)&copy, buffer));
    testrun(copy);
    testrun(NULL != copy->start);
    testrun(3 == copy->length);
    testrun(3 == copy->capacity);
    testrun(0 == strncmp(data, (char *)copy->start, copy->length));

    // copy from allocated to pointer buffer
    testrun(buffer == dtn_buffer_copy((void **)&buffer, copy));
    testrun(NULL != buffer->start);
    testrun(3 == buffer->length);
    testrun(3 == buffer->capacity);
    testrun(0 == strncmp(data, (char *)buffer->start, buffer->length));
    testrun(copy->start != buffer->start);

    // copy with longer data
    data = "12345678";
    testrun(dtn_buffer_set(buffer, data, strlen(data)));
    testrun(copy == dtn_buffer_copy((void **)&copy, buffer));
    testrun(NULL != copy->start);
    testrun(8 == copy->length);
    testrun(8 == copy->capacity);
    testrun(0 == strncmp(data, (char *)copy->start, copy->length));

    testrun(NULL == dtn_buffer_free(buffer));
    testrun(NULL == dtn_buffer_free(copy));

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_buffer_dump() {

    dtn_buffer *buffer = dtn_buffer_create(0);

    testrun(!dtn_buffer_dump(NULL, NULL));
    testrun(!dtn_buffer_dump(NULL, buffer));
    testrun(!dtn_buffer_dump(stdout, NULL));

    // dump empty pointer buffer
    testrun(dtn_buffer_dump(stdout, buffer));

    char *data = "test";
    buffer->start = (uint8_t *)data;
    buffer->length = 3;
    // dump non empty pointer buffer
    testrun(dtn_buffer_dump(stdout, buffer));

    testrun(dtn_buffer_set(buffer, data, strlen(data)));
    testrun(dtn_buffer_dump(stdout, buffer));

    testrun(NULL == dtn_buffer_free(buffer));
    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_buffer_push() {

    dtn_buffer *buffer = dtn_buffer_create(0);
    testrun(buffer);
    testrun(buffer->capacity == 0);

    testrun(!dtn_buffer_push(NULL, NULL, 0));
    testrun(!dtn_buffer_push(NULL, "test", 4));
    testrun(!dtn_buffer_push(buffer, NULL, 4));
    testrun(!dtn_buffer_push(buffer, "test", 0));

    testrun(NULL == buffer->start);
    testrun(0 == buffer->length);
    testrun(0 == buffer->capacity);

    testrun(dtn_buffer_push(buffer, "test", 4));
    testrun(NULL != buffer->start);
    testrun(4 == buffer->length);
    testrun(4 == buffer->capacity);
    testrun(0 == strncmp((char *)buffer->start, "test", 4));

    testrun(dtn_buffer_push(buffer, "abcd", 2));
    testrun(NULL != buffer->start);
    testrun(6 == buffer->length);
    testrun(6 == buffer->capacity);
    testrun(0 == strncmp((char *)buffer->start, "testab", 6));

    testrun(dtn_buffer_push(buffer, "abcd", 4));
    testrun(NULL != buffer->start);
    testrun(10 == buffer->length);
    testrun(10 == buffer->capacity);
    testrun(0 == strncmp((char *)buffer->start, "testababcd", 10));

    testrun(NULL == dtn_buffer_free(buffer));

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_buffer_data_functions() {

    dtn_data_function func = dtn_buffer_data_functions();
    testrun(func.clear == dtn_buffer_clear);
    testrun(func.free == dtn_buffer_free);
    testrun(func.copy == dtn_buffer_copy);
    testrun(func.dump == dtn_buffer_dump);

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_buffer_enable_caching() {

    dtn_buffer_enable_caching(1);

    dtn_buffer *buffer = dtn_buffer_create(3);

    testrun(0 != buffer);
    testrun(3 <= buffer->capacity);

    testrun(0 == dtn_buffer_free(buffer));

    dtn_buffer *buffer_2 = dtn_buffer_create(2);

    /*
     * Since we cache, and `buffer` was only freed, `buffer_2` should actually
     * be `buffer`
     *
     * BEWARE:
     * This actually tests implementation details,
     * but the only way I see to test anything reasonable in here...
     */

    testrun(buffer == buffer_2);
    testrun(2 <= buffer_2->capacity);

    testrun(0 == dtn_buffer_free(buffer_2));

    buffer_2 = dtn_buffer_create(7);

    testrun(buffer == buffer_2);

    /* Don't need `buffer` as reference any more */
    buffer = 0;

    testrun(7 <= buffer_2->capacity);

    buffer_2 = dtn_buffer_free(buffer_2);
    testrun(0 == buffer_2);

    /* Should enable default cache size */
    dtn_buffer_enable_caching(0);

    buffer = dtn_buffer_create(3);

    testrun(0 != buffer);
    testrun(3 <= buffer->capacity);

    testrun(0 == dtn_buffer_free(buffer));

    /* Now check if we get the same buffer again ... */
    /* See note further up */

    buffer_2 = dtn_buffer_create(2);

    testrun(buffer == buffer_2);
    testrun(2 <= buffer_2->capacity);

    testrun(0 == dtn_buffer_free(buffer_2));

    buffer_2 = dtn_buffer_create(7);

    testrun(buffer == buffer_2);
    testrun(7 <= buffer_2->capacity);

    buffer_2 = dtn_buffer_free(buffer_2);

    testrun(0 == buffer_2);

    dtn_registered_cache_free_all();
    g_cache = NULL;
    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_buffer_shift() {

    dtn_buffer *buffer = dtn_buffer_create(100);

    testrun(buffer);
    testrun(NULL != buffer->start);
    testrun(0 == buffer->length);
    testrun(100 == buffer->capacity);

    for (size_t i = 0; i < 100; i++) {
        buffer->start[i] = i;
    }
    buffer->length = 100;

    uint8_t *ptr = buffer->start + 10;

    testrun(!dtn_buffer_shift(NULL, NULL));
    testrun(!dtn_buffer_shift(buffer, NULL));
    testrun(!dtn_buffer_shift(NULL, ptr));

    testrun(dtn_buffer_shift(buffer, ptr));
    // dtn_buffer_dump(stdout, buffer);
    testrun(buffer->start[0] == 10);
    testrun(buffer->length == 90);
    testrun(buffer->capacity == 100);

    for (size_t i = 0; i < 100; i++) {

        if (i < 90) {
            testrun(buffer->start[i] == i + 10);
        } else {
            testrun(buffer->start[i] == 0);
        }
    }

    // try to shift > length
    ptr = buffer->start + buffer->length + 1;
    testrun(!dtn_buffer_shift(buffer, ptr));
    testrun(buffer->start[0] == 10);
    testrun(buffer->length == 90);
    testrun(buffer->capacity == 100);

    // try to shift by length
    ptr = buffer->start + buffer->length;
    testrun(dtn_buffer_shift(buffer, ptr));
    testrun(buffer->start[0] == 0);
    testrun(buffer->length == 0);
    testrun(buffer->capacity == 100);

    char *somewhere = "somewhere";

    ptr = (uint8_t *)somewhere;
    testrun(!dtn_buffer_shift(buffer, ptr));

    testrun(NULL == dtn_buffer_free(buffer));
    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_buffer_shift_length() {

    dtn_buffer *buffer = dtn_buffer_create(100);

    testrun(buffer);
    testrun(NULL != buffer->start);
    testrun(0 == buffer->length);
    testrun(100 == buffer->capacity);

    for (size_t i = 0; i < 100; i++) {
        buffer->start[i] = i;
    }
    buffer->length = 100;

    testrun(!dtn_buffer_shift_length(NULL, 0));
    testrun(!dtn_buffer_shift_length(NULL, 10));

    testrun(dtn_buffer_shift_length(buffer, 0));

    testrun(dtn_buffer_shift_length(buffer, 10));
    testrun(buffer->start[0] == 10);
    testrun(buffer->length == 90);
    testrun(buffer->capacity == 100);

    for (size_t i = 0; i < 100; i++) {

        if (i < 90) {
            testrun(buffer->start[i] == i + 10);
        } else {
            testrun(buffer->start[i] == 0);
        }
    }

    // try to shift > length
    testrun(!dtn_buffer_shift_length(buffer, buffer->length + 1));
    testrun(buffer->start[0] == 10);
    testrun(buffer->length == 90);
    testrun(buffer->capacity == 100);

    // try to shift by length
    testrun(dtn_buffer_shift_length(buffer, buffer->length));
    testrun(buffer->start[0] == 0);
    testrun(buffer->length == 0);
    testrun(buffer->capacity == 100);

    testrun(!dtn_buffer_shift_length(buffer, 1000));

    testrun(NULL == dtn_buffer_free(buffer));

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_buffer_extend() {

    dtn_buffer *buffer = dtn_buffer_create(1);

    testrun(buffer);
    testrun(NULL != buffer->start);
    testrun(0 == buffer->length);
    testrun(1 == buffer->capacity);

    buffer->start[0] = 'a';
    buffer->length = 1;

    testrun(!dtn_buffer_extend(NULL, 0));
    testrun(!dtn_buffer_extend(buffer, 0));
    testrun(!dtn_buffer_extend(NULL, 1));

    testrun(dtn_buffer_extend(buffer, 1));
    testrun(buffer->length == 1);
    testrun(buffer->capacity == 2);
    testrun(buffer->start[0] == 'a');

    testrun(dtn_buffer_extend(buffer, 100));
    testrun(buffer->length == 1);
    testrun(buffer->capacity == 102);
    testrun(buffer->start[0] == 'a');

    testrun(dtn_buffer_extend(buffer, 10));
    testrun(buffer->length == 1);
    testrun(buffer->capacity == 112);
    testrun(buffer->start[0] == 'a');

    testrun(dtn_buffer_extend(buffer, 8));
    testrun(buffer->length == 1);
    testrun(buffer->capacity == 120);
    testrun(buffer->start[0] == 'a');

    testrun(NULL == dtn_buffer_free(buffer));

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
    testrun_test(test_dtn_buffer_create);
    testrun_test(test_dtn_buffer_from_string);
    testrun_test(test_dtn_buffer_concat);
    testrun_test(test_dtn_buffer_cast);
    testrun_test(test_dtn_buffer_set);
    testrun_test(test_dtn_buffer_clear);
    testrun_test(test_dtn_buffer_free);
    testrun_test(test_dtn_buffer_copy);
    testrun_test(test_dtn_buffer_dump);
    testrun_test(test_dtn_buffer_push);
    testrun_test(test_dtn_buffer_data_functions);
    testrun_test(test_dtn_buffer_enable_caching);
    testrun_test(test_dtn_buffer_shift);
    testrun_test(test_dtn_buffer_shift_length);
    testrun_test(test_dtn_buffer_extend);

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
