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
        @file           dtn_router.c
        @author         Töpfer, Markus

        @date           2025-12-21


        ------------------------------------------------------------------------
*/


#include <dtn_base/dtn_config.h>
#include <dtn_base/dtn_config_log.h>
#include <dtn_base/dtn_event_loop.h>
#include <dtn_base/dtn_item.h>
#include <dtn_base/dtn_item_json.h>
#include <dtn_base/dtn_string.h>

#include <dtn_core/dtn_event_api.h>
#include <dtn_core/dtn_io.h>
#include <dtn_core/dtn_webserver.h>

#include <dtn_nodes/dtn_router_app.h>

#include <dtn_os/dtn_os_event_loop.h>

/*---------------------------------------------------------------------------*/

#define CONFIG_PATH                                                            \
  DTN_ROOT                                                                \
  "/src/service/dtn_router/config/default_config.json"

/*---------------------------------------------------------------------------*/

int main(int argc, char **argv) {

    int retval = EXIT_FAILURE;

    dtn_event_loop *loop = NULL;
    dtn_io *io = NULL;
    dtn_webserver *server = NULL;
    dtn_router_app *node = NULL;
    dtn_item *config = NULL;

    dtn_event_loop_config loop_config = (dtn_event_loop_config){
        .max.sockets = dtn_socket_get_max_supported_runtime_sockets(0),
        .max.timers = dtn_socket_get_max_supported_runtime_sockets(0)};

    const char *path = dtn_config_path_from_command_line(argc, argv);
    if (!path) path = CONFIG_PATH;

    if (path == VERSION_REQUEST_ONLY) goto done;

    config = dtn_config_load(path);

    if (!config) {
        
        dtn_log_error("failed to load config from %s", path);
        goto error;

    } else {
        
        dtn_log_debug("loaded config from %s", path);
    }

    if (!dtn_config_log_from_json(config)) goto error;

    // load eventloop

    loop = dtn_os_event_loop(loop_config);
    if (!loop) goto error;
    if (!dtn_event_loop_setup_signals(loop)) goto error;

    // load io interface 

    dtn_io_config io_config = dtn_io_config_from_item(config);
    io_config.loop = loop;

    io = dtn_io_create(io_config);
    if (!io) goto error;

    // load webserver 

    dtn_webserver_config server_config = dtn_webserver_config_from_item(config);
    server_config.loop = loop;
    server_config.io = io;

    server = dtn_webserver_create(server_config);
    if (!server) goto error;

    if (!dtn_webserver_enable_domains(server, config)) goto error;

    // add the node

    const char *domain = dtn_item_get_string(dtn_item_get(config, 
                            "/webserver/domains/0/name"));

    dtn_log_debug("using domain %s", domain);

    dtn_router_app_config node_config = dtn_router_app_config_from_item(config);
    node_config.loop = loop;
    node_config.io = io;
    node_config.server = server;

    node = dtn_router_app_create(node_config);
    if (!node) goto error;

    if (!dtn_router_enable_ip_interfaces(node, config))
        goto error;

    if (!dtn_webserver_enable_callback(
        server, 
        domain,
        node,
        dtn_router_app_websocket_callback)) goto error;

    dtn_log_info("Enabled JSON IO callback for node %s", domain);

    dtn_event_loop_run(loop, DTN_RUN_MAX);

done:
    retval = EXIT_SUCCESS;

error:
    
    config = dtn_item_free(config);
    io = dtn_io_free(io);
    server = dtn_webserver_free(server);
    node = dtn_router_app_free(node);
    loop = dtn_event_loop_free(loop);
    return retval;
}