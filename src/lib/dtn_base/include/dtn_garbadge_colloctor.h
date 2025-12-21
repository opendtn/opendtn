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
        @file           dtn_garbadge_colloctor.h
        @author         Töpfer, Markus

        @date           2025-12-21

        Garbadge Collection of blocking items of dicts and hashmaps.  

        The intended use case is as follows:
        you use a dict or a hashmap and have some self locking item in them.
        This means the datastructure within the dict has some threadlocks. 
        In this case you may control access of the dict using a threadlock on 
        the dict itself, but you may have no control if the item within the 
        dict is locked. In this case you may use the garbadge collotor to 
        ensure your items are cleaned up. 

        Code example:

        struct item {
            dtn_threadlock item_lock;
            void *data;
        };

        dtn_dict_push(dict, "name", struct item);

        When you want to remove the item you may use:

        struct item *item = dtn_dict_remove(dict, "name");
        No you can eventually delete the item, if all threads have given up the 
        lock, or the lock is still hold by some thread.
        If the lock is still hold you may simply push the item to the garbadge 
        collector using 

        dtn_garbadge_colloctor_push(collector, struct item*, item_free_function)

        This way you may use threadlocks within the item itself and check the lock
        during your item_free functionality.

        ------------------------------------------------------------------------
*/
#ifndef dtn_garbadge_colloctor_h
#define dtn_garbadge_colloctor_h

#include "dtn_event_loop.h"

/*------------------------------------------------------------------*/

typedef struct dtn_garbadge_colloctor dtn_garbadge_colloctor;

/*------------------------------------------------------------------*/

typedef struct dtn_garbadge_colloctor_config {

    dtn_event_loop *loop;

    struct {

        uint64_t run_cleanup_usec;
        uint64_t threadlock_timeout_usec;

    } limits;

} dtn_garbadge_colloctor_config;

/*
 *      ------------------------------------------------------------------------
 *
 *      GENERIC FUNCTIONS
 *
 *      ------------------------------------------------------------------------
 */

dtn_garbadge_colloctor *dtn_garbadge_colloctor_create(
    dtn_garbadge_colloctor_config config);

/*------------------------------------------------------------------*/

dtn_garbadge_colloctor *dtn_garbadge_colloctor_free(
    dtn_garbadge_colloctor *self);

/*------------------------------------------------------------------*/

bool dtn_garbadge_colloctor_push(
    dtn_garbadge_colloctor *self,
    void *data,
    void *(*free_data)(void *data));


#endif /* dtn_garbadge_colloctor_h */
