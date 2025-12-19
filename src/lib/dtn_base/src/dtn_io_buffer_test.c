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
        @file           dtn_io_buffer_test.c
        @author         Töpfer, Markus

        @date           2025-12-19


        ------------------------------------------------------------------------
*/
#include <dtn_base/testrun.h>
#include "dtn_io_buffer.c"

/*
 *      ------------------------------------------------------------------------
 *
 *      TEST CASES                                                      #CASES
 *
 *      ------------------------------------------------------------------------
 */

int test_dtn_io_buffer_create(){
    
    dtn_io_buffer *self = dtn_io_buffer_create((dtn_io_buffer_config){0});
    testrun(self);
    testrun(self->dict);
    testrun(NULL == dtn_io_buffer_free(self));

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_io_buffer_free(){
    
    dtn_io_buffer *self = dtn_io_buffer_create((dtn_io_buffer_config){0});
    testrun(self);
    testrun(self->dict);

    dtn_socket_data remote = {0};

    testrun(dtn_io_buffer_push(self, &remote, (uint8_t*) "test", 4));

    testrun(NULL == dtn_io_buffer_free(self));

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_io_buffer_push(){
    
    dtn_io_buffer *self = dtn_io_buffer_create((dtn_io_buffer_config){0});
    testrun(self);
    testrun(self->dict);

    dtn_socket_data remote = {0};

    testrun(dtn_io_buffer_push(self, &remote, (uint8_t*) "test", 4));
    dtn_buffer *buffer = dtn_io_buffer_pop(self, &remote);
    testrun(buffer);
    testrun(0 == memcmp("test", buffer->start, buffer->length));
    testrun(NULL == dtn_buffer_free(buffer));

    testrun(NULL == dtn_io_buffer_free(self));

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_io_buffer_pop(){
    
    dtn_io_buffer *self = dtn_io_buffer_create((dtn_io_buffer_config){0});
    testrun(self);
    testrun(self->dict);

    dtn_socket_data remote = {0};

    testrun(dtn_io_buffer_push(self, &remote, (uint8_t*) "test", 4));
    dtn_buffer *buffer = dtn_io_buffer_pop(self, &remote);
    testrun(buffer);
    testrun(0 == memcmp("test", buffer->start, buffer->length));
    testrun(NULL == dtn_buffer_free(buffer));

    testrun(NULL == dtn_io_buffer_free(self));

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
    testrun_test(test_dtn_io_buffer_create);
    testrun_test(test_dtn_io_buffer_free);
    testrun_test(test_dtn_io_buffer_push);
    testrun_test(test_dtn_io_buffer_pop);

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
