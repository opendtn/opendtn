/***
        ------------------------------------------------------------------------

        Copyright (c) 2020 German Aerospace Center DLR e.V. (GSOC)

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
        @file           dtn_domain.h
        @author         Markus Töpfer

        @date           2020-12-18

        dtn_domain is a structure intendet do be used in dtn_webserver as an array
        of domains and to hold all domain specific data.

        ------------------------------------------------------------------------
*/
#ifndef dtn_domain_h
#define dtn_domain_h

#include <limits.h>
#include <dtn_base/dtn_dict.h>
#include <dtn_base/dtn_item.h>

#include <openssl/conf.h>
#include <openssl/ssl.h>

typedef struct dtn_domain dtn_domain;
typedef struct dtn_domain_config dtn_domain_config;

#define DTN_DOMAIN_MAGIC_BYTE 0x0D0C

/*----------------------------------------------------------------------------*/

struct dtn_domain_config {

  /* Support UTF8 here for non ASCCI names */

  struct {

    uint8_t start[PATH_MAX];
    size_t length;

  } name;

  /* Default MAY be set in exactly of the domains,
   * of some domain configuration,
   * to allow IP based HTTPs access instead of SNI. */
  bool is_default;

  /* Certificate settings */

  struct {

    char cert[PATH_MAX];
    char key[PATH_MAX];

    struct {

      char file[PATH_MAX]; // path to CA verify file
      char path[PATH_MAX]; // path to CAs to use

    } ca;

  } certificate;

  /* Document root path */

  char path[PATH_MAX];
};

/*----------------------------------------------------------------------------*/

struct dtn_domain {

  const uint16_t magic_byte;
  dtn_domain_config config;

  struct {

    SSL_CTX *tls;

  } context;

};

/*
 *      ------------------------------------------------------------------------
 *
 *      GENERIC FUNCTIONS
 *
 *      ------------------------------------------------------------------------
 */

/**
    This function will set the magic byte,
    and clean all other values of the domain struct.
*/
bool dtn_domain_init(dtn_domain *domain);

/*----------------------------------------------------------------------------*/

/**
    Verify existence and read access to all required path parameter set in
    the config.
*/
bool dtn_domain_config_verify(const dtn_domain_config *config);

/*----------------------------------------------------------------------------*/

/**
    This function will read all JSON files at path and create the
    dtn_domain array.

    It will initialized the domains TLS context and load the certificates.

    @param path     PATH to read
    @param size     pointer to size of array to be set
    @param array    pointer to array to create *array MUST be NULL!

    @retruns true if the array was created and all configs are set in array
    @returns false if ANY TLS context was NOT initialized
*/
bool dtn_domain_load(const char *path, size_t *size, dtn_domain **array);

/*----------------------------------------------------------------------------*/

/**
    This funcion will walk some array of dtn_domain entires and
    clean all data contained, before actually freeing the array.

    It SHOULD be called before the array will run out of scope to free all
    SSL related content.

    @param size     size of the array
    @param array    pointer of the array
*/
bool dtn_domain_array_clean(size_t size, dtn_domain *array);

/*----------------------------------------------------------------------------*/

/**
    This function will clear all entry of some domain array, and free the
    pointer to the array.

    @param size     size of the array
    @param array    pointer of the array
*/
dtn_domain *dtn_domain_array_free(size_t size, dtn_domain *array);

/*----------------------------------------------------------------------------*/

/**
    This function will unset the TLS context of a domain.
    It MUST be called to free up the SSL context.
*/
void dtn_domain_deinit_tls_context(dtn_domain *domain);

/*----------------------------------------------------------------------------*/

/**
    Create a config out of some JSON object.

    Will check for KEY dtn_KEY_DOMAIN or try to parse the input direct.

    Expected input:

    {
        "name":"<some name>",
        "path":"<path document root>",
        "certificate":
        {
            "authority":
            {
                "file":"<path ca file>",
                "path":"<path ca dir>"
            },
            "file":"<path certificate file>",
            "key":"<path certificate key>"
        }
    }
    @param value    JSON value to parse
*/
dtn_domain_config dtn_domain_config_from_item(const dtn_item *value);

/*----------------------------------------------------------------------------*/

/**
    Create the JSON config of form @see dtn_domain_config_from_json
*/
dtn_item *dtn_domain_config_to_item(dtn_domain_config config);

#endif /* dtn_domain_h */
