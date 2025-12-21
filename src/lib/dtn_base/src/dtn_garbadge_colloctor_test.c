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
        @file           dtn_garbadge_colloctor_test.c
        @author         Töpfer, Markus

        @date           2025-12-21


        ------------------------------------------------------------------------
*/
#include "../include/testrun.h"
#include "dtn_garbadge_colloctor.c"

#include "../include/dtn_string.h"
#include <unistd.h>

/*
 *      ------------------------------------------------------------------------
 *
 *      TEST CASES                                                      #CASES
 *
 *      ------------------------------------------------------------------------
 */

int test_dtn_garbadge_colloctor_create(){
    
    dtn_event_loop *loop = dtn_event_loop_default(
        (dtn_event_loop_config){.max.sockets = 100, .max.timers = 100});
    
    testrun(loop);

    dtn_garbadge_colloctor_config config = (dtn_garbadge_colloctor_config){
        .loop = loop
    };

    dtn_garbadge_colloctor *self = dtn_garbadge_colloctor_create(config);
    testrun(self);
    testrun(self->data.queue);
    testrun(self->tloop);
    testrun(DTN_TIMER_INVALID != self->timer.collect);

    testrun(NULL == dtn_garbadge_colloctor_free(self));
    testrun(NULL == dtn_event_loop_free(loop));

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_garbadge_colloctor_free(){
    
    dtn_event_loop *loop = dtn_event_loop_default(
        (dtn_event_loop_config){.max.sockets = 100, .max.timers = 100});
    
    testrun(loop);

    dtn_garbadge_colloctor_config config = (dtn_garbadge_colloctor_config){
        .loop = loop
    };

    dtn_garbadge_colloctor *self = dtn_garbadge_colloctor_create(config);
    
    char *string = dtn_string_dup("test");

    testrun(dtn_garbadge_colloctor_push(self, string, dtn_data_pointer_free));

    testrun(NULL == dtn_garbadge_colloctor_free(self));
    testrun(NULL == dtn_event_loop_free(loop));

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_garbadge_colloctor_push(){
    
    dtn_event_loop *loop = dtn_event_loop_default(
        (dtn_event_loop_config){.max.sockets = 100, .max.timers = 100});
    
    testrun(loop);

    dtn_garbadge_colloctor_config config = (dtn_garbadge_colloctor_config){
        .loop = loop
    };

    dtn_garbadge_colloctor *self = dtn_garbadge_colloctor_create(config);
    
    char *string = dtn_string_dup("test");

    testrun(!dtn_garbadge_colloctor_push(NULL, string, dtn_data_pointer_free));
    testrun(!dtn_garbadge_colloctor_push(self, NULL, dtn_data_pointer_free));
    testrun(!dtn_garbadge_colloctor_push(self, string, NULL));

    testrun(dtn_garbadge_colloctor_push(self, string, dtn_data_pointer_free));
    testrun(1 == dtn_list_count(self->data.queue));

    testrun(NULL == dtn_garbadge_colloctor_free(self));
    testrun(NULL == dtn_event_loop_free(loop));

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int check_garbadge_collection(){
    
    dtn_event_loop *loop = dtn_event_loop_default(
        (dtn_event_loop_config){.max.sockets = 100, .max.timers = 100});
    
    testrun(loop);

    dtn_garbadge_colloctor_config config = (dtn_garbadge_colloctor_config){
        .loop = loop,
        .limits.run_cleanup_usec = 10000
    };

    dtn_garbadge_colloctor *self = dtn_garbadge_colloctor_create(config);
    
    char *string = dtn_string_dup("test");

    testrun(dtn_garbadge_colloctor_push(self, string, dtn_data_pointer_free));
    testrun(1 == dtn_list_count(self->data.queue));

    // let the loop run twice to ensure the trigger is sent
    usleep(20000);
    testrun(dtn_event_loop_run(loop, DTN_RUN_ONCE));
    usleep(20000);
    testrun(dtn_event_loop_run(loop, DTN_RUN_ONCE));

    testrun(0 == dtn_list_count(self->data.queue));

    testrun(NULL == dtn_garbadge_colloctor_free(self));
    testrun(NULL == dtn_event_loop_free(loop));

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
    testrun_test(test_dtn_garbadge_colloctor_create);
    testrun_test(test_dtn_garbadge_colloctor_free);
    testrun_test(test_dtn_garbadge_colloctor_push);
    testrun_test(check_garbadge_collection);

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
