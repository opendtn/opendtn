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
        @file           dtn_event_api.c
        @author         Töpfer, Markus

        @date           2025-12-07


        ------------------------------------------------------------------------
*/
#include "../include/dtn_event_api.h"

#include <dtn_base/dtn_id.h>

dtn_item *dtn_event_message_create(const char *uuid, const char *event){

    dtn_id id = {0};

    dtn_item *msg = NULL;

    if (!event) goto error;

    msg = dtn_item_object();
    if (!msg) goto error;

    if (!dtn_item_object_set(msg, 
        DTN_EVENT_KEY_EVENT, dtn_item_string(event))) goto error;

    if (uuid){

        if (!dtn_item_object_set(msg, 
            DTN_EVENT_KEY_UUID, dtn_item_string(uuid))) goto error;
    
    } else {

        dtn_id_fill_with_uuid(id);

        if (!dtn_item_object_set(msg, 
            DTN_EVENT_KEY_UUID, dtn_item_string(id))) goto error;

    }
    
    return msg;

error:
    dtn_item_free(msg);
    return NULL;
}

/*----------------------------------------------------------------------------*/

dtn_item *dtn_event_message_create_reponse(const dtn_item *message){

    dtn_item *out = NULL;
    dtn_item *val = NULL;

    if (!message) goto error;

    if (!dtn_item_copy((void**)&val, message)) goto error;

    out = dtn_item_object();
    if (!dtn_item_object_set(out, DTN_EVENT_KEY_REQUEST, val))
        goto error;

    val = dtn_item_object();
    if (!dtn_item_object_set(out, DTN_EVENT_KEY_RESPONSE, val))
        goto error;

    return out;
error:
    dtn_item_free(out);
    dtn_item_free(val);
    return NULL;
}

/*----------------------------------------------------------------------------*/

bool dtn_event_set_error(dtn_item *message, uint64_t code, const char* desc){

    dtn_item *err = NULL;

    if (!message) goto error;

    err = dtn_item_object();

    if (!dtn_item_object_set(err, 
        DTN_EVENT_KEY_ERROR_CODE, dtn_item_number(code))) goto error;

    if (desc)
        if (!dtn_item_object_set(err, 
            DTN_EVENT_KEY_ERROR_DESC, dtn_item_string(desc))) goto error;

    if (!dtn_item_object_set(message, DTN_EVENT_KEY_ERROR, err)) goto error;

    return true;
error:
    dtn_item_free(err);
    return false;
}

/*----------------------------------------------------------------------------*/

uint64_t dtn_event_get_error_code(const dtn_item *message){

    return dtn_item_get_int(
        dtn_item_get(message, 
            "/"DTN_EVENT_KEY_ERROR"/"DTN_EVENT_KEY_ERROR_CODE));
}

/*----------------------------------------------------------------------------*/

const char *dtn_event_get_error_desc(const dtn_item *message){

    return dtn_item_get_string(
        dtn_item_get(message, 
            "/"DTN_EVENT_KEY_ERROR"/"DTN_EVENT_KEY_ERROR_DESC));
}

/*----------------------------------------------------------------------------*/

const char *dtn_event_get_event(const dtn_item *message){

    return dtn_item_get_string(
        dtn_item_get(message, 
            "/"DTN_EVENT_KEY_EVENT));
}

/*----------------------------------------------------------------------------*/

const char *dtn_event_get_type(const dtn_item *message){

    return dtn_item_get_string(
        dtn_item_get(message, 
            "/"DTN_EVENT_KEY_TYPE));
}

/*----------------------------------------------------------------------------*/

const char *dtn_event_get_uuid(const dtn_item *message){

    return dtn_item_get_string(
        dtn_item_get(message, 
            "/"DTN_EVENT_KEY_UUID));
}

/*----------------------------------------------------------------------------*/

dtn_item *dtn_event_get_paramenter(dtn_item *message){

    dtn_item *par = NULL;
    if (!message) goto error;

    par = dtn_item_object_get(message,DTN_EVENT_KEY_PARAMETER);
    
    if (!par){

        par = dtn_item_object();
        if (!dtn_item_object_set(message,DTN_EVENT_KEY_PARAMETER, par)){
            par = dtn_item_free(par);
            goto error;
        }
    }

    return par;
error:
    return NULL;
}

/*----------------------------------------------------------------------------*/

dtn_item *dtn_event_get_request(dtn_item *message){

    dtn_item *par = NULL;
    if (!message) goto error;

    par = dtn_item_object_get(message,DTN_EVENT_KEY_REQUEST);
    
    if (!par){

        par = dtn_item_object();
        if (!dtn_item_object_set(message,DTN_EVENT_KEY_REQUEST, par)){
            par = dtn_item_free(par);
            goto error;
        }
    }

    return par;
error:
    return NULL;
}

/*----------------------------------------------------------------------------*/

dtn_item *dtn_event_get_response(dtn_item *message){

    dtn_item *par = NULL;
    if (!message) goto error;

    par = dtn_item_object_get(message,DTN_EVENT_KEY_RESPONSE);
    
    if (!par){

        par = dtn_item_object();
        if (!dtn_item_object_set(message,DTN_EVENT_KEY_RESPONSE, par)){
            par = dtn_item_free(par);
            goto error;
        }
    }

    return par;
error:
    return NULL;
}
