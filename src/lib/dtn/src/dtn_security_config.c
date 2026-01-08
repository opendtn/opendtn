/***
        ------------------------------------------------------------------------

        Copyright (c) 2026 German Aerospace Center DLR e.V. (GSOC)

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
        @file           dtn_security_config.c
        @author         Töpfer, Markus

        @date           2026-01-07


        ------------------------------------------------------------------------
*/
#include "../include/dtn_security_config.h"

#include <dtn_base/dtn_string.h>

dtn_security_config dtn_security_config_from_item(const dtn_item *input){

    dtn_security_config config = {0};

    const dtn_item *conf = dtn_item_get(input, "/security");
    if (!conf) conf = input;

    dtn_item *bib = dtn_item_get(conf, "/bib");

    if (bib) {

        if (dtn_item_is_true(dtn_item_get(bib, "/protect/header")))
            config.bib.protect.header = true;

        if (dtn_item_is_true(dtn_item_get(bib, "/new_key")))
            config.bib.new_key = true;

        if (dtn_item_is_true(dtn_item_get(bib, "/aad")))
            config.bib.aad_flags = 0x07;

        const char *sha = dtn_item_get_string(dtn_item_get(bib, "/sha"));
        
        if (0 == dtn_string_compare(sha, "sha256")){
            config.bib.sha = HMAC256;
        } else if (0 == dtn_string_compare(sha, "sha512")){
            config.bib.sha = HMAC512;
        } else {
            config.bib.sha = HMAC384;
        }
    }

    dtn_item *bcb = dtn_item_get(conf, "/bcb");
    
    if (bcb) {

        if (dtn_item_is_true(dtn_item_get(bcb, "/protect/bib")))
            config.bcb.protect.bib = true;

        if (dtn_item_is_true(dtn_item_get(bcb, "/protect/payload")))
            config.bcb.protect.payload = true;

        if (dtn_item_is_true(dtn_item_get(bcb, "/new_key")))
            config.bcb.new_key = true;

        if (dtn_item_is_true(dtn_item_get(bcb, "/aad")))
            config.bcb.aad_flags = 0x07;

        const char *sha = dtn_item_get_string(dtn_item_get(bcb, "/aes"));
        
        if (0 == dtn_string_compare(sha, "128")){
            config.bcb.aes = A128GCM;
        } else {
            config.bcb.aes = A256GCM;
        }
    }

    return config;
}