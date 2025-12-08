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
        @file           dtn_echo_app.c
        @author         Töpfer, Markus

        @date           2025-12-04

        This app will echo the register request of the client for some infinite 
        timespan. 

        This is a showcase sample to show how to use dtn_app for some real work. 

        ------------------------------------------------------------------------
*/

#include <dtn_base/dtn_event_loop.h>
#include <dtn_base/dtn_config.h>
#include <dtn_base/dtn_config_log.h>
#include <dtn_core/dtn_io.h>
#include <dtn_core/dtn_app.h>
#include <dtn_os/dtn_os.h>
#include <dtn_os/dtn_os_event_loop.h>

/*----------------------------------------------------------------------------*/

#define CONFIG_PATH                                                            \
  DTN_ROOT                                                                \
  "/src/samples/dtn_echo_app/config/server_config.json"

/*----------------------------------------------------------------------------*/

static bool echo_callback(void *userdata, int socket, dtn_item *input){

    dtn_app *app = dtn_app_cast(userdata);

    sleep(1);

    if (!dtn_app_send_json(app, socket, input)){

        dtn_log_error("Failed to send echo at %i", socket);
        goto error;

    }

    dtn_item_free(input);
    return true;
error:
    dtn_item_free(input);
    return false;
}

/*----------------------------------------------------------------------------*/

int main(int argc, char **argv) {

    int retval = EXIT_FAILURE;

    dtn_event_loop *loop = NULL;
    dtn_io *io = NULL;
    dtn_app *app = NULL;

    dtn_item *config = NULL;

    dtn_event_loop_config loop_config = (dtn_event_loop_config){
      .max.sockets = dtn_socket_get_max_supported_runtime_sockets(0),
      .max.timers  = dtn_socket_get_max_supported_runtime_sockets(0)
    };

    dtn_log_debug("support for %i socket connections", loop_config.max.sockets);

    const char *path = dtn_config_path_from_command_line(argc, argv);
    if (!path) path = CONFIG_PATH;

    if (path == VERSION_REQUEST_ONLY) goto error;

    config = dtn_config_load(path);

    if (!config){

        dtn_log_error("Failed to load config from path %s", path);
        goto error;

    } else {

        dtn_log_debug("Loaded config from path %s", path);

    }

    if (!dtn_config_log_from_json(config)) goto error;

    loop = dtn_event_loop_default(loop_config);

    if (!loop) {
        dtn_log_error("Failed to create eventloop");
        goto error;
    }

    if (!dtn_event_loop_setup_signals(loop)) goto error;

    dtn_io_config io_config = dtn_io_config_from_item(config);
    io_config.loop = loop;

    io = dtn_io_create(io_config);
    
    if (!io){
        dtn_log_error("Failed to create io layer. Abort.");
        goto error;
    }

    dtn_app_config app_config = dtn_app_config_from_item(config);
    app_config.loop = loop;
    app_config.io = io;
    app_config.register_client = true; // auto register for client connections

    app = dtn_app_create(app_config);

    dtn_app_set_debug(app, true);

    if (!app){
        dtn_log_error("Failed to create app layer. Abort.");
        goto error;
    }

    if (!dtn_app_register(app, "register", echo_callback, app)){

        dtn_log_error("Failed to initiate event callback. Abort.");
        goto error;
    }

    int socket = 0;

    if (dtn_item_is_true(dtn_item_get(config, "/as_client"))){

        socket = dtn_app_open_connection(app, 
            dtn_socket_configuration_from_item(
                    dtn_item_get(config, "/socket")),
            (dtn_io_ssl_config){0}
            );
    
    } else {

        socket = dtn_app_open_listener(app, 
            dtn_socket_configuration_from_item(
                    dtn_item_get(config, "/socket"))
            );

        if (socket < 1){
            dtn_log_error("Failed to open server socket. Abort.");
            goto error;
        }

    }

    loop->run(loop, DTN_RUN_MAX);
    retval = true;

error:
    dtn_item_free(config);
    dtn_event_loop_free(loop);
    dtn_io_free(io);
    dtn_app_free(app);
    return retval;
}