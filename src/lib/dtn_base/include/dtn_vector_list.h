/***
        ------------------------------------------------------------------------

        Copyright (c) 2018 German Aerospace Center DLR e.V. (GSOC)

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
        @file           dtn_vector_list.h
        @author         Markus Toepfer

        @date           2018-08-09

        @ingroup        dtn_base

        @brief          Definition of a vector based list implementation.


        ------------------------------------------------------------------------
*/
#ifndef dtn_vector_list_h
#define dtn_vector_list_h

#include "dtn_list.h"

/*
 *      ------------------------------------------------------------------------
 *
 *                        STRUCTURE CREATION
 *
 *      ------------------------------------------------------------------------
 */

/**
        Create a vector (array) based list.
*/
dtn_list *dtn_vector_list_create(dtn_list_config config);

/*----------------------------------------------------------------------------*/

/**
        Set the vector items array to shrink automatically (shrink == true).

        Check at source code. Usecase: dynamic growing and shrinking of the
        array used for implementation.
*/
bool dtn_vector_list_set_shrink(dtn_list *list, bool shrink);

/*----------------------------------------------------------------------------*/

/**
        Set growth rate of a vector array list.
        This setting defines the default slot allocations and reallocations
        of the array used for implementation of the vector list.

        Check at source code. Usecase: Set the rate to the optimal setting for
        the use case of the list.
*/
bool dtn_vector_list_set_rate(dtn_list *list, size_t rate);

#endif /* dtn_vector_list_h */
