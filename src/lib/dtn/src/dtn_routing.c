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
        @file           dtn_routing.c
        @author         Töpfer, Markus

        @date           2025-12-21


        ------------------------------------------------------------------------
*/
#include "../include/dtn_routing.h"

#include <dtn_base/dtn_item_json.h>

#define ROUTING_NAME "router"
#define ROUTING_CONFIG "/etc/opendtn/dtn_router/routes"

/*---------------------------------------------------------------------------*/

struct dtn_routing {

    dtn_routing_config config;

    dtn_item *routes;

};

/*---------------------------------------------------------------------------*/

static bool load_config(dtn_routing *self){

    if (!self) goto error;
    
    if (0 == self->config.route_config_path[0])
        goto error;

    dtn_log_info("using routes from %s", self->config.route_config_path);
    
    dtn_item *routes = dtn_item_json_read_dir(
        self->config.route_config_path, "route");

    if (!routes) goto error;

    char *string = dtn_item_to_json(routes);
    dtn_log_debug("loaded routes %s", string);
    string = dtn_data_pointer_free(string);

    self->routes = routes;
    return true;
error:
    return false;
}

/*---------------------------------------------------------------------------*/

static bool init_config(dtn_routing_config *config){

    if (!config->loop) goto error;

    if (0 == config->route_config_path[0])
        strncpy(config->route_config_path, ROUTING_CONFIG, PATH_MAX);

    if (0 == config->name[0])
        strncpy(config->name, ROUTING_NAME, PATH_MAX);

    return true;
error:
    return false;
}

/*
 *      ------------------------------------------------------------------------
 *
 *      GENERIC FUNCTIONS
 *
 *      ------------------------------------------------------------------------
 */

dtn_routing *dtn_routing_create(dtn_routing_config config){

    dtn_routing *self = NULL;

    if (!init_config(&config)) goto error;

    self = calloc(1, sizeof(dtn_routing));
    if (!self) goto error;

    self->config = config;
    
    if (!load_config(self)) goto error;

    return self;

error:
    dtn_routing_free(self);
    return NULL;
}

/*---------------------------------------------------------------------------*/

dtn_routing *dtn_routing_free(dtn_routing *self){

    if (!self) return self;

    self->routes = dtn_item_free(self->routes);
    self = dtn_data_pointer_free(self);
    return NULL;
}

/*
 *      ------------------------------------------------------------------------
 *
 *      INTERFACES
 *
 *      ------------------------------------------------------------------------
 */

bool dtn_routing_register_interface(dtn_routing *self, const char *name){

    if (!self || !name) goto error;


    return true;
error:
    return false;
}

/*---------------------------------------------------------------------------*/

bool dtn_routing_deregister_interface(dtn_routing *self, const char *name){

    if (!self || !name) goto error;


    return true;
error:
    return false;
}

/*
 *      ------------------------------------------------------------------------
 *
 *      CONFIG
 *
 *      ------------------------------------------------------------------------
 */

