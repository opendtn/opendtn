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
        @file           dtn_ringbuffer_test.c
        @author         Michael Beer

        @date           2018-01-11

        @ingroup        dtn_ringbuffer

        @brief          Unit tests of


        ------------------------------------------------------------------------
*/
#include "dtn_ringbuffer.c"
#include "../include/testrun.h"
/*
 *      ------------------------------------------------------------------------
 *
 *      TEST HELPER
 *
 *      ------------------------------------------------------------------------
 */

void count_calls(void *additional_arg, void *element_to_free) {

  if (0 == element_to_free)
    return;

  int *ip = (int *)additional_arg;

  ++*ip;
}

/*
 *      ------------------------------------------------------------------------
 *
 *      TEST CASES                                                      #CASES
 *
 *      ------------------------------------------------------------------------
 */

int test_dtn_ringbuffer_create() {

  testrun(0 == dtn_ringbuffer_create(0, 0, 0));

  dtn_ringbuffer *buffer = dtn_ringbuffer_create(1, 0, 0);
  testrun(0 != buffer);
  testrun(0 != buffer->pop);
  testrun(0 != buffer->insert);
  testrun(0 != buffer->clear);
  testrun(0 != buffer->free);

  buffer->free(buffer);

  buffer = dtn_ringbuffer_create(1, 0, 0);
  testrun(0 != buffer);

  buffer = buffer->free(buffer);

  /* Should work with some arbitrary additional argument as well -
   * the buffer itself would not use it in ay ways */
  buffer = dtn_ringbuffer_create(1, 0, &buffer);

  buffer = buffer->free(buffer);

  return testrun_log_success();
}

/*---------------------------------------------------------------------------*/

int test_impl_ringbuffer_capacity() {

  dtn_ringbuffer *buffer = dtn_ringbuffer_create(1, 0, 0);
  DTN_ASSERT(0 == buffer->capacity(0));
  DTN_ASSERT(1 == buffer->capacity(buffer));
  DTN_ASSERT(1 == buffer->capacity(buffer));
  DTN_ASSERT(1 == buffer->capacity(buffer));
  buffer->free(buffer);

  buffer = dtn_ringbuffer_create(132, 0, 0);
  DTN_ASSERT(132 == buffer->capacity(buffer));
  DTN_ASSERT(132 == buffer->capacity(buffer));
  DTN_ASSERT(132 == buffer->capacity(buffer));
  DTN_ASSERT(132 == buffer->capacity(buffer));
  buffer->free(buffer);

  return testrun_log_success();
}

/*---------------------------------------------------------------------------*/

int test_impl_ringbuffer_pop() {

  int a = 1;
  int b = 2;
  int c = 3;
  int d = 4;

  int free_count = 0;

  dtn_ringbuffer *buffer = 0;
  buffer = dtn_ringbuffer_create(1, count_calls, &free_count);

  testrun(0 == buffer->pop(0));
  testrun(0 == buffer->pop(buffer));

  testrun(buffer->insert(buffer, &a));
  testrun(&a == buffer->pop(buffer));

  /* Free-function should have not been called */
  testrun(0 == free_count);
  testrun(0 == buffer->pop(buffer));

  testrun(buffer->insert(buffer, &a));
  testrun(buffer->insert(buffer, &b));

  testrun(1 == free_count);
  free_count = 0;

  testrun(&b == buffer->pop(buffer));

  testrun(0 == buffer->pop(buffer));
  testrun(0 == buffer->pop(buffer));
  testrun(0 == buffer->pop(buffer));

  buffer->free(buffer);

  free_count = 0;
  buffer = dtn_ringbuffer_create(3, count_calls, &free_count);

  testrun(buffer->insert(buffer, &a));
  testrun(&a == buffer->pop(buffer));
  testrun(0 == free_count);

  testrun(buffer->insert(buffer, &b));
  testrun(buffer->insert(buffer, &c));
  testrun(&b == buffer->pop(buffer));
  testrun(0 == free_count);
  testrun(&c == buffer->pop(buffer));
  testrun(0 == free_count);

  testrun(0 == buffer->pop(buffer));
  testrun(0 == free_count);

  /* This should wrap */
  testrun(buffer->insert(buffer, &d));
  testrun(buffer->insert(buffer, &c));
  testrun(buffer->insert(buffer, &b));
  testrun(buffer->insert(buffer, &a));

  testrun(1 == free_count);

  testrun(&c == buffer->pop(buffer));
  testrun(&b == buffer->pop(buffer));
  testrun(&a == buffer->pop(buffer));

  /* Buffer should now be empty again ... */
  testrun(0 == buffer->pop(buffer));
  testrun(0 == buffer->pop(buffer));

  free_count = 0;

  testrun(buffer->insert(buffer, &d));
  testrun(buffer->insert(buffer, &c));
  testrun(buffer->insert(buffer, &b));
  testrun(buffer->insert(buffer, &a));

  testrun(1 == free_count);

  testrun(buffer->insert(buffer, &d));

  testrun(2 == free_count);

  testrun(&b == buffer->pop(buffer));
  testrun(&a == buffer->pop(buffer));
  testrun(&d == buffer->pop(buffer));
  testrun(0 == buffer->pop(buffer));
  testrun(0 == buffer->pop(buffer));

  buffer->free(buffer);
  return testrun_log_success();
}

