/***

Copyright   2018        German Aerospace Center DLR e.V.,
                        German Space Operations Center (GSOC)

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.

    This file is part of the openvocs project. http://openvocs.org

***/ /**

     \file               dtn_thread_message.h
     \author             Michael J. Beer, DLR/GSOC <michael.beer@dlr.de>
     \date               2018-01-29

 **/
/*---------------------------------------------------------------------------*/

#ifndef dtn_thread_message_h
#define dtn_thread_message_h

/*---------------------------------------------------------------------------*/

#include "dtn_item.h"

/******************************************************************************
 *                             Message structure
 ******************************************************************************/

#define DTN_THREAD_MESSAGE_MAGIC_BYTES 0x744D

#define DTN_THREAD_MESSAGE_VALID(x)                                             \
  ((0 != x) && (x)->magic_bytes == DTN_THREAD_MESSAGE_MAGIC_BYTES)

typedef enum {

  DTN_THREAD_MESSAGE_TYPE_ENSURE_SIGNED_INT_TYPE = -1,
  DTN_GENERIC_MESSAGE,
  DTN_THREAD_MESSAGE_START_USER_TYPES /* Last entry to allow this */

} dtn_thread_message_type;

/*---------------------------------------------------------------------------*/

/**
 * Basic message format - basically just an int determining the actual message
 * format.
 */
typedef struct dtn_thread_message_struct dtn_thread_message;

struct dtn_thread_message_struct {

  uint16_t magic_bytes;

  /**
   * Sub-type to distinguish different thread messages
   * Must be int to ensure enum constants fit in here ...
   */
  int type;

  /**
   * Original message received over signaling connection
   */
  dtn_item *message;

  /**
   * Hack
   */
  int socket;

  /**
   * Free this message.
   * Prefer dtn_thread_message_free() instead of calling this method directly
   */
  dtn_thread_message *(*free)(dtn_thread_message *);
};

/******************************************************************************
 *
 *  FUNCTIONS
 *
 ******************************************************************************/

/**
 * Create a standard thread message, i.e. a message that does not contain any
 * extensions to the basic dtn_thread_message struct.
 * Suitable for pure signaling messages like a 'shutdown' message, whose
 * aim is to trigger a shutdown of the thread pool etc.
 * Provides a default free()-method that frees the attached json value and
 * the message itself.
 */
dtn_thread_message *
dtn_thread_message_standard_create(dtn_thread_message_type type,
                                  dtn_item *message);

/*----------------------------------------------------------------------------*/

dtn_thread_message *dtn_thread_message_cast(void *vptr);

/*---------------------------------------------------------------------------*/

dtn_thread_message *dtn_thread_message_free(dtn_thread_message *msg);

/*----------------------------------------------------------------------------*/

#endif /* dtn_thread_message_h */
