/***
        ------------------------------------------------------------------------

        Copyright (c) 2018 German Aerospace Center DLR e.V. (GSOC)

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
        @file           dtn_match_functions_test.c
        @author         Michael Beer
        @author         Markus Toepfer

        @date           2018-08-15

        @ingroup        dtn_data_structures

        @brief          Unit tests of


        ------------------------------------------------------------------------
*/
#include "../include/dtn_socket.h"
#include "../include/testrun.h"
#include "dtn_match_functions.c"

/*
 *      ------------------------------------------------------------------------
 *
 *      TEST CASES                                                      #CASES
 *
 *      ------------------------------------------------------------------------
 */

int test_dtn_match_strict() {

    char buffer1[10] = {0};
    char buffer2[10] = {0};

    strcat(buffer1, "test");
    strcat(buffer2, "test");
    testrun(!dtn_match_strict(NULL, NULL));
    testrun(!dtn_match_strict(buffer1, NULL));
    testrun(!dtn_match_strict(NULL, buffer2));

    testrun(dtn_match_strict(buffer1, buffer2));

    buffer1[0] = 'T';
    testrun(!dtn_match_strict(buffer1, buffer2));
    buffer2[0] = 'T';
    testrun(dtn_match_strict(buffer1, buffer2));

    buffer2[4] = 's';
    testrun(!dtn_match_strict(buffer1, buffer2));
    buffer1[4] = 's';
    testrun(dtn_match_strict(buffer1, buffer2));

    buffer1[5] = '1';
    testrun(!dtn_match_strict(buffer1, buffer2));

    buffer2[5] = '1';
    testrun(dtn_match_strict(buffer1, buffer2));

    buffer2[5] = 0xFF;
    testrun(!dtn_match_strict(buffer1, buffer2));
    buffer1[5] = 0xFF;
    testrun(dtn_match_strict(buffer1, buffer2));

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_match_c_string_strict() {

    char buffer1[10] = {0};
    char buffer2[10] = {0};

    strcat(buffer1, "test");
    strcat(buffer2, "test");
    testrun(dtn_match_c_string_strict(NULL, NULL));
    testrun(!dtn_match_c_string_strict(buffer1, NULL));
    testrun(!dtn_match_c_string_strict(NULL, buffer2));

    testrun(dtn_match_c_string_strict(buffer1, buffer2));

    buffer1[0] = 'T';
    testrun(!dtn_match_c_string_strict(buffer1, buffer2));
    buffer2[0] = 'T';
    testrun(dtn_match_c_string_strict(buffer1, buffer2));

    buffer2[4] = 's';
    testrun(!dtn_match_c_string_strict(buffer1, buffer2));
    buffer1[4] = 's';
    testrun(dtn_match_c_string_strict(buffer1, buffer2));

    buffer1[5] = '1';
    testrun(!dtn_match_c_string_strict(buffer1, buffer2));

    buffer2[5] = '1';
    testrun(dtn_match_c_string_strict(buffer1, buffer2));

    buffer2[5] = 0xFF;
    testrun(!dtn_match_c_string_strict(buffer1, buffer2));
    buffer1[5] = 0xFF;
    testrun(dtn_match_c_string_strict(buffer1, buffer2));

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_match_c_string_case_ignore_strict() {

    char buffer1[10] = {0};
    char buffer2[10] = {0};

    strcat(buffer1, "test");
    strcat(buffer2, "test");
    testrun(dtn_match_c_string_case_ignore_strict(NULL, NULL));
    testrun(!dtn_match_c_string_case_ignore_strict(buffer1, NULL));
    testrun(!dtn_match_c_string_case_ignore_strict(NULL, buffer2));

    testrun(dtn_match_c_string_case_ignore_strict(buffer1, buffer2));

    buffer1[0] = 'T';
    testrun(dtn_match_c_string_case_ignore_strict(buffer1, buffer2));
    buffer2[0] = 'T';
    testrun(dtn_match_c_string_case_ignore_strict(buffer1, buffer2));

    buffer2[4] = 's';
    testrun(!dtn_match_c_string_case_ignore_strict(buffer1, buffer2));
    buffer1[4] = 'S';
    testrun(dtn_match_c_string_case_ignore_strict(buffer1, buffer2));

    buffer1[5] = '1';
    testrun(!dtn_match_c_string_case_ignore_strict(buffer1, buffer2));

    buffer2[5] = '1';
    testrun(dtn_match_c_string_case_ignore_strict(buffer1, buffer2));

    buffer2[5] = 0xFF;
    testrun(!dtn_match_c_string_case_ignore_strict(buffer1, buffer2));
    buffer1[5] = 0xFF;
    testrun(dtn_match_c_string_case_ignore_strict(buffer1, buffer2));

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_match_intptr() {

    intptr_t ptr1 = 123;
    intptr_t ptr2 = 123;

    testrun(dtn_match_intptr(NULL, NULL));
    testrun(!dtn_match_intptr((void *)ptr1, NULL));
    testrun(!dtn_match_intptr(NULL, (void *)ptr2));

    testrun(dtn_match_intptr((void *)ptr1, (void *)ptr2));

    ptr1 = 1;
    testrun(!dtn_match_intptr((void *)ptr1, (void *)ptr2));
    ptr2 = 1;
    testrun(dtn_match_intptr((void *)ptr1, (void *)ptr2));

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_match_uint64() {

    uint64_t ptr1 = 123;
    uint64_t ptr2 = 123;

    testrun(!dtn_match_uint64(NULL, NULL));
    testrun(!dtn_match_uint64(&ptr1, NULL));
    testrun(!dtn_match_uint64(NULL, &ptr2));

    testrun(dtn_match_uint64(&ptr1, &ptr2));

    ptr1 = 1;
    testrun(!dtn_match_uint64(&ptr1, &ptr2));
    ptr2 = 1;
    testrun(dtn_match_uint64(&ptr1, &ptr2));

    // check max
    ptr1 = 0xFFFFFFFFFFFFFFFF;
    testrun(!dtn_match_uint64(&ptr1, &ptr2));
    ptr2 = 0xFFFFFFFFFFFFFFFF;
    testrun(dtn_match_uint64(&ptr1, &ptr2));

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_match_dtn_socket_data() {

    dtn_socket_data one = {0};
    dtn_socket_data two = {0};

    testrun(dtn_match_dtn_socket_data(&one, &two));
    one.port = 1;
    testrun(!dtn_match_dtn_socket_data(&one, &two));
    two.port = 1;
    testrun(dtn_match_dtn_socket_data(&one, &two));
    strcat(one.host, "0.0.0.0");
    testrun(!dtn_match_dtn_socket_data(&one, &two));
    strcat(two.host, "0.0.0.0");
    testrun(dtn_match_dtn_socket_data(&one, &two));

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

    testrun_test(test_dtn_match_c_string_strict);
    testrun_test(test_dtn_match_c_string_case_ignore_strict);
    testrun_test(test_dtn_match_intptr);
    testrun_test(test_dtn_match_uint64);
    testrun_test(test_dtn_match_dtn_socket_data);

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
