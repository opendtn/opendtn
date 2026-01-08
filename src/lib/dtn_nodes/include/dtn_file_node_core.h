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
        @file           dtn_file_node_core.h
        @author         Töpfer, Markus

        @date           2025-12-23

        This is the core of a DTN node able to receive and send some files.

        ------------------------------------------------------------------------
*/
#ifndef dtn_file_node_core_h
#define dtn_file_node_core_h

#include <dtn/dtn_cbor.h>
#include <dtn/dtn_security_config.h>
#include <dtn_base/dtn_event_loop.h>

/*---------------------------------------------------------------------------*/

typedef struct dtn_file_node_core dtn_file_node_core;

/*---------------------------------------------------------------------------*/

typedef struct dtn_file_node_core_config {

    dtn_event_loop *loop;

    char keys[PATH_MAX];

    struct {

        uint64_t threadlock_timeout_usec;
        uint64_t message_queue_capacity;
        uint64_t threads;
        uint64_t link_check;
        uint64_t buffer_time_cleanup_usecs;
        uint64_t history_secs;
        uint64_t max_buffer_time_secs;

    } limits;

    dtn_security_config sec;

} dtn_file_node_core_config;

/*
 *      ------------------------------------------------------------------------
 *
 *      GENERIC FUNCTIONS
 *
 *      ------------------------------------------------------------------------
 */

dtn_file_node_core *dtn_file_node_core_create(dtn_file_node_core_config config);
dtn_file_node_core *dtn_file_node_core_free(dtn_file_node_core *self);
dtn_file_node_core *dtn_file_node_core_cast(const void *data);

/*
 *      ------------------------------------------------------------------------
 *
 *      CONFIG FUNCTIONS
 *
 *      ------------------------------------------------------------------------
 */

bool dtn_file_node_core_enable_ip_interfaces(dtn_file_node_core *self,
                                             const dtn_item *config);

/*---------------------------------------------------------------------------*/

bool dtn_file_node_core_enable_routes(dtn_file_node_core *self,
                                      const char *path);

/*---------------------------------------------------------------------------*/

bool dtn_file_node_core_set_source_uri(dtn_file_node_core *self,
                                       const char *uri);

/*---------------------------------------------------------------------------*/

bool dtn_file_node_core_set_reception_path(dtn_file_node_core *self,
                                           const char *path);

/*
 *      ------------------------------------------------------------------------
 *
 *      SEND FUNCTIONS
 *
 *      ------------------------------------------------------------------------
 */

bool dtn_file_node_core_send_file(dtn_file_node_core *self, const char *uri,
                                  const char *source_path,
                                  const char *destination_path);

#endif /* dtn_file_node_core_h */
