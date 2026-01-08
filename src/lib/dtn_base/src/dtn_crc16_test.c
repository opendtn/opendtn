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

        This file is part of the opendtn project. https://opendtn.com

        ------------------------------------------------------------------------
*//**
        @file           dtn_crc16_test.c
        @author         Töpfer, Markus

        @date           2025-12-16


        ------------------------------------------------------------------------
*/
#include "../include/testrun.h"
#include "dtn_crc16.c"

/*
 *      ------------------------------------------------------------------------
 *
 *      TEST CASES                                                      #CASES
 *
 *      ------------------------------------------------------------------------
 */

int test_crc16x25() {

    uint8_t buffer[0xff] = {0};

    buffer[0] = 0x28;
    buffer[1] = 0x64;
    buffer[2] = 0x43;
    buffer[3] = 0x43;
    buffer[4] = 0x97;

    uint16_t expext = 0x7BE4;
    uint16_t result = crc16x25(buffer, 5);
    testrun(expext == result);

    buffer[0] = 0x12;
    buffer[1] = 0x34;
    buffer[2] = 0x56;
    buffer[3] = 0x78;
    buffer[4] = 0x00;

    expext = 0x9B2E;
    result = crc16x25(buffer, 4);
    testrun(expext == result);

    buffer[0] = 0xab;
    buffer[1] = 0xfd;
    buffer[2] = 0xef;
    buffer[3] = 0x12;
    buffer[4] = 0x00;

    expext = 0xB47F;
    result = crc16x25(buffer, 4);
    testrun(expext == result);

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
    testrun_test(test_crc16x25);

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
