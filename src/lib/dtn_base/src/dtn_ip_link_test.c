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
        @file           dtn_ip_link_test.c
        @author         Töpfer, Markus

        @date           2025-12-19


        ------------------------------------------------------------------------
*/
#include "dtn_ip_link.c"
#include <dtn_base/testrun.h>

#include "../include/dtn_item_json.h"

/*
 *      ------------------------------------------------------------------------
 *
 *      TEST CASES                                                      #CASES
 *
 *      ------------------------------------------------------------------------
 */

int test_dtn_ip_link_get_interface_name() {

    dtn_socket_configuration config = dtn_socket_load_dynamic_port(
        (dtn_socket_configuration){.host = "localhost", .type = UDP});

    int socket = dtn_socket_create(config, false, NULL);
    char *name = dtn_ip_link_get_interface_name(socket);
    testrun(name);
    name = dtn_data_pointer_free(name);

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_ip_link_get_state() {

    dtn_socket_configuration config = dtn_socket_load_dynamic_port(
        (dtn_socket_configuration){.host = "localhost", .type = UDP});

    int socket = dtn_socket_create(config, false, NULL);
    char *name = dtn_ip_link_get_interface_name(socket);
    testrun(name);

    // localhost should be up
    testrun(DTN_IP_LINK_UP == dtn_ip_link_get_state(name));
    name = dtn_data_pointer_free(name);

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_io_link_get_all_interfaces() {

    dtn_item *item = dtn_io_link_get_all_interfaces();
    testrun(item);
    char *string = dtn_item_to_json(item);
    fprintf(stdout, "%s\n", string);
    string = dtn_data_pointer_free(string);
    item = dtn_item_free(item);

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
    testrun_test(test_dtn_ip_link_get_interface_name);
    testrun_test(test_dtn_ip_link_get_state);
    testrun_test(test_dtn_io_link_get_all_interfaces);

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