/*---------------------------------------------------------------------------*/

int test_impl_ringbuffer_insert() {

  /* Since we wont use internals of the ringbuffer in here,
   * this test is not really this helpful.
   * Better tests are performed in test_impl_ringbuffer_pop()
   */
  int a = 1;
  int b = 2;
  int c = 3;
  int d = 4;

  int free_count = 0;

  dtn_ringbuffer *buffer = 0;
  buffer = dtn_ringbuffer_create(1, count_calls, &free_count);

  testrun(0 == buffer->insert(0, 0));

  testrun(buffer->insert(buffer, &a));

  buffer->free(buffer);

  testrun(free_count == 1);
  free_count = 0;

  buffer = dtn_ringbuffer_create(1, count_calls, &free_count);

  testrun(buffer->insert(buffer, &a));
  testrun(buffer->insert(buffer, &b));

  buffer->free(buffer);

  testrun(free_count == 2);
  free_count = 0;

  /*------------------------------------------------------------------------*/

  buffer = dtn_ringbuffer_create(1, count_calls, &free_count);

  testrun(0 == buffer->insert(0, 0));

  testrun(buffer->insert(buffer, &a));
  testrun(buffer->insert(buffer, &b));
  testrun(buffer->insert(buffer, &a));

  buffer->free(buffer);

  testrun(free_count == 3);
  free_count = 0;

  buffer = dtn_ringbuffer_create(3, count_calls, &free_count);

  testrun(buffer->insert(buffer, &a));
  testrun(buffer->insert(buffer, &a));
  testrun(buffer->insert(buffer, &b));
  testrun(buffer->insert(buffer, &c));
  testrun(buffer->insert(buffer, &b));
  testrun(buffer->insert(buffer, &d));

  buffer->free(buffer);

  testrun(free_count == 6);
  free_count = 0;

  buffer = dtn_ringbuffer_create(132, count_calls, &free_count);

  testrun(buffer->insert(buffer, &a));

  /* Fill until just before wrap should occur */
  for (size_t i = 1; i < 132; ++i) {

    testrun(buffer->insert(buffer, &b));
  }

  for (size_t i = 0; i < 3; ++i) {

    testrun(buffer->insert(buffer, &c));
  }

  buffer->free(buffer);
  testrun(free_count == 135);
  free_count = 0;

  return testrun_log_success();
}

/*---------------------------------------------------------------------------*/

int test_impl_ringbuffer_clear() {

  int a, b, c;

  int free_count = 0;

  testrun(!ringbuffer_clear(0));

  dtn_ringbuffer *buffer = dtn_ringbuffer_create(1, 0, 0);
  testrun(ringbuffer_clear(buffer));

  testrun(1 == buffer->capacity(buffer));

  testrun(buffer->insert(buffer, &a));
  testrun(ringbuffer_clear(buffer));

  /* buffer should be empty ... */
  testrun(0 == buffer->pop(buffer));
  testrun(0 == buffer->pop(buffer));

  buffer->free(buffer);

  buffer = dtn_ringbuffer_create(5, count_calls, &free_count);
  testrun(ringbuffer_clear(buffer));
  testrun(0 == free_count);

  testrun(5 == buffer->capacity(buffer));

  testrun(buffer->insert(buffer, &a));
  testrun(buffer->insert(buffer, &b));
  testrun(buffer->insert(buffer, &c));

  testrun(ringbuffer_clear(buffer));
  testrun(3 == free_count);

  /* buffer should be empty ... */
  testrun(0 == buffer->pop(buffer));
  testrun(0 == buffer->pop(buffer));

  buffer = buffer->free(buffer);

  free_count = 0;

  buffer = dtn_ringbuffer_create(1, count_calls, &free_count);

  testrun(buffer->insert(buffer, &a));

  testrun(buffer->clear(buffer));

  testrun(1 == free_count);

  /* buffer should be empty ... */

  testrun(0 == buffer->pop(buffer));
  testrun(0 == buffer->pop(buffer));

  buffer = buffer->free(buffer);

  return testrun_log_success();
}

