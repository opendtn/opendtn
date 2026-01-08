/***
        ------------------------------------------------------------------------

        Copyright (c) 2023 German Aerospace Center DLR e.V. (GSOC)

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
        @file           dtn_dtls.h
        @author         Markus

        @date           2023-12-13


        ------------------------------------------------------------------------
*/
#ifndef dtn_dtls_h
#define dtn_dtls_h

#include "dtn_hash.h"
#include <dtn_base/dtn_event_loop.h>
#include <dtn_base/dtn_item.h>
#include <limits.h>

#define DTN_DTLS_SRTP_PROFILES                                                 \
    "SRTP_AES128_CM_SHA1_80:"                                                  \
    "SRTP_AES128_CM_SHA1_32"

#define DTN_DTLS_PROFILE_MAX 1024

#define DTN_KEY_ACTIVE "active"
#define DTN_KEY_PASSIVE "passive"
#define DTN_KEY_UNSET "unset"

#define DTN_DTLS_KEY_MAX 48  // min 32 key + 14 salt
#define DTN_DTLS_SALT_MAX 14 // 112 bit salt

#define DTN_DTLS_SSL_BUFFER_SIZE 16000 // max SSL buffer size
#define DTN_DTLS_SSL_ERROR_STRING_SIZE 200

// Default DTLS keys and key length
#define DTN_DTLS_FINGERPRINT_MAX 3 * DTN_SHA512_SIZE + 10 // SPname/0 HEX

/*----------------------------------------------------------------------------*/

typedef struct dtn_dtls dtn_dtls;

/*----------------------------------------------------------------------------*/

typedef enum {

    DTN_DTLS_ACTIVE = 1,
    DTN_DTLS_PASSIVE = 2

} dtn_dtls_type;

/*----------------------------------------------------------------------------*/

typedef struct {

    dtn_event_loop *loop;

    /*
     *      Certificate configuration for SSL,
     *      including pathes to certificate, key and verification chains.
     */

    char cert[PATH_MAX];
    char key[PATH_MAX];

    struct {

        char file[PATH_MAX]; // path to CA verify file
        char path[PATH_MAX]; // path to CAs to use

    } ca;

    /*
     *      SRTP specific configuration
     */

    struct {

        char profile[DTN_DTLS_PROFILE_MAX];

    } srtp;

    /*
     *      DTLS specific configuration
     */

    struct {

        struct {

            size_t quantity; // amount of DTLS keys used for cookies
            size_t length;   // min length of DTLS cookie

            uint64_t lifetime_usec; // lifetime of keys in usecs

        } keys;

    } dtls;

    uint64_t reconnect_interval_usec; // handshaking interval

} dtn_dtls_config;

/*
 *      ------------------------------------------------------------------------
 *
 *      GENERIC FUNCTIONS
 *
 *      ------------------------------------------------------------------------
 */

dtn_dtls_config dtn_dtls_config_from_item(const dtn_item *input);

dtn_dtls *dtn_dtls_create(dtn_dtls_config config);

dtn_dtls *dtn_dtls_free(dtn_dtls *self);

/*----------------------------------------------------------------------------*/

const char *dtn_dtls_get_fingerprint(const dtn_dtls *self);

/*----------------------------------------------------------------------------*/

const char *dtn_dtls_type_to_string(dtn_dtls_type type);

/*----------------------------------------------------------------------------*/

SSL_CTX *dtn_dtls_get_ctx(dtn_dtls *self);

/*----------------------------------------------------------------------------*/

const char *dtn_dtls_get_srtp_profile(dtn_dtls *self);

/*----------------------------------------------------------------------------*/

const char *dtn_dtls_get_verify_file(dtn_dtls *self);

/*----------------------------------------------------------------------------*/

const char *dtn_dtls_get_verify_path(dtn_dtls *self);

#endif /* dtn_dtls_h */
