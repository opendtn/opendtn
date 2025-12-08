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
        @file           dtn_app_test.c
        @author         Töpfer, Markus

        @date           2025-12-04


        ------------------------------------------------------------------------
*/
#include <dtn_base/testrun.h>
#include "dtn_app.c"

/*
 *      ------------------------------------------------------------------------
 *
 *      TEST CASES                                                      #CASES
 *
 *      ------------------------------------------------------------------------
 */

int test_dtn_app_create(){

    dtn_event_loop *loop = dtn_event_loop_default(
      (dtn_event_loop_config){.max.sockets = 100, .max.timers = 100});
    testrun(loop);
    dtn_io_config io_config = {.loop = loop};
    dtn_io *io = dtn_io_create(io_config);
    testrun(dtn_io_cast(io));

    // check with minimal config
    dtn_app_config config = (dtn_app_config){
        .loop = loop,
        .io = io
    };

    dtn_app *app = dtn_app_create(config);
    testrun(app);
    testrun(dtn_app_cast(app));
    
    testrun(app->config.limits.threadlock_timeout_usec == 100000);
    testrun(app->config.limits.message_queue_capacity == 1000);
    testrun(app->config.limits.threads >= 1 );

    testrun(app->io_buffer);
    testrun(app->events.dict);
    testrun(app->thread_loop);

    testrun(NULL == dtn_app_free(app));

    // check with full config

    config = (dtn_app_config){
        .loop = loop,
        .io = io,
        .limits.threadlock_timeout_usec = 1234567,
        .limits.message_queue_capacity = 1234,
        .limits.threads = 2
    };

    app = dtn_app_create(config);
    testrun(app);
    testrun(dtn_app_cast(app));
    
    testrun(app->config.limits.threadlock_timeout_usec == 1234567);
    testrun(app->config.limits.message_queue_capacity == 1234);
    testrun(app->config.limits.threads == 2 );

    testrun(app->io_buffer);
    testrun(app->events.dict);
    testrun(app->thread_loop);

    testrun(NULL == dtn_app_free(app));

    testrun(NULL == dtn_io_free(io));
    testrun(NULL == dtn_event_loop_free(loop));

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

struct dummy_data {

    int socket;
    dtn_item *message;
};

/*----------------------------------------------------------------------------*/

static bool dummy_function(void *userdata, int socket, dtn_item *input){

    struct dummy_data *data = (struct dummy_data*) userdata;
    data->socket = socket;
    data->message = input;
    return true;
}

/*----------------------------------------------------------------------------*/

int test_dtn_app_free(){

    dtn_event_loop *loop = dtn_event_loop_default(
      (dtn_event_loop_config){.max.sockets = 100, .max.timers = 100});
    testrun(loop);
    dtn_io_config io_config = {.loop = loop};
    dtn_io *io = dtn_io_create(io_config);
    testrun(dtn_io_cast(io));

    // check with minimal config
    dtn_app_config config = (dtn_app_config){
        .loop = loop,
        .io = io
    };

    struct dummy_data dummy_userdata = {0};

    dtn_app *app = dtn_app_create(config);
    testrun(app);
    testrun(dtn_app_cast(app));
    
    testrun(app->config.limits.threadlock_timeout_usec == 100000);
    testrun(app->config.limits.message_queue_capacity == 1000);
    testrun(app->config.limits.threads >= 1 );

    testrun(app->io_buffer);
    testrun(app->events.dict);
    testrun(app->thread_loop);

    testrun(NULL == dtn_app_free(app));

    // check with data

    config = (dtn_app_config){
        .loop = loop,
        .io = io,
        .limits.threadlock_timeout_usec = 1234567,
        .limits.message_queue_capacity = 1234,
        .limits.threads = 2
    };

    app = dtn_app_create(config);
    testrun(app);
    testrun(dtn_app_cast(app));
    
    testrun(app->config.limits.threadlock_timeout_usec == 1234567);
    testrun(app->config.limits.message_queue_capacity == 1234);
    testrun(app->config.limits.threads == 2 );

    char *json_fragment = "{\"event\": null, \"data\":";
    testrun(dtn_json_io_buffer_push(app->io_buffer, 1, 
        (dtn_memory_pointer){
            .start = (uint8_t*) json_fragment,
            .length = strlen(json_fragment)
        }));

    testrun(dtn_app_register(app, "dummy", dummy_function, &dummy_userdata));

    testrun(app->io_buffer);
    testrun(app->events.dict);
    testrun(app->thread_loop);

    testrun(NULL == dtn_app_free(app));

    testrun(NULL == dtn_io_free(io));
    testrun(NULL == dtn_event_loop_free(loop));

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_app_set_debug(){

    dtn_event_loop *loop = dtn_event_loop_default(
      (dtn_event_loop_config){.max.sockets = 100, .max.timers = 100});
    testrun(loop);
    dtn_io_config io_config = {.loop = loop};
    dtn_io *io = dtn_io_create(io_config);
    testrun(dtn_io_cast(io));

    // check with minimal config
    dtn_app_config config = (dtn_app_config){
        .loop = loop,
        .io = io
    };

    dtn_app *app = dtn_app_create(config);
    testrun(app);

    dtn_app_set_debug(app, true);
    testrun(app->debug == true);

    dtn_app_set_debug(app, false);
    testrun(app->debug == false);

    testrun(NULL == dtn_app_free(app));

    testrun(NULL == dtn_io_free(io));
    testrun(NULL == dtn_event_loop_free(loop));

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_app_open_listener(){

    dtn_event_loop *loop = dtn_event_loop_default(
      (dtn_event_loop_config){.max.sockets = 100, .max.timers = 100});
    testrun(loop);
    dtn_io_config io_config = {.loop = loop};
    dtn_io *io = dtn_io_create(io_config);
    testrun(dtn_io_cast(io));

    // check with minimal config
    dtn_app_config config = (dtn_app_config){
        .loop = loop,
        .io = io
    };

    dtn_app *app = dtn_app_create(config);
    testrun(app);

    dtn_socket_configuration socket_config = dtn_socket_load_dynamic_port(
            (dtn_socket_configuration){
                .type = TCP,
                .host = "localhost"
            });

    int socket = dtn_app_open_listener(app, socket_config);
    testrun(socket > 0);

    testrun(dtn_app_close(app, socket));
    
    testrun(NULL == dtn_app_free(app));

    testrun(NULL == dtn_io_free(io));
    testrun(NULL == dtn_event_loop_free(loop));

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_app_open_connection(){

    dtn_event_loop *loop = dtn_event_loop_default(
      (dtn_event_loop_config){.max.sockets = 100, .max.timers = 100});
    testrun(loop);
    dtn_io_config io_config = {.loop = loop};
    dtn_io *io = dtn_io_create(io_config);
    testrun(dtn_io_cast(io));

    // check with minimal config
    dtn_app_config config = (dtn_app_config){
        .loop = loop,
        .io = io
    };

    dtn_app *app = dtn_app_create(config);
    testrun(app);

    dtn_socket_configuration socket_config =dtn_socket_load_dynamic_port(
            (dtn_socket_configuration){
                .type = TCP,
                .host = "127.0.0.1"
            });;

    int socket = dtn_app_open_listener(app, socket_config);
    testrun(socket > 0);

    testrun(dtn_event_loop_run(loop, DTN_RUN_ONCE));

    int connection = dtn_app_open_connection(app, 
        socket_config, (dtn_io_ssl_config){0});
    testrun(connection > socket);
    
    testrun(dtn_event_loop_run(loop, DTN_RUN_ONCE));

    testrun(NULL == dtn_io_free(io));
    testrun(NULL == dtn_app_free(app));
    testrun(NULL == dtn_event_loop_free(loop));

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_app_close(){

    dtn_event_loop *loop = dtn_event_loop_default(
      (dtn_event_loop_config){.max.sockets = 100, .max.timers = 100});
    testrun(loop);
    dtn_io_config io_config = {.loop = loop};
    dtn_io *io = dtn_io_create(io_config);
    testrun(dtn_io_cast(io));

    // check with minimal config
    dtn_app_config config = (dtn_app_config){
        .loop = loop,
        .io = io
    };

    dtn_app *app = dtn_app_create(config);
    testrun(app);

    dtn_socket_configuration socket_config = dtn_socket_load_dynamic_port(
            (dtn_socket_configuration){
                .type = TCP,
                .host = "127.0.0.1"
            });;

    int socket = dtn_app_open_listener(app, socket_config);
    testrun(socket > 0);

    testrun(dtn_event_loop_run(loop, DTN_RUN_ONCE));

    int connection = dtn_app_open_connection(app, 
        socket_config, (dtn_io_ssl_config){0});
    testrun(connection > socket);

    testrun(dtn_app_close(app, connection));
    testrun(dtn_app_close(app, socket));
    
    testrun(dtn_event_loop_run(loop, DTN_RUN_ONCE));

    testrun(NULL == dtn_io_free(io));
    testrun(NULL == dtn_app_free(app));
    testrun(NULL == dtn_event_loop_free(loop));

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_app_send(){

    struct dummy_data dummy = {0};

    dtn_event_loop *loop = dtn_event_loop_default(
      (dtn_event_loop_config){.max.sockets = 100, .max.timers = 100});
    testrun(loop);
    dtn_io_config io_config = {.loop = loop};
    dtn_io *io = dtn_io_create(io_config);
    testrun(dtn_io_cast(io));

    // check with minimal config
    dtn_app_config config = (dtn_app_config){
        .loop = loop,
        .io = io
    };

    dtn_app *app = dtn_app_create(config);
    testrun(app);

    dtn_socket_configuration socket_config =  dtn_socket_load_dynamic_port(
            (dtn_socket_configuration){
                .type = TCP,
                .host = "127.0.0.1"
            });;

    int socket = dtn_app_open_listener(app, socket_config);
    testrun(socket > 0);

    testrun(dtn_app_register(app, "key", dummy_function, &dummy));

    testrun(dtn_event_loop_run(loop, DTN_RUN_ONCE));
    testrun(dtn_event_loop_run(loop, DTN_RUN_ONCE));

    int connection = dtn_app_open_connection(app, 
        socket_config, (dtn_io_ssl_config){0});
    testrun(connection > socket);
    
    testrun(dtn_event_loop_run(loop, DTN_RUN_ONCE));

    char *buffer = "{\"event\":\"key\"}";
    testrun(dtn_app_send(app, connection, (uint8_t*)buffer, strlen(buffer)));

    while(!dummy.message){
        testrun(dtn_event_loop_run(loop, DTN_RUN_ONCE));
    }

    testrun(dtn_item_is_object(dummy.message));

    const char *event = dtn_item_get_string(dtn_item_get(dummy.message, "/event"));
    testrun(0 == strcmp(event, "key"));

    testrun(NULL == dtn_io_free(io));
    testrun(NULL == dtn_app_free(app));
    testrun(NULL == dtn_event_loop_free(loop));

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_app_send_json(){

    struct dummy_data dummy = {0};

    dtn_event_loop *loop = dtn_event_loop_default(
      (dtn_event_loop_config){.max.sockets = 100, .max.timers = 100});
    testrun(loop);
    dtn_io_config io_config = {.loop = loop};
    dtn_io *io = dtn_io_create(io_config);
    testrun(dtn_io_cast(io));

    // check with minimal config
    dtn_app_config config = (dtn_app_config){
        .loop = loop,
        .io = io
    };

    dtn_app *app = dtn_app_create(config);
    testrun(app);

    dtn_socket_configuration socket_config = dtn_socket_load_dynamic_port(
            (dtn_socket_configuration){
                .type = TCP,
                .host = "127.0.0.1"
            });;

    int socket = dtn_app_open_listener(app, socket_config);
    testrun(socket > 0);

    testrun(dtn_app_register(app, "key", dummy_function, &dummy));

    testrun(dtn_event_loop_run(loop, DTN_RUN_ONCE));
    testrun(dtn_event_loop_run(loop, DTN_RUN_ONCE));

    int connection = dtn_app_open_connection(app, 
        socket_config, (dtn_io_ssl_config){0});
    testrun(connection > socket);
    
    testrun(dtn_event_loop_run(loop, DTN_RUN_ONCE));

    dtn_item *out = dtn_item_object();
    testrun(dtn_item_object_set(out, "event", dtn_item_string("key")));

    testrun(dtn_app_send_json(app, connection, out));

    while(!dummy.message){
        testrun(dtn_event_loop_run(loop, DTN_RUN_ONCE));
    }

    testrun(dtn_item_is_object(dummy.message));

    const char *event = dtn_item_get_string(dtn_item_get(dummy.message, "/event"));
    testrun(0 == strcmp(event, "key"));

    testrun(NULL == dtn_io_free(io));
    testrun(NULL == dtn_app_free(app));
    testrun(NULL == dtn_event_loop_free(loop));

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_app_register(){

    struct dummy_data dummy = {0};

    dtn_event_loop *loop = dtn_event_loop_default(
      (dtn_event_loop_config){.max.sockets = 100, .max.timers = 100});
    testrun(loop);
    dtn_io_config io_config = {.loop = loop};
    dtn_io *io = dtn_io_create(io_config);
    testrun(dtn_io_cast(io));

    // check with minimal config
    dtn_app_config config = (dtn_app_config){
        .loop = loop,
        .io = io
    };

    dtn_app *app = dtn_app_create(config);
    testrun(app);

    testrun(dtn_app_register(app, "key", dummy_function, &dummy));
    testrun(NULL != dtn_dict_get(app->events.dict, "key"));
   
    testrun(NULL == dtn_io_free(io));
    testrun(NULL == dtn_app_free(app));
    testrun(NULL == dtn_event_loop_free(loop));

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_app_deregister(){

    struct dummy_data dummy = {0};
    
    dtn_event_loop *loop = dtn_event_loop_default(
      (dtn_event_loop_config){.max.sockets = 100, .max.timers = 100});
    testrun(loop);
    dtn_io_config io_config = {.loop = loop};
    dtn_io *io = dtn_io_create(io_config);
    testrun(dtn_io_cast(io));

    // check with minimal config
    dtn_app_config config = (dtn_app_config){
        .loop = loop,
        .io = io
    };

    dtn_app *app = dtn_app_create(config);
    testrun(app);

    testrun(dtn_app_register(app, "key", dummy_function, &dummy));
    testrun(NULL != dtn_dict_get(app->events.dict, "key"));
    testrun(dtn_app_deregister(app, "key"));
    testrun(NULL == dtn_dict_get(app->events.dict, "key"));

    testrun(NULL == dtn_io_free(io));
    testrun(NULL == dtn_app_free(app));
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
    testrun_test(test_dtn_app_create);
    testrun_test(test_dtn_app_free);
    testrun_test(test_dtn_app_set_debug);
    testrun_test(test_dtn_app_open_listener);
    testrun_test(test_dtn_app_open_connection);
    testrun_test(test_dtn_app_close);
    testrun_test(test_dtn_app_send);
    testrun_test(test_dtn_app_send_json);
    testrun_test(test_dtn_app_register);
    testrun_test(test_dtn_app_deregister);

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
