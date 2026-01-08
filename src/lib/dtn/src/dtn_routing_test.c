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
        @file           dtn_routing_test.c
        @author         Töpfer, Markus

        @date           2025-12-21


        ------------------------------------------------------------------------
*/
#include "dtn_routing.c"
#include <dtn_base/testrun.h>

#ifndef DTN_TEST_RESOURCE_DIR
#error "Must provide -D DTN_TEST_RESOURCE_DIR=value while compiling this file."
#endif

/*
 *      ------------------------------------------------------------------------
 *
 *      TEST CASES                                                      #CASES
 *
 *      ------------------------------------------------------------------------
 */

int test_dtn_routing_create() {

    dtn_event_loop_config loop_config =
        (dtn_event_loop_config){.max.sockets = 100, .max.timers = 100};

    dtn_event_loop *loop = dtn_event_loop_default(loop_config);
    testrun(loop);

    dtn_routing_config config = (dtn_routing_config){
        .loop = loop, .route_config_path = DTN_TEST_RESOURCE_DIR "/routes"};

    dtn_routing *self = dtn_routing_create(config);
    testrun(self);
    testrun(self->routes.data);

    testrun(NULL == dtn_routing_free(self));
    testrun(NULL == dtn_event_loop_free(loop));

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_routing_free() {

    dtn_event_loop_config loop_config =
        (dtn_event_loop_config){.max.sockets = 100, .max.timers = 100};

    dtn_event_loop *loop = dtn_event_loop_default(loop_config);
    testrun(loop);

    dtn_routing_config config = (dtn_routing_config){
        .loop = loop, .route_config_path = DTN_TEST_RESOURCE_DIR "/routes"};

    dtn_routing *self = dtn_routing_create(config);
    testrun(self);
    testrun(self->routes.data);

    testrun(NULL == dtn_routing_free(self));
    testrun(NULL == dtn_event_loop_free(loop));

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_routing_dump() {

    dtn_event_loop_config loop_config =
        (dtn_event_loop_config){.max.sockets = 100, .max.timers = 100};

    dtn_event_loop *loop = dtn_event_loop_default(loop_config);
    testrun(loop);

    dtn_routing_config config = (dtn_routing_config){
        .loop = loop, .route_config_path = DTN_TEST_RESOURCE_DIR "/routes"};

    dtn_routing *self = dtn_routing_create(config);

    testrun(dtn_routing_dump(stderr, self));

    testrun(NULL == dtn_routing_free(self));
    testrun(NULL == dtn_event_loop_free(loop));

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_routing_save() {

    dtn_event_loop_config loop_config =
        (dtn_event_loop_config){.max.sockets = 100, .max.timers = 100};

    dtn_event_loop *loop = dtn_event_loop_default(loop_config);
    testrun(loop);

    dtn_routing_config config = (dtn_routing_config){
        .loop = loop, .route_config_path = DTN_TEST_RESOURCE_DIR "/routes"};

    dtn_routing *self = dtn_routing_create(config);

    testrun(dtn_routing_save(self, NULL));
    testrun(dtn_routing_save(self, DTN_TEST_RESOURCE_DIR "/test"));

    testrun(NULL == dtn_routing_free(self));
    testrun(NULL == dtn_event_loop_free(loop));

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_routing_load() {

    dtn_event_loop_config loop_config =
        (dtn_event_loop_config){.max.sockets = 100, .max.timers = 100};

    dtn_event_loop *loop = dtn_event_loop_default(loop_config);
    testrun(loop);

    dtn_routing_config config = (dtn_routing_config){
        .loop = loop, .route_config_path = DTN_TEST_RESOURCE_DIR "/routes"};

    dtn_routing *self = dtn_routing_create(config);

    testrun(dtn_routing_load(self, NULL));
    testrun(dtn_routing_load(self, DTN_TEST_RESOURCE_DIR "/test"));

    testrun(NULL == dtn_routing_free(self));
    testrun(NULL == dtn_event_loop_free(loop));

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
    testrun_test(test_dtn_routing_create);
    testrun_test(test_dtn_routing_free);
    testrun_test(test_dtn_routing_dump);
    testrun_test(test_dtn_routing_save);
    testrun_test(test_dtn_routing_load);

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
