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
#include "../include/dtn_dtn_uri.h"

#include <dtn_base/dtn_item_json.h>
#include <dtn_base/dtn_linked_list.h>
#include <dtn_base/dtn_thread_lock.h>
#include <dtn_base/dtn_socket.h>
#include <dtn_base/dtn_dir.h>

#define ROUTING_NAME "router"
#define ROUTING_CONFIG "/etc/opendtn/dtn_router/routes"

/*---------------------------------------------------------------------------*/

struct dtn_routing {

    dtn_routing_config config;

    struct {

        dtn_thread_lock lock;
        dtn_item *data;

    } routes;
    

};

/*---------------------------------------------------------------------------*/

static bool load_config(dtn_routing *self){

    if (!self) goto error;
    
    if (0 == self->config.route_config_path[0])
        goto error;

    dtn_log_debug("loading routes from %s", self->config.route_config_path);

    dtn_item *routes = dtn_item_json_read_dir(
        self->config.route_config_path, "route");

    if (!routes) goto error;

    char *string = dtn_item_to_json(routes);
    dtn_log_debug("loaded routes %s", string);
    string = dtn_data_pointer_free(string);

    if (!dtn_thread_lock_try_lock(&self->routes.lock)) goto error;

    self->routes.data = dtn_item_free(self->routes.data);
    self->routes.data = routes;

    if (!dtn_thread_lock_unlock(&self->routes.lock)){
        dtn_log_error("failed to unlock routes");
    }

    dtn_log_info("activated routes from %s", self->config.route_config_path);

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

    if (0 == config->limits.threadlock_timeout_usecs)
        config->limits.threadlock_timeout_usecs = 100000;

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

    if (!dtn_thread_lock_init(&self->routes.lock, 
        self->config.limits.threadlock_timeout_usecs)) goto error;
    
    if (!load_config(self))
        dtn_log_error("failed to load routes for %s", self->config.route_config_path);

    return self;

error:
    dtn_routing_free(self);
    return NULL;
}

/*---------------------------------------------------------------------------*/

dtn_routing *dtn_routing_free(dtn_routing *self){

    if (!self) return self;

    dtn_thread_lock_clear(&self->routes.lock);
    self->routes.data = dtn_item_free(self->routes.data);
    self = dtn_data_pointer_free(self);
    return NULL;
}

/*----------------------------------------------------------------------------*/

struct container {

    dtn_routing *self;
    const dtn_dtn_uri *uri;
    dtn_list *list;

};

/*----------------------------------------------------------------------------*/

static bool find_socket_config(
    const char *key, dtn_item const *val, void *userdata){

    dtn_routing_info *info = NULL;

    if (!key) return true;

    struct container *container = (struct container*) userdata;

    if (!container->uri) return true;
    if (!container->uri->name) return true;

    dtn_item *uris = dtn_item_object_get(val, "uris");
    if (!uris) goto done;

    dtn_item *uri = dtn_item_object_get(uris, container->uri->name);
    dtn_item *in = dtn_item_object_get(uri, "interface");
    const char *interface = dtn_item_get_string(in);

    if (uri){

        info = calloc(1, sizeof(dtn_routing_info));
        if (!info) goto error;

        info->class = DTN_ROUTING_REGNAME;
        info->remote = dtn_socket_configuration_from_item(
            dtn_item_object_get(uri, "socket"));


        if (interface)
            strncpy(info->interface, interface, strlen(interface));

        dtn_list_push(container->list, info);
    
    }

    char dtn_uri[2028] = {0};
    snprintf(dtn_uri, 2048, "%s/%s", container->uri->name, container->uri->demux);

    uri = dtn_item_object_get(uris, dtn_uri);
    in = dtn_item_object_get(uri, "interface");
    interface = dtn_item_get_string(in);

    
    if (uri){

        info = calloc(1, sizeof(dtn_routing_info));
        if (!info) goto error;

        info->class = DTN_ROUTING_DIRECT;
        info->remote = dtn_socket_configuration_from_item(
            dtn_item_object_get(uri, "socket"));

        if (interface)
            strncpy(info->interface, interface, strlen(interface));

        dtn_list_push(container->list, info);
    
    }

done:
    return true;
error:
    return false;
}

/*---------------------------------------------------------------------------*/

dtn_list *dtn_routing_get_info_for_uri(
    dtn_routing *self, const dtn_dtn_uri *uri){

    struct container container = (struct container){
        .self = self,
        .uri = uri,
        .list = dtn_linked_list_create((dtn_list_config){
            .item.free = dtn_data_pointer_free
        })
    };

    if (!dtn_thread_lock_try_lock(&self->routes.lock)) goto error;

    dtn_item_object_for_each(
        self->routes.data,
        find_socket_config,
        &container);

    if (!dtn_thread_lock_unlock(&self->routes.lock)){
        dtn_log_error("failed to unlock routes");
    }

    return container.list;
error:
    return NULL;
}

/*---------------------------------------------------------------------------*/

bool dtn_routing_load(dtn_routing *self, const char *path){

    if (!self) goto error;

    if (path) strncpy(self->config.route_config_path, path, PATH_MAX);
    return load_config(self);

error:
    return false;
}

/*---------------------------------------------------------------------------*/

struct container2 {

    dtn_routing *self;
    const char *path;
};

/*---------------------------------------------------------------------------*/

static bool write_route_info(const char *key, dtn_item const *item, void *data){

    if (!key) return true;
    struct container2 *container = (struct container2*) data;

    char file[2 * PATH_MAX] = {0};
    
    if (container->path){
        dtn_dir_tree_create(container->path);
        snprintf(file, 2 *PATH_MAX, "%s/%s", container->path, key);
    } else {
        snprintf(file, 2 *PATH_MAX, "%s/%s", container->self->config.route_config_path, key);
    }

    return dtn_item_json_write_file(file, item);
}

/*---------------------------------------------------------------------------*/

bool dtn_routing_save(dtn_routing *self, const char *path){

    if (!self) goto error;

    struct container2 container = (struct container2){
        .self = self,
        .path = path
    };

    if (!dtn_thread_lock_try_lock(&self->routes.lock)) goto error;

    dtn_item_object_for_each(
        self->routes.data,
        write_route_info,
        &container);

    if (!dtn_thread_lock_unlock(&self->routes.lock)){
        dtn_log_error("failed to unlock routes");
    }

    return true;

error:
    return false;
}

/*---------------------------------------------------------------------------*/

bool dtn_routing_dump(FILE *file, dtn_routing* self){

    if (!file || !self) goto error;

    if (!dtn_thread_lock_try_lock(&self->routes.lock)) goto error;

    char *string = dtn_item_to_json(self->routes.data);
    fprintf(file, "\n%s\n", string);
    string = dtn_data_pointer_free(string);

    if (!dtn_thread_lock_unlock(&self->routes.lock)){
        dtn_log_error("failed to unlock routes");
    }

    return true;

error:
    return false;
}