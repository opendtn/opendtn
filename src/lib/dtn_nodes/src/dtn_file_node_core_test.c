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
#include <dtn_base/testrun.h>
#include "dtn_file_node_core.c"

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

int test_dtn_file_node_core_create(){
    
    dtn_event_loop_config loop_config = (dtn_event_loop_config){
        .max.sockets = dtn_socket_get_max_supported_runtime_sockets(0),
        .max.timers = dtn_socket_get_max_supported_runtime_sockets(0)};

    dtn_event_loop *loop = dtn_event_loop_default(loop_config);
    testrun(loop);

    dtn_file_node_core *core = dtn_file_node_core_create(
        (dtn_file_node_core_config){
            .loop = loop
        });

    testrun(core);
    testrun(dtn_file_node_core_cast(core));
    testrun(core->garbadge);
    testrun(core->tloop);
    testrun(!core->routes.data);
    testrun(core->interfaces.ip);

    testrun(NULL == dtn_file_node_core_free(core));
    testrun(NULL == dtn_event_loop_free(loop));

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_file_node_core_enable_ip_interfaces(){
    
    dtn_event_loop_config loop_config = (dtn_event_loop_config){
        .max.sockets = dtn_socket_get_max_supported_runtime_sockets(0),
        .max.timers = dtn_socket_get_max_supported_runtime_sockets(0)};

    dtn_event_loop *loop = dtn_event_loop_default(loop_config);
    testrun(loop);

    dtn_file_node_core *core = dtn_file_node_core_create(
        (dtn_file_node_core_config){
            .loop = loop
        });

    testrun(core);

    dtn_item *conf = dtn_item_json_read_file(DTN_TEST_RESOURCE_DIR"/config/default_config.json");
    testrun(dtn_file_node_core_enable_ip_interfaces(core, conf));
    testrun(1 == dtn_dict_count(core->interfaces.ip));

    testrun(NULL == dtn_item_free(conf));
    testrun(NULL == dtn_file_node_core_free(core));
    testrun(NULL == dtn_event_loop_free(loop));

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_file_node_core_enable_routes(){
    
    dtn_event_loop_config loop_config = (dtn_event_loop_config){
        .max.sockets = dtn_socket_get_max_supported_runtime_sockets(0),
        .max.timers = dtn_socket_get_max_supported_runtime_sockets(0)};

    dtn_event_loop *loop = dtn_event_loop_default(loop_config);
    testrun(loop);

    dtn_file_node_core *core = dtn_file_node_core_create(
        (dtn_file_node_core_config){
            .loop = loop
        });

    testrun(core);
    testrun(dtn_file_node_core_enable_routes(core, 
        DTN_TEST_RESOURCE_DIR"/config/routes"));
    testrun(0 < dtn_item_count(core->routes.data));

    testrun(NULL == dtn_file_node_core_free(core));
    testrun(NULL == dtn_event_loop_free(loop));

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int check_get_default_interface(){
    
    dtn_event_loop_config loop_config = (dtn_event_loop_config){
        .max.sockets = dtn_socket_get_max_supported_runtime_sockets(0),
        .max.timers = dtn_socket_get_max_supported_runtime_sockets(0)};

    dtn_event_loop *loop = dtn_event_loop_default(loop_config);
    testrun(loop);

    dtn_file_node_core *core = dtn_file_node_core_create(
        (dtn_file_node_core_config){
            .loop = loop
        });

    testrun(core);
    testrun(dtn_file_node_core_enable_routes(core, 
        DTN_TEST_RESOURCE_DIR"/config/routes"));
    testrun(0 < dtn_item_count(core->routes.data));
    dtn_item *conf = dtn_item_json_read_file(DTN_TEST_RESOURCE_DIR"/config/default_config.json");
    testrun(dtn_file_node_core_enable_ip_interfaces(core, conf));
    testrun(1 == dtn_dict_count(core->interfaces.ip));
    testrun(NULL == dtn_item_free(conf));

    dtn_interface_ip *ip = get_default_interface(core);
    testrun(ip);

    testrun(NULL == dtn_file_node_core_free(core));
    testrun(NULL == dtn_event_loop_free(loop));

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int check_get_socket_config_for_uri(){
    
    dtn_event_loop_config loop_config = (dtn_event_loop_config){
        .max.sockets = dtn_socket_get_max_supported_runtime_sockets(0),
        .max.timers = dtn_socket_get_max_supported_runtime_sockets(0)};

    dtn_event_loop *loop = dtn_event_loop_default(loop_config);
    testrun(loop);

    dtn_file_node_core *core = dtn_file_node_core_create(
        (dtn_file_node_core_config){
            .loop = loop
        });

    testrun(core);
    testrun(dtn_file_node_core_enable_routes(core, 
        DTN_TEST_RESOURCE_DIR"/config/routes"));
    testrun(0 < dtn_item_count(core->routes.data));
    dtn_item *conf = dtn_item_json_read_file(DTN_TEST_RESOURCE_DIR"/config/default_config.json");
    testrun(dtn_file_node_core_enable_ip_interfaces(core, conf));
    testrun(1 == dtn_dict_count(core->interfaces.ip));
    testrun(NULL == dtn_item_free(conf));

    dtn_socket_configuration c = get_socket_config_for_uri(core, "test/one");
    testrun(0 == dtn_string_compare(c.host, "127.0.0.1"));
    testrun(4556 == c.port);

    c = get_socket_config_for_uri(core, "test/two");
    testrun(0 == dtn_string_compare(c.host, "127.0.0.1"));
    testrun(4557 == c.port);


    testrun(NULL == dtn_file_node_core_free(core));
    testrun(NULL == dtn_event_loop_free(loop));

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int check_get_interface_for_uri(){
    
    dtn_event_loop_config loop_config = (dtn_event_loop_config){
        .max.sockets = dtn_socket_get_max_supported_runtime_sockets(0),
        .max.timers = dtn_socket_get_max_supported_runtime_sockets(0)};

    dtn_event_loop *loop = dtn_event_loop_default(loop_config);
    testrun(loop);

    dtn_file_node_core *core = dtn_file_node_core_create(
        (dtn_file_node_core_config){
            .loop = loop
        });

    testrun(core);
    testrun(dtn_file_node_core_enable_routes(core, 
        DTN_TEST_RESOURCE_DIR"/config/routes"));
    testrun(0 < dtn_item_count(core->routes.data));
    dtn_item *conf = dtn_item_json_read_file(DTN_TEST_RESOURCE_DIR"/config/default_config.json");
    testrun(dtn_file_node_core_enable_ip_interfaces(core, conf));
    testrun(1 == dtn_dict_count(core->interfaces.ip));
    testrun(NULL == dtn_item_free(conf));

    dtn_socket_configuration c = get_socket_config_for_uri(core, "test/one");
    testrun(0 == dtn_string_compare(c.host, "127.0.0.1"));
    testrun(4556 == c.port);

    dtn_interface_ip *in = get_interface_for_uri(core, "test/one", c);
    testrun(in);
    testrun(0 == dtn_string_compare("127.0.0.1", dtn_interface_ip_name(in)));

    c = get_socket_config_for_uri(core, "test/two");
    testrun(0 == dtn_string_compare(c.host, "127.0.0.1"));
    testrun(4557 == c.port);

    in = get_interface_for_uri(core, "test/two", c);
    testrun(in);
    testrun(0 == dtn_string_compare("127.0.0.1", dtn_interface_ip_name(in)));

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
    testrun_test(test_dtn_file_node_core_enable_routes);
    testrun_test(check_get_default_interface);
    testrun_test(check_get_socket_config_for_uri);
    testrun_test(check_get_interface_for_uri);

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

