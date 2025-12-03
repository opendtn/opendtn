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

        This file is part of the openvocs project. http://openvocs.org

        ------------------------------------------------------------------------
*//**
        @file           dtn_data_function_tests.c
        @author         Markus Toepfer

        @date           2018-03-23

        @ingroup        dtn_data_structures

        @brief          Unit Tests of dtn_data_function.


        ------------------------------------------------------------------------
*/
#include "dtn_data_function.c"
#include "../include/testrun.h"
/*----------------------------------------------------------------------------*/

/*
 *      ------------------------------------------------------------------------
 *
 *      TEST CASES                                                      #CASES
 *
 *      ------------------------------------------------------------------------
 */

/*----------------------------------------------------------------------------*/

int test_dtn_data_function_create() {

  dtn_data_function *func = dtn_data_function_create();

  testrun(func);
  testrun(func->free == NULL);
  testrun(func->clear == NULL);
  testrun(func->copy == NULL);
  testrun(func->dump == NULL);

  dtn_data_function_free(func);
  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_data_function_clear() {

  dtn_data_function *func = dtn_data_function_create();

  testrun(func);
  testrun(func->free == NULL);
  testrun(func->clear == NULL);
  testrun(func->copy == NULL);
  testrun(func->dump == NULL);

  testrun(!dtn_data_function_clear(NULL));
  testrun(dtn_data_function_clear(func));

  func->clear = dtn_data_string_clear;
  func->free = dtn_data_string_free;
  func->copy = dtn_data_string_copy;
  func->dump = dtn_data_string_dump;

  testrun(dtn_data_function_clear(func));
  testrun(func->free == NULL);
  testrun(func->clear == NULL);
  testrun(func->copy == NULL);
  testrun(func->dump == NULL);

  dtn_data_function_free(func);
  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_data_function_free() {

  dtn_data_function *func = dtn_data_function_create();

  testrun(func);
  testrun(func->free == NULL);
  testrun(func->clear == NULL);
  testrun(func->copy == NULL);
  testrun(func->dump == NULL);

  testrun(NULL == dtn_data_function_free(NULL));
  testrun(NULL == dtn_data_function_free(func));

  func = dtn_data_function_create();
  func->clear = dtn_data_string_clear;
  func->free = dtn_data_string_free;
  func->copy = dtn_data_string_copy;
  func->dump = dtn_data_string_dump;
  testrun(NULL == dtn_data_function_free(func));

  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_data_function_copy() {

  dtn_data_function *func = dtn_data_function_create();
  dtn_data_function *copy = NULL;
  dtn_data_function *dest = NULL;

  testrun(func);
  testrun(func->free == NULL);
  testrun(func->clear == NULL);
  testrun(func->copy == NULL);
  testrun(func->dump == NULL);

  testrun(!dtn_data_function_copy(NULL, NULL));
  testrun(!dtn_data_function_copy(&dest, NULL));
  testrun(!dtn_data_function_copy(NULL, func));

  copy = dtn_data_function_copy(&dest, func);
  testrun(copy);
  testrun(copy == dest);
  testrun(copy->free == NULL);
  testrun(copy->clear == NULL);
  testrun(copy->copy == NULL);
  testrun(copy->dump == NULL);

  func->clear = dtn_data_string_clear;
  func->free = dtn_data_string_free;
  func->copy = dtn_data_string_copy;
  func->dump = dtn_data_string_dump;

  copy = dtn_data_function_copy(&dest, func);
  testrun(copy);
  testrun(copy == dest);
  testrun(copy->free == dtn_data_string_free);
  testrun(copy->clear == dtn_data_string_clear);
  testrun(copy->copy == dtn_data_string_copy);
  testrun(copy->dump == dtn_data_string_dump);

  func->free = NULL;
  func->copy = NULL;

  copy = dtn_data_function_copy(&dest, func);
  testrun(copy);
  testrun(copy == dest);
  testrun(copy->free == NULL);
  testrun(copy->clear == dtn_data_string_clear);
  testrun(copy->copy == NULL);
  testrun(copy->dump == dtn_data_string_dump);

  // INDEPENDENCE
  dtn_data_function_free(func);

  testrun(copy->free == NULL);
  testrun(copy->clear == dtn_data_string_clear);
  testrun(copy->copy == NULL);
  testrun(copy->dump == dtn_data_string_dump);

  dtn_data_function_free(copy);
  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_data_function_dump() {

  dtn_data_function *func = dtn_data_function_create();

  func->clear = dtn_data_string_clear;
  func->free = dtn_data_string_free;
  func->copy = dtn_data_string_copy;
  func->dump = dtn_data_string_dump;

  testrun(!dtn_data_function_dump(NULL, NULL));
  testrun(!dtn_data_function_dump(stdout, NULL));
  testrun(!dtn_data_function_dump(NULL, func));

  testrun_log("TEST DUMP FOLLOWS");
  testrun(dtn_data_function_dump(stdout, func));

  dtn_data_function_free(func);
  return testrun_log_success();
}

/*
 *      ------------------------------------------------------------------------
 *
 *      EXAMPLES
 *
 *      ------------------------------------------------------------------------
 */

/*----------------------------------------------------------------------------*/

int test_dtn_data_string_clear() {

  char *string = strdup("test");

  testrun(string[0] == 't');

  testrun(!dtn_data_string_clear(NULL));
  testrun(dtn_data_string_clear(string));

  testrun(string[0] == 0);
  testrun(strlen(string) == 0);

  free(string);

  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_data_string_free() {

  char *string = strdup("test");

  testrun(NULL == dtn_data_string_free(NULL));
  testrun(NULL == dtn_data_string_free(string));

  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_data_string_copy() {

  char *string = strdup("test");
  char *copy = NULL;

  testrun(!dtn_data_string_copy(NULL, NULL));
  testrun(!dtn_data_string_copy((void **)&copy, NULL));
  testrun(!dtn_data_string_copy(NULL, string));

  testrun(dtn_data_string_copy((void **)&copy, string));

  // check same content
  testrun(strncmp(string, copy, strlen(string)) == 0);

  // check independent content
  string = dtn_data_string_free(string);
  testrun(strncmp("test", copy, strlen(copy)) == 0);
  copy = dtn_data_string_free(copy);
  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_data_string_dump() {

  char *string = strdup("test");

  testrun(string[0] == 't');

  testrun(!dtn_data_string_dump(NULL, NULL));
  testrun(!dtn_data_string_dump(stdout, NULL));
  testrun(!dtn_data_string_dump(NULL, string));

  testrun_log("TEST DUMP FOLLOWS");
  testrun(dtn_data_string_dump(stdout, string));

  free(string);
  return testrun_log_success();
}

/*
 *      ------------------------------------------------------------------------
 *
 *      EXAMPLE IMPLEMENTATIONS (int64 pointer based)
 *
 *      ------------------------------------------------------------------------
 */

int test_dtn_data_int64_clear() {

  int64_t number = 123;

  testrun(!dtn_data_int64_clear(NULL));
  testrun(dtn_data_int64_clear((void *)&number));

  testrun(number == 0);
  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_data_int64_free() {

  int64_t *number = calloc(1, sizeof(int64_t));
  *number = 123;

  testrun(NULL == dtn_data_int64_free(NULL));
  testrun(NULL == dtn_data_int64_free(number));

  // check valgrind
  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_data_int64_copy() {

  int64_t number = 123;
  void *dest = NULL;

  testrun(!dtn_data_int64_copy(NULL, NULL));
  testrun(!dtn_data_int64_copy((void **)&dest, NULL));
  testrun(!dtn_data_int64_copy(NULL, &number));

  testrun(dtn_data_int64_copy((void **)&dest, &number));
  testrun(*(int64_t *)dest == 123);

  free(dest);

  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_data_int64_dump() {

  int64_t number = 123;

  testrun(!dtn_data_int64_dump(NULL, NULL));
  testrun(!dtn_data_int64_dump(stdout, NULL));
  testrun(!dtn_data_int64_dump(NULL, (void *)&number));

  testrun_log("TEST DUMP FOLLOWS");
  testrun(dtn_data_int64_dump(stdout, (void *)&number));

  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_data_int64_direct_copy() {

  intptr_t number = 123;
  intptr_t copy = 0;

  testrun(!dtn_data_int64_direct_copy(NULL, NULL));
  testrun(!dtn_data_int64_direct_copy((void **)&copy, NULL));
  testrun(!dtn_data_int64_direct_copy(NULL, &number));

  testrun(dtn_data_int64_direct_copy((void **)&copy, (void *)number));
  testrun(copy == 123);

  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_data_int64_direct_dump() {

  intptr_t number = 123;

  testrun(!dtn_data_int64_direct_dump(NULL, NULL));
  testrun(!dtn_data_int64_direct_dump(stdout, NULL));
  testrun(!dtn_data_int64_direct_dump(NULL, (void *)number));

  testrun_log("TEST DUMP FOLLOWS");
  testrun(dtn_data_int64_direct_dump(stdout, (void *)number));

  return testrun_log_success();
}

/*
 *      ------------------------------------------------------------------------
 *
 *      Standard free
 *
 *      ------------------------------------------------------------------------
 */

int test_dtn_data_function_wrapper_free() {

  void *ptr = calloc(1, sizeof(dtn_data_function));

  testrun(NULL == dtn_data_function_wrapper_free(NULL));
  testrun(NULL == dtn_data_function_wrapper_free(ptr));

  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_data_string_data_functions() {

  dtn_data_function function = dtn_data_string_data_functions();

  testrun(function.clear == dtn_data_string_clear);
  testrun(function.free == dtn_data_string_free);
  testrun(function.copy == dtn_data_string_copy);
  testrun(function.dump == dtn_data_string_dump);

  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_data_string_data_functions_are_valid() {

  dtn_data_function function = dtn_data_string_data_functions();

  testrun(dtn_data_string_data_functions_are_valid(&function));

  function.clear = NULL;
  testrun(!dtn_data_string_data_functions_are_valid(&function));
  function.clear = dtn_data_int64_clear;
  testrun(!dtn_data_string_data_functions_are_valid(&function));
  function.clear = dtn_data_string_clear;
  testrun(dtn_data_string_data_functions_are_valid(&function));

  function.free = NULL;
  testrun(!dtn_data_string_data_functions_are_valid(&function));
  function.free = dtn_data_int64_free;
  testrun(!dtn_data_string_data_functions_are_valid(&function));
  function.free = dtn_data_string_free;
  testrun(dtn_data_string_data_functions_are_valid(&function));

  function.copy = NULL;
  testrun(!dtn_data_string_data_functions_are_valid(&function));
  function.copy = dtn_data_int64_copy;
  testrun(!dtn_data_string_data_functions_are_valid(&function));
  function.copy = dtn_data_string_copy;
  testrun(dtn_data_string_data_functions_are_valid(&function));

  function.dump = NULL;
  testrun(!dtn_data_string_data_functions_are_valid(&function));
  function.dump = dtn_data_int64_dump;
  testrun(!dtn_data_string_data_functions_are_valid(&function));
  function.dump = dtn_data_string_dump;
  testrun(dtn_data_string_data_functions_are_valid(&function));

  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_data_function_allocated() {

  dtn_data_function *func = dtn_data_function_allocated(NULL);
  testrun(func);
  testrun(func->clear == NULL);
  testrun(func->copy == NULL);
  testrun(func->dump == NULL);
  testrun(func->free == NULL);
  free(func);

  func = dtn_data_function_allocated(dtn_data_string_data_functions);
  testrun(func);
  testrun(func->clear == dtn_data_string_clear);
  testrun(func->copy == dtn_data_string_copy);
  testrun(func->dump == dtn_data_string_dump);
  testrun(func->free == dtn_data_string_free);
  free(func);

  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_data_uint64_clear() {

  uint64_t number = 123;

  testrun(!dtn_data_uint64_clear(NULL));
  testrun(dtn_data_uint64_clear((void *)&number));

  testrun(number == 0);
  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_data_uint64_free() {

  uint64_t *number = calloc(1, sizeof(uint64_t));
  *number = 123;

  testrun(NULL == dtn_data_uint64_free(NULL));
  testrun(NULL == dtn_data_uint64_free(number));

  // check valgrind
  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_data_uint64_copy() {

  uint64_t number = 123;
  void *dest = NULL;

  testrun(!dtn_data_uint64_copy(NULL, NULL));
  testrun(!dtn_data_uint64_copy((void **)&dest, NULL));
  testrun(!dtn_data_uint64_copy(NULL, &number));

  testrun(dtn_data_uint64_copy((void **)&dest, &number));
  testrun(*(uint64_t *)dest == 123);

  free(dest);

  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_data_uint64_dump() {

  uint64_t number = 123;

  testrun(!dtn_data_uint64_dump(NULL, NULL));
  testrun(!dtn_data_uint64_dump(stdout, NULL));
  testrun(!dtn_data_uint64_dump(NULL, (void *)&number));

  testrun_log("TEST DUMP FOLLOWS");
  testrun(dtn_data_uint64_dump(stdout, (void *)&number));

  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_data_int64_data_functions() {

  dtn_data_function function = dtn_data_int64_data_functions();

  testrun(function.clear == dtn_data_int64_clear);
  testrun(function.free == dtn_data_int64_free);
  testrun(function.copy == dtn_data_int64_copy);
  testrun(function.dump == dtn_data_int64_dump);

  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_data_uint64_data_functions() {

  dtn_data_function function = dtn_data_uint64_data_functions();

  testrun(function.clear == dtn_data_uint64_clear);
  testrun(function.free == dtn_data_uint64_free);
  testrun(function.copy == dtn_data_uint64_copy);
  testrun(function.dump == dtn_data_uint64_dump);

  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_data_timeval_clear() {

  struct timeval self;

  testrun(0 == gettimeofday(&self, NULL));

  testrun(self.tv_sec != 0)

      testrun(!dtn_data_timeval_clear(NULL));
  testrun(dtn_data_timeval_clear((void *)&self));

  testrun(self.tv_sec == 0);
  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_data_timeval_free() {

  struct timeval *self = calloc(1, sizeof(struct timeval));
  testrun(0 == gettimeofday(self, NULL));

  testrun(NULL == dtn_data_timeval_free(NULL));
  testrun(NULL == dtn_data_timeval_free(self));

  // check valgrind
  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_data_timeval_copy() {

  struct timeval self;
  void *dest = NULL;

  testrun(0 == gettimeofday(&self, NULL));

  testrun(!dtn_data_timeval_copy(NULL, NULL));
  testrun(!dtn_data_timeval_copy((void **)&dest, NULL));
  testrun(!dtn_data_timeval_copy(NULL, &self));

  testrun(dtn_data_timeval_copy((void **)&dest, &self));
  testrun(dest) testrun(0 == memcmp(dest, &self, sizeof(struct timeval)));

  free(dest);

  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_data_timeval_dump() {

  struct timeval self;

  testrun(!dtn_data_timeval_dump(NULL, NULL));
  testrun(!dtn_data_timeval_dump(stdout, NULL));
  testrun(!dtn_data_timeval_dump(NULL, (void *)&self));

  testrun_log("TEST DUMP FOLLOWS");

  testrun(0 == gettimeofday(&self, NULL));
  testrun(dtn_data_timeval_dump(stdout, (void *)&self));

  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_data_timeval_data_functions() {

  dtn_data_function function = dtn_data_timeval_data_functions();

  testrun(function.clear == dtn_data_timeval_clear);
  testrun(function.free == dtn_data_timeval_free);
  testrun(function.copy == dtn_data_timeval_copy);
  testrun(function.dump == dtn_data_timeval_dump);

  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_data_pointer_free() {

  void *self = calloc(1, sizeof(struct timeval));

  testrun(NULL == dtn_data_timeval_free(NULL));
  testrun(NULL == dtn_data_timeval_free(self));

  // check valgrind
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
  testrun_test(test_dtn_data_function_create);
  testrun_test(test_dtn_data_function_clear);
  testrun_test(test_dtn_data_function_free);
  testrun_test(test_dtn_data_function_copy);
  testrun_test(test_dtn_data_function_dump);

  testrun_test(test_dtn_data_string_clear);
  testrun_test(test_dtn_data_string_free);
  testrun_test(test_dtn_data_string_copy);
  testrun_test(test_dtn_data_string_dump);

  testrun_test(test_dtn_data_int64_clear);
  testrun_test(test_dtn_data_int64_free);
  testrun_test(test_dtn_data_int64_copy);
  testrun_test(test_dtn_data_int64_dump);

  testrun_test(test_dtn_data_int64_direct_copy);
  testrun_test(test_dtn_data_int64_direct_dump);

  testrun_test(test_dtn_data_function_wrapper_free);

  testrun_test(test_dtn_data_string_data_functions);
  testrun_test(test_dtn_data_string_data_functions_are_valid);

  testrun_test(test_dtn_data_function_allocated);

  testrun_test(test_dtn_data_uint64_clear);
  testrun_test(test_dtn_data_uint64_free);
  testrun_test(test_dtn_data_uint64_copy);
  testrun_test(test_dtn_data_uint64_dump);
  testrun_test(test_dtn_data_uint64_data_functions);
  testrun_test(test_dtn_data_int64_data_functions);

  testrun_test(test_dtn_data_timeval_clear);
  testrun_test(test_dtn_data_timeval_copy);
  testrun_test(test_dtn_data_timeval_dump);
  testrun_test(test_dtn_data_timeval_free);
  testrun_test(test_dtn_data_timeval_data_functions);

  testrun_test(test_dtn_data_pointer_free);

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
