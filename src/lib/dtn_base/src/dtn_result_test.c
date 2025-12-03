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
        See the License for the specific language gdtnerning permissions and
        limitations under the License.

        This file is part of the openvocs project. https://openvocs.org

        ------------------------------------------------------------------------
*//**

        @author         Michael J. Beer

        ------------------------------------------------------------------------
*/

#include "dtn_result.c"

#include "../include/testrun.h"

/*----------------------------------------------------------------------------*/

static int test_dtn_result_set() {

  dtn_result result = {0};

  testrun(!dtn_result_set(0, 0, 0));
  testrun(dtn_result_set(&result, DTN_ERROR_NO_ERROR, 0));

  testrun(DTN_ERROR_NO_ERROR == result.error_code);
  testrun(0 == result.message);

  testrun(!dtn_result_set(0, 100, 0));
  testrun(!dtn_result_set(&result, 100, 0));

  char const *TEST_MSG = "fensalir";

  testrun(dtn_result_set(&result, 100, TEST_MSG));

  testrun(100 == result.error_code);
  testrun(0 != result.message);
  testrun(0 == strcmp(TEST_MSG, result.message));

  testrun(dtn_result_set(&result, 200,
                        "200"));
  testrun(200 == result.error_code);
  testrun(0 == strcmp("200", result.message));

  dtn_result_clear(&result);

  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

static int test_dtn_result_get_message() {

  dtn_result result = {0};

  testrun(0 != dtn_result_get_message(result));

  dtn_result_set(&result, DTN_ERROR_NO_ERROR, 0);
  testrun(0 != dtn_result_get_message(result));

  char const *MESSAGE = "Geir nu garmr mjoek fyr gnipahellir";

  testrun(dtn_result_set(&result, 100, MESSAGE));
  testrun(0 == strcmp(MESSAGE, dtn_result_get_message(result)));

  dtn_result_clear(&result);

  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

static int test_dtn_result_clear() {

  testrun(!dtn_result_clear(0));

  dtn_result result = {0};

  testrun(dtn_result_clear(&result));

  testrun(0 == result.error_code);
  testrun(0 == result.message);

  result.message = strdup("himinbjoerg");
  result.error_code = 1243;

  testrun(dtn_result_clear(&result));

  testrun(0 == result.error_code);
  testrun(0 == result.message);

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
  testrun_test(test_dtn_result_set);
  testrun_test(test_dtn_result_get_message);
  testrun_test(test_dtn_result_clear);

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
