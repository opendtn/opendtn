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
        @file           dtn_ip_link.h
        @author         Töpfer, Markus

        @date           2025-12-19


        ------------------------------------------------------------------------
*/
#ifndef dtn_ip_link_h
#define dtn_ip_link_h

#include "dtn_item.h"

typedef enum dtn_ip_link_state {

    DTN_IP_LINK_ERROR = 0,
    DTN_IP_LINK_DOWN = 1,
    DTN_IP_LINK_UP = 2

} dtn_ip_link_state;

/*------------------------------------------------------------------*/

char *dtn_ip_link_get_interface_name(int socket);

/*------------------------------------------------------------------*/

dtn_ip_link_state dtn_ip_link_get_state(const char *interface_name);

/*------------------------------------------------------------------*/

/**
 *      Returns a dict of name and ip address
 */
dtn_item *dtn_io_link_get_all_interfaces();

#endif /* dtn_ip_link_h */
