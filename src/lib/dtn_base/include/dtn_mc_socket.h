/***
        ------------------------------------------------------------------------

        Copyright (c) 2022 German Aerospace Center DLR e.V. (GSOC)

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
        @file           dtn_mc_socket.h
        @author         Markus Töpfer

        @date           2022-11-09


        ------------------------------------------------------------------------
*/
#ifndef dtn_mc_socket_h
#define dtn_mc_socket_h

#include "dtn_socket.h"
/*
 *      ------------------------------------------------------------------------
 *
 *      GENERIC FUNCTIONS
 *
 *      ------------------------------------------------------------------------
 */

int dtn_mc_socket(dtn_socket_configuration config);

bool dtn_mc_socket_drop_membership(int socket);

#endif /* dtn_mc_socket_h */
