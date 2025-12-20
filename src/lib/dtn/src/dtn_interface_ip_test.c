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
        @file           dtn_interface_ip_test.c
        @author         Töpfer, Markus

        @date           2025-12-19


        ------------------------------------------------------------------------
*/
#include <dtn_base/testrun.h>
#include "dtn_interface_ip.c"

/*
 *      ------------------------------------------------------------------------
 *
 *      TEST CASES                                                      #CASES
 *
 *      ------------------------------------------------------------------------
 */

int test_dtn_interface_ip_create(){
    
    dtn_event_loop *loop = dtn_event_loop_default(
        (dtn_event_loop_config){.max.sockets = 100, .max.timers = 100});
    testrun(loop);

    dtn_interface_ip_config config = (dtn_interface_ip_config){
        .loop = loop,
        .socket = dtn_socket_load_dynamic_port((dtn_socket_configuration){
            .host = "localhost",
            .type = UDP
        })};

    dtn_interface_ip *self = dtn_interface_ip_create(config);
    testrun(self);
    testrun(dtn_interface_ip_cast(self));
    testrun(self->buffer)
    testrun(DTN_TIMER_INVALID != self->timer.link_check);

    testrun(NULL == dtn_interface_ip_free(self));
    testrun(NULL == dtn_event_loop_free(loop));
    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_interface_ip_free(){
    
    dtn_event_loop *loop = dtn_event_loop_default(
        (dtn_event_loop_config){.max.sockets = 100, .max.timers = 100});
    testrun(loop);

    dtn_interface_ip_config config = (dtn_interface_ip_config){
        .loop = loop,
        .socket = dtn_socket_load_dynamic_port((dtn_socket_configuration){
            .host = "localhost",
            .type = UDP
        })};

    dtn_interface_ip *self = dtn_interface_ip_create(config);
    testrun(NULL == dtn_interface_ip_free(self));
    testrun(NULL == dtn_event_loop_free(loop));
    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_interface_ip_name(){
    
    dtn_event_loop *loop = dtn_event_loop_default(
        (dtn_event_loop_config){.max.sockets = 100, .max.timers = 100});
    testrun(loop);

    dtn_interface_ip_config config = (dtn_interface_ip_config){
        .loop = loop,
        .socket = dtn_socket_load_dynamic_port((dtn_socket_configuration){
            .host = "127.0.0.1",
            .type = UDP
        })};

    dtn_interface_ip *self = dtn_interface_ip_create(config);
    testrun(self);
    const char *name = dtn_interface_ip_name(self);
    testrun(0 == strcmp("127.0.0.1", name));

    testrun(NULL == dtn_interface_ip_free(self));
    testrun(NULL == dtn_event_loop_free(loop));
    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

struct dummy {

    dtn_socket_data remote;
    dtn_bundle *bundle;
};

/*----------------------------------------------------------------------------*/

static void dummy_io(void *userdata, 
    const dtn_socket_data *remote, dtn_bundle *bundle, const char *name){

    UNUSED(name);

    struct dummy *dummy = (struct dummy*) userdata;

    dummy->bundle = dtn_bundle_free(dummy->bundle);
    dummy->remote = *remote;
    dummy->bundle = bundle;
    return;
}


/*----------------------------------------------------------------------------*/

int check_io(){

    struct dummy dummy = {0};
    
    dtn_event_loop *loop = dtn_event_loop_default(
        (dtn_event_loop_config){.max.sockets = 100, .max.timers = 100});
    testrun(loop);

    dtn_interface_ip_config config = (dtn_interface_ip_config){
        .loop = loop,
        .socket = dtn_socket_load_dynamic_port((dtn_socket_configuration){
            .host = "127.0.0.1",
            .type = UDP
        }),
        .callbacks.userdata = &dummy,
        .callbacks.io = dummy_io
    };

    dtn_interface_ip *self = dtn_interface_ip_create(config);
    testrun(self);

    dtn_socket_configuration client_config = dtn_socket_load_dynamic_port(
        (dtn_socket_configuration){
            .host = "127.0.0.1",
            .type = UDP
        });
    
    int client = dtn_socket_create(client_config, false, NULL);
    testrun(dtn_socket_ensure_nonblocking(client));

    dtn_bundle *bundle = dtn_bundle_create();

    // check min valid
    dtn_cbor *primary = dtn_bundle_add_primary_block(
        bundle,
        0,
        0,
        "destination",
        "source",
        "report",
        3,
        4,
        5,
        0,
        0);

    dtn_cbor *payload = dtn_bundle_add_block(
        bundle,
        1,
        1,
        0,
        0,
        dtn_cbor_string("test"));

    testrun(primary);
    testrun(payload);
    testrun(bundle);

    struct sockaddr_storage sa = {0};
    socklen_t sa_len = sizeof(sa);

    testrun(dtn_socket_fill_sockaddr_storage(&sa, AF_INET,
        config.socket.host, config.socket.port));

    uint8_t *next = 0;
    uint8_t buffer[1024] = {0};
    testrun(dtn_bundle_encode(bundle, buffer, 1024, &next));
    size_t one_bundle = next - buffer;
    ssize_t bytes = -1;

    while(-1 == bytes){

        bytes = sendto(client, buffer, next - buffer, 0, (struct sockaddr *)&sa, sa_len);
        dtn_event_loop_run(loop, DTN_RUN_ONCE);
    }

    testrun(dummy.bundle);
    dummy.bundle = dtn_bundle_free(dummy.bundle);

    // send some garbadge
    bytes = -1;

    while(-1 == bytes){

        bytes = sendto(client, buffer +10, 20 , 0, (struct sockaddr *)&sa, sa_len);
        dtn_event_loop_run(loop, DTN_RUN_ONCE);
    }

    bytes = -1;

    while(-1 == bytes){

        bytes = sendto(client, buffer + 15, 20 , 0, (struct sockaddr *)&sa, sa_len);
        dtn_event_loop_run(loop, DTN_RUN_ONCE);
    }

    testrun(!dummy.bundle);

    // send one valid bundle after garbadge
    bytes = -1;

    while(-1 == bytes){

        bytes = sendto(client, buffer, one_bundle , 0, (struct sockaddr *)&sa, sa_len);
        dtn_event_loop_run(loop, DTN_RUN_ONCE);
    }

    testrun(dummy.bundle);
    dummy.bundle = dtn_bundle_free(dummy.bundle);

    // send one partial valid bundle
    bytes = -1;

    while(-1 == bytes){

        bytes = sendto(client, buffer, 10 , 0, (struct sockaddr *)&sa, sa_len);
        dtn_event_loop_run(loop, DTN_RUN_ONCE);
    }

    testrun(!dummy.bundle);


    // add some none matching buffer
    bytes = -1;

    while(-1 == bytes){

        bytes = sendto(client, "test1234567890", 14 , 0, (struct sockaddr *)&sa, sa_len);
        dtn_event_loop_run(loop, DTN_RUN_ONCE);
    }

    // send one valid bundle after garbadge
    bytes = -1;

    // send after after garbadge
    bytes = -1;

    while(-1 == bytes){

        bytes = sendto(client, buffer, one_bundle , 0, (struct sockaddr *)&sa, sa_len);
        dtn_event_loop_run(loop, DTN_RUN_ONCE);
    }

    testrun(dummy.bundle);
    dummy.bundle = dtn_bundle_free(dummy.bundle);

    memset(buffer, 0, 1024);
    testrun(dtn_bundle_encode(bundle, buffer, 1024, &next));

    bytes = -1;

    while(-1 == bytes){

        bytes = sendto(client, buffer, 10, 0, (struct sockaddr *)&sa, sa_len);
        dtn_event_loop_run(loop, DTN_RUN_ONCE);
    }

    testrun(!dummy.bundle);

    bytes = -1;

    while(-1 == bytes){

        bytes = sendto(client, buffer + 10, 10, 0, (struct sockaddr *)&sa, sa_len);
        dtn_event_loop_run(loop, DTN_RUN_ONCE);
    }

    testrun(!dummy.bundle);

    bytes = -1;

    while(-1 == bytes){

        bytes = sendto(client, buffer + 20, next - buffer - 20, 0, (struct sockaddr *)&sa, sa_len);
        dtn_event_loop_run(loop, DTN_RUN_ONCE);
    }

    testrun(dummy.bundle);
    dummy.bundle = dtn_bundle_free(dummy.bundle);


    testrun(NULL == dtn_interface_ip_free(self));
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
    testrun_test(test_dtn_interface_ip_create);
    testrun_test(test_dtn_interface_ip_free);
    testrun_test(test_dtn_interface_ip_name);
    testrun_test(check_io);

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
