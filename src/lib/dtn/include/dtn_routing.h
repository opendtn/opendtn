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
        @file           dtn_routing.h
        @author         Töpfer, Markus

        @date           2025-12-21


        ------------------------------------------------------------------------
*/
#ifndef dtn_routing_h
#define dtn_routing_h

#include "dtn_dtn_uri.h"
#include <dtn_base/dtn_event_loop.h>

typedef enum dtn_routing_class {

    DTN_ROUTING_ERROR = 0,
    DTN_ROUTING_DIRECT = 1,
    DTN_ROUTING_REGNAME = 2,
    DTN_ROUTING_DEFAULT = 3

} dtn_routing_class;

/*---------------------------------------------------------------------------*/

typedef struct dtn_routing_info {

    dtn_routing_class class;
    dtn_socket_configuration remote;
    char interface[DTN_HOST_NAME_MAX];

} dtn_routing_info;

/*---------------------------------------------------------------------------*/

typedef struct dtn_routing dtn_routing;

/*---------------------------------------------------------------------------*/

typedef struct dtn_routing_config {

    dtn_event_loop *loop;

    char name[PATH_MAX];
    char route_config_path[PATH_MAX];

    struct {

        uint64_t threadlock_timeout_usecs;

    } limits;

} dtn_routing_config;

/*
 *      ------------------------------------------------------------------------
 *
 *      GENERIC FUNCTIONS
 *
 *      ------------------------------------------------------------------------
 */

dtn_routing *dtn_routing_create(dtn_routing_config config);
dtn_routing *dtn_routing_free(dtn_routing *self);

bool dtn_routing_dump(FILE *file, dtn_routing *self);

/*---------------------------------------------------------------------------*/

/**
 *  Find a list of routing entries for the given uri,
 *  @returns a list of dtn_routing_info items.
 */
dtn_list *dtn_routing_get_info_for_uri(dtn_routing *self,
                                       const dtn_dtn_uri *uri);

/*---------------------------------------------------------------------------*/

bool dtn_routing_load(dtn_routing *self, const char *path);
bool dtn_routing_save(dtn_routing *self, const char *path);

#endif /* dtn_routing_h */
