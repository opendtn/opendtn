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
        @file           dtn_list.h
        @author         Michael Beer
        @author         Markus Toepfer

        @date           2018-07-02

        @ingroup        dtn_base

        @brief          Definition of a standard interface for LIST
                        implementations used for openvocs.

        Supports both FIFO and LIFO functionality:

        LIFO

        * insert via dtn_list_push(list, item) at end of list
        * get & remove via dtn_list_pop(list) to retrive from end of list

        FIFO

        *  Insert via dtn_list_insert(list, item, 0) at front of list
        *  get & remove via dtn_list_pop(list) to retrieve from end of list

        BEWARE: Default implementation is not very performant in terms of
        FIFO operations - better use `dtn_linked_list`  in that case.

        ------------------------------------------------------------------------
*/
#ifndef dtn_list_h
#define dtn_list_h

#include "dtn_data_function.h"
#include "dtn_log.h"

#define dtn_LIST_MAGIC_BYTE 0xabcd

typedef struct dtn_list dtn_list;
typedef struct dtn_list_config dtn_list_config;

/*---------------------------------------------------------------------------*/

struct dtn_list_config {

    dtn_data_function item;
};

/*---------------------------------------------------------------------------*/

struct dtn_list {

    uint16_t magic_byte;
    uint16_t type;

    dtn_list_config config;

    bool (*is_empty)(const dtn_list *self);

    bool (*clear)(dtn_list *self);
    /* Returns self on ERROR, NULL on SUCCESS */
    dtn_list *(*free)(dtn_list *self);

    /* Returns pointer to destination on success, NULL otherwise */
    dtn_list *(*copy)(const dtn_list *self, dtn_list *destination);

    /* GET position of item within list, returns 0 on error */
    size_t (*get_pos)(const dtn_list *self, const void *item);

    /* GET returns list item at pos FIRST item is at pos 1*/
    void *(*get)(dtn_list *self, size_t pos);
    /* SET returns any previously set old_item over the replaced pointer */
    bool (*set)(dtn_list *self, size_t pos, void *item, void **replaced);

    /* INSERT will move all following items to pos + 1 */
    bool (*insert)(dtn_list *self, size_t pos, void *item);
    /* REMOVE will return a removed element, NULL on error, all following
     * will move pos - 1*/
    void *(*remove)(dtn_list *self, size_t pos);

    bool (*push)(dtn_list *self, void *item);
    void *(*pop)(dtn_list *self);

    size_t (*count)(const dtn_list *self);

    bool (*for_each)(dtn_list *self, void *data,
                     bool (*function)(void *item, void *data));

    /**
     * @return an iterator pointing at the front of the list. To be used
     * with next. Becomes invalid if list is modified.
     */
    void *(*iter)(dtn_list *self);

    /**
     * @param iter an iterator retrieved with e.g. dtn_llist->iter.
     * @param element receives a pointer to the element iter currently
     * points at
     * @return updated iterator
     */
    void *(*next)(dtn_list *self, void *iter, void **element);
};

/*
 *      ------------------------------------------------------------------------
 *
 *                        DEFAULT STRUCTURE CREATION
 *
 *      ------------------------------------------------------------------------
 */

dtn_list *dtn_list_cast(const void *data);
dtn_list *dtn_list_create(dtn_list_config config);

/*
 *      ------------------------------------------------------------------------
 *
 *                        DATA FUNCTIONS
 *
 *       ... used to use a dtn_list within a parent container structure.
 *
 *      ------------------------------------------------------------------------
 */

bool dtn_list_clear(void *data);
void *dtn_list_free(void *data);
void *dtn_list_copy(void **destination, const void *source);
bool dtn_list_dump(FILE *stream, const void *data);
dtn_data_function dtn_list_data_functions();

/*
 *      ------------------------------------------------------------------------
 *
 *                        FUNCTIONS TO INTERNAL POINTERS
 *
 *
 *      ... following functions check if the list has the respective function
 * and execute the linked function.
 *      ------------------------------------------------------------------------
 */

bool dtn_list_is_empty(const dtn_list *list);
size_t dtn_list_get_pos(const dtn_list *list, void *item);
void *dtn_list_get(const dtn_list *list, size_t pos);
bool dtn_list_set(dtn_list *list, size_t pos, void *item, void **replaced);
bool dtn_list_insert(dtn_list *list, size_t pos, void *item);
void *dtn_list_remove(dtn_list *list, size_t pos);
bool dtn_list_delete(dtn_list *list, size_t pos);

/**
 * Insert element at end of list
 */
bool dtn_list_push(dtn_list *list, void *item);

/**
 * Retrieve * remove element from end of list
 */
void *dtn_list_pop(dtn_list *list);

size_t dtn_list_count(const dtn_list *list);
bool dtn_list_for_each(dtn_list *list, void *data,
                       bool (*function)(void *item, void *data));

/******************************************************************************
 *    Implementation independent default versions of some member functions
 ******************************************************************************/

dtn_list *dtn_list_set_magic_bytes(dtn_list *list);

typedef struct dtn_list_default_implementations {

    dtn_list *(*copy)(const dtn_list *self, dtn_list *destination);

} dtn_list_default_implementations;

dtn_list_default_implementations dtn_list_get_default_implementations();

/*
 *      ------------------------------------------------------------------------
 *
 *                        GENERIC FUNCTIONS
 *
 *       ... definition of common generic list functions
 *
 *      ------------------------------------------------------------------------
 */

/**
        Remove a item from a list, if it is included.
        This function checks if the item pointer is contained as list content.
        If so the value will be removed from the list. (first occurance only)
*/
bool dtn_list_remove_if_included(dtn_list *list, const void *item);

bool dtn_list_queue_push(dtn_list *list, void *item);
void *dtn_list_queue_pop(dtn_list *list);

#endif /* dtn_list_h */