/*---------------------------------------------------------------------------*/

int test_impl_ringbuffer_free() {

  DTN_ASSERT(0 == ringbuffer_free(0));

  return testrun_log_success();
}

/*---------------------------------------------------------------------------*/

int test_impl_ringbuffer_get_statistics() {

  dtn_ringbuffer *buffer = dtn_ringbuffer_create(1, 0, 0);

  int a = 1;
  testrun(buffer->insert(buffer, &a));
  dtn_ringbuffer_statistics stats = buffer->get_statistics(buffer);
  testrun(0 == stats.elements_dropped);
  testrun(1 == stats.elements_inserted);

  testrun(buffer->insert(buffer, &a));
  stats = buffer->get_statistics(buffer);

  for (size_t i = 1; i < 16; ++i) {

    testrun(buffer->insert(buffer, &a));
    stats = buffer->get_statistics(buffer);
    // testrun(i <= stats.elements_dropped);
    testrun(i + 2 == stats.elements_inserted);
  }

  buffer->pop(buffer);
  stats = buffer->get_statistics(buffer);
  //    testrun(16 == stats.elements_dropped);
  testrun(17 == stats.elements_inserted);

  buffer = buffer->free(buffer);

  return testrun_log_success();
}

/******************************************************************************
 *                                  CACHING
 ******************************************************************************/

static dtn_ringbuffer **get_ringbuffers(size_t num, size_t capacity_single_rb) {

  DTN_ASSERT(0 < num);

  dtn_ringbuffer **rb = calloc(num, sizeof(dtn_ringbuffer *));
  DTN_ASSERT(0 != rb);

  for (size_t i = 0; i < num; ++i) {

    rb[i] = dtn_ringbuffer_create(capacity_single_rb, 0, 0);
  }

  return rb;
}

/*----------------------------------------------------------------------------*/

void free_ringbuffers(dtn_ringbuffer **rbs, size_t const num_ringbuffers) {

  DTN_ASSERT(0 != rbs);

  for (size_t i = 0; i < num_ringbuffers; ++i) {

    if (0 != rbs[i]) {
      rbs[i]->free(rbs[i]);
    }
  }

  free(rbs);
}

/*----------------------------------------------------------------------------*/

static bool pointer_in_pointer_array(void *ptr, void **ptr_array,
                                     size_t num_entries) {

  for (size_t i = 0; i < num_entries; ++i) {

    if (ptr == ptr_array[i])
      return true;
  }

  return false;
}

/*----------------------------------------------------------------------------*/

int test_dtn_ringbuffer_enable_caching() {

  dtn_ringbuffer_enable_caching(0);
  dtn_ringbuffer_enable_caching(2);

  size_t num_ringbuffers = 4;

  dtn_ringbuffer **rbs = get_ringbuffers(num_ringbuffers, 1);

  for (size_t i = 0; i < num_ringbuffers; ++i) {

    testrun(0 == rbs[i]->free(rbs[i]));
  }

  /* rb now holds the old pointers, which should reappear for new ringbuffers
   */

  dtn_ringbuffer **rbs_2 = get_ringbuffers(3, 1);

  testrun(pointer_in_pointer_array(rbs_2[0], (void **)rbs, num_ringbuffers));
  testrun(pointer_in_pointer_array(rbs_2[1], (void **)rbs, num_ringbuffers));

  /* it is tempting to test for the opposite - if caching is disabled,
   * check for new ringbuffers to hold different mem locations as the
   * freed ones - however, malloc could reuse their mem anyways, thus
   * we cannot rely on that - and actually, we don't care */

  free_ringbuffers(rbs_2, 3);
  rbs_2 = 0;

  free(rbs);
  rbs = 0;

  return testrun_log_success();
}
/*----------------------------------------------------------------------------*/

int tear_down() {
  dtn_registered_cache_free_all();
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
  testrun_test(test_dtn_ringbuffer_create);
  testrun_test(test_impl_ringbuffer_capacity);
  testrun_test(test_impl_ringbuffer_pop);
  testrun_test(test_impl_ringbuffer_insert);
  testrun_test(test_impl_ringbuffer_clear);
  testrun_test(test_impl_ringbuffer_free);
  testrun_test(test_impl_ringbuffer_get_statistics);
  testrun_test(test_dtn_ringbuffer_enable_caching);
  testrun_test(tear_down);

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