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
        @file           dtn_security_config.h
        @author         Töpfer, Markus

        @date           2026-01-07


        ------------------------------------------------------------------------
*/
#ifndef dtn_security_config_h
#define dtn_security_config_h

/*----------------------------------------------------------------------------*/

#include "dtn_bpsec.h"
#include <dtn_base/dtn_item.h>

/*----------------------------------------------------------------------------*/

typedef struct dtn_security_config {

    struct {

        uint8_t aad_flags;
        dtn_bpsec_sha_variant sha;

        bool new_key;

        struct {

            bool header;

        } protect;

    } bib;

    struct {

        uint8_t aad_flags;
        dtn_bpsec_aes_variant aes;

        bool new_key;

        struct {

            bool bib;
            bool payload;

        } protect;

    } bcb;

} dtn_security_config;

/*----------------------------------------------------------------------------*/

dtn_security_config dtn_security_config_from_item(const dtn_item *input);

#endif /* dtn_security_config_h */
