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

#include <dtn_base/dtn_event_loop.h>

/*---------------------------------------------------------------------------*/

typedef struct dtn_routing dtn_routing;

/*---------------------------------------------------------------------------*/

typedef struct dtn_routing_config {

    dtn_event_loop *loop;

    char name[PATH_MAX];
    char route_config_path[PATH_MAX];

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

/*
 *      ------------------------------------------------------------------------
 *
 *      INTERFACES
 *
 *      ------------------------------------------------------------------------
 */

bool dtn_routing_register_interface(dtn_routing *self, const char *name);
bool dtn_routing_deregister_interface(dtn_routing *self, const char *name);

/*
 *      ------------------------------------------------------------------------
 *
 *      CONFIG
 *
 *      ------------------------------------------------------------------------
 */


#endif /* dtn_routing_h */
