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
        @file           dtn_sdnv_test.c
        @author         Töpfer, Markus

        @date           2025-12-08


        ------------------------------------------------------------------------
*/
#include <dtn_base/testrun.h>
#include "dtn_sdnv.c"

#include <dtn_base/dtn_dump.h>

/*
 *      ------------------------------------------------------------------------
 *
 *      TEST CASES                                                      #CASES
 *
 *      ------------------------------------------------------------------------
 */

int test_dtn_sdnv_decode(){
    
    uint8_t input[10] = {0};

    input[0] = 0x95;
    input[1] = 0x3C;
    input[2] = 0x00;

    uint64_t expect = 0xabc;

    uint64_t out = 0;
    uint8_t *next = NULL;

    testrun(dtn_sdnv_decode(input, 2, &out, &next));
    testrun(out == expect);
    testrun(next == input + 2);

    input[0] = 0xA4;
    input[1] = 0x34;
    input[2] = 0x00;

    expect = 0x1234;

    testrun(dtn_sdnv_decode(input, 2, &out, &next));
    testrun(out == expect);
    testrun(next == input + 2);

    input[0] = 0x81;
    input[1] = 0x84;
    input[2] = 0x34;

    expect = 0x4234;

    testrun(dtn_sdnv_decode(input, 3, &out, &next));
    testrun(out == expect);
    testrun(next == input + 3);

    input[0] = 0x7F;

    expect = 0x7F;

    testrun(dtn_sdnv_decode(input, 1, &out, &next));
    testrun(out == expect);
    testrun(next == input + 1);

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_sdnv_encode(){
    
    uint8_t buffer[10] = {0};
    uint8_t *next = NULL;

    memset(buffer, 0, 10);
    testrun(dtn_sdnv_encode(0xabc, buffer, 10, &next));
    testrun(next == buffer + 2);

    //dtn_dump_binary_as_hex(stdout, buffer, 5);

    testrun(buffer[0] == 0x95);
    testrun(buffer[1] == 0x3C);
    testrun(buffer[2] == 0x00);

    memset(buffer, 0, 10);
    testrun(dtn_sdnv_encode(0x1234, buffer, 10, &next));
    testrun(next == buffer + 2);

    //dtn_dump_binary_as_hex(stdout, buffer, 5);

    testrun(buffer[0] == 0xA4);
    testrun(buffer[1] == 0x34);
    testrun(buffer[2] == 0x00);
    testrun(buffer[3] == 0x00);

    memset(buffer, 0, 10);
    testrun(dtn_sdnv_encode(0x4234, buffer, 10, &next));
    testrun(next == buffer + 3);

    //dtn_dump_binary_as_hex(stdout, buffer, 5);

    testrun(buffer[0] == 0x81);
    testrun(buffer[1] == 0x84);
    testrun(buffer[2] == 0x34);
    testrun(buffer[3] == 0x00);

    memset(buffer, 0, 10);
    testrun(dtn_sdnv_encode(0x7F, buffer, 10, &next));
    testrun(next == buffer + 1);

    testrun(buffer[0] == 0x7F);
    testrun(buffer[1] == 0x00);
    testrun(buffer[2] == 0x00);
    testrun(buffer[3] == 0x00);

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
    testrun_test(test_dtn_sdnv_decode);
    testrun_test(test_dtn_sdnv_encode);

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
