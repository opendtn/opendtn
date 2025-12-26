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
        @file           dtn_interface_ip.h
        @author         Töpfer, Markus

        @date           2025-12-19


        ------------------------------------------------------------------------
*/
#ifndef dtn_interface_ip_h
#define dtn_interface_ip_h

#include <dtn_base/dtn_event_loop.h>
#include <dtn_base/dtn_socket.h>
#include <dtn_base/dtn_ip_link.h>

#include "dtn_bundle.h"

/*---------------------------------------------------------------------------*/

typedef struct dtn_interface_ip dtn_interface_ip;

/*---------------------------------------------------------------------------*/

typedef struct dtn_interface_ip_config {
    
    dtn_event_loop *loop;
    dtn_socket_configuration socket;

    struct {

        uint64_t link_check;
        uint64_t threadlock_timeout_usecs;

    } limits;

    struct {

        void *userdata;

        // bundle reception
        void (*io)(void *userdata, 
            const dtn_socket_data *remote, dtn_bundle *bundle, const char *name);

        // state change propagation
        void (*state)(void *userdata, dtn_ip_link_state state, const char *name);

        // close state propagation
        void (*close)(void *userdata, const char *name);

    } callbacks;

} dtn_interface_ip_config;

/*
 *      ------------------------------------------------------------------------
 *
 *      GENERIC FUNCTIONS
 *
 *      ------------------------------------------------------------------------
 */

dtn_interface_ip *dtn_interface_ip_create(dtn_interface_ip_config config);
void *dtn_interface_ip_free(void *self);
dtn_interface_ip *dtn_interface_ip_cast(const void *self);

const char *dtn_interface_ip_name(const dtn_interface_ip *self);

bool dtn_interface_ip_send(dtn_interface_ip *self,
    dtn_socket_configuration remote,
    const uint8_t *buffer,
    size_t size);


#endif /* dtn_interface_ip_h */
