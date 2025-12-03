/***
        ------------------------------------------------------------------------

        Copyright 2018 German Aerospace Center DLR e.V. (GSOC)

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
        @file           dtn_linked_list.h
        @author         Michael Beer

        @date           2018-07-09

        @ingroup        dtn_base

        @brief          Definition of a linked list based list implementaion.


        ------------------------------------------------------------------------
*/
#ifndef dtn_linked_list_h
#define dtn_linked_list_h

#include "dtn_list.h"

/*
 *      ------------------------------------------------------------------------
 *
 *                        STRUCTURE CREATION
 *
 *      ------------------------------------------------------------------------
 */

dtn_list *dtn_linked_list_create(dtn_list_config config);

/**
 * Enables caching for lists / internal data structures.
 * BEWARE: Call dtn_registerd_cache_free_all before process termination,
 * otherwise you will encounter memleaks
 */
void dtn_linked_list_enable_caching(size_t capacity);

#endif /* dtn_linked_list_h */
