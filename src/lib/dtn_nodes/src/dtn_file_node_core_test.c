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
        @file           dtn_file_node_core_test.c
        @author         Töpfer, Markus

        @date           2025-12-23


        ------------------------------------------------------------------------
*/
#include "dtn_file_node_core.c"
#include <dtn_base/testrun.h>

#include <dtn_base/dtn_random.h>

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

int test_dtn_file_node_core_create() {

    dtn_event_loop_config loop_config = (dtn_event_loop_config){
        .max.sockets = dtn_socket_get_max_supported_runtime_sockets(0),
        .max.timers = dtn_socket_get_max_supported_runtime_sockets(0)};

    dtn_event_loop *loop = dtn_event_loop_default(loop_config);
    testrun(loop);

    dtn_file_node_core *core =
        dtn_file_node_core_create((dtn_file_node_core_config){.loop = loop});

    testrun(core);
    testrun(dtn_file_node_core_cast(core));
    testrun(core->garbadge);
    testrun(core->tloop);
    testrun(core->interfaces.ip);

    testrun(NULL == dtn_file_node_core_free(core));
    testrun(NULL == dtn_event_loop_free(loop));

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_file_node_core_enable_ip_interfaces() {

    dtn_event_loop_config loop_config = (dtn_event_loop_config){
        .max.sockets = dtn_socket_get_max_supported_runtime_sockets(0),
        .max.timers = dtn_socket_get_max_supported_runtime_sockets(0)};

    dtn_event_loop *loop = dtn_event_loop_default(loop_config);
    testrun(loop);

    dtn_file_node_core *core =
        dtn_file_node_core_create((dtn_file_node_core_config){.loop = loop});

    testrun(core);

    dtn_item *conf = dtn_item_json_read_file(DTN_TEST_RESOURCE_DIR
                                             "/config/default_config.json");
    testrun(dtn_file_node_core_enable_ip_interfaces(core, conf));
    testrun(1 == dtn_dict_count(core->interfaces.ip));

    testrun(NULL == dtn_item_free(conf));
    testrun(NULL == dtn_file_node_core_free(core));
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
    testrun_test(test_dtn_file_node_core_create);
    testrun_test(test_dtn_file_node_core_enable_ip_interfaces);

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
