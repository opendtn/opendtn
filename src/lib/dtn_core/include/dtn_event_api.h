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

        This file is part of the openvocs project. https://openvocs.org

        ------------------------------------------------------------------------
*//**
        @file           dtn_event_api.h
        @author         Töpfer, Markus

        @date           2025-12-07


        ------------------------------------------------------------------------
*/
#ifndef dtn_event_api_h
#define dtn_event_api_h

#define DTN_EVENT_KEY_EVENT "event"
#define DTN_EVENT_KEY_UUID "uuid"
#define DTN_EVENT_KEY_TYPE "type"
#define DTN_EVENT_KEY_REQUEST "request"
#define DTN_EVENT_KEY_RESPONSE "response"
#define DTN_EVENT_KEY_PARAMETER "parameter"
#define DTN_EVENT_KEY_ERROR "error"
#define DTN_EVENT_KEY_ERROR_CODE "code"
#define DTN_EVENT_KEY_ERROR_DESC "desc"

#define DTN_EVENT_ERROR_CODE_INPUT 1000
#define DTN_EVENT_ERROR_DESC_INPUT "input missing"
#define DTN_EVENT_ERROR_CODE_AUTH 666
#define DTN_EVENT_ERROR_DESC_AUTH "auth error"



#define ETN_EVENT_ERROR_NO_ERROR 0

/*----------------------------------------------------------------------------*/

#include <stdbool.h>
#include <inttypes.h>
#include <dtn_base/dtn_item.h>

dtn_item *dtn_event_message_create(const char *uuid, const char *event);
dtn_item *dtn_event_message_create_response(const dtn_item *message);

bool dtn_event_is(const dtn_item *msg, const char *name);

/*----------------------------------------------------------------------------*/

bool dtn_event_set_error(dtn_item *message, uint64_t code, const char* desc);

uint64_t dtn_event_get_error_code(const dtn_item *message);
const char *dtn_event_get_error_desc(const dtn_item *message);

/*----------------------------------------------------------------------------*/

const char *dtn_event_get_event(const dtn_item *message);
const char *dtn_event_get_type(const dtn_item *message);
const char *dtn_event_get_uuid(const dtn_item *message);

/*----------------------------------------------------------------------------*/

dtn_item *dtn_event_get_paramenter(dtn_item *message);
dtn_item *dtn_event_get_request(dtn_item *message);
dtn_item *dtn_event_get_response(dtn_item *message);


#endif /* dtn_event_api_h */
