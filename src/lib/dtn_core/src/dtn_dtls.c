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
        @file           dtn_dtls.c
        @author         Markus

        @date           2023-12-13


        ------------------------------------------------------------------------
*/
#include "../include/dtn_dtls.h"

#define DTN_DTLS_KEYS_QUANTITY_DEFAULT 10
#define DTN_DTLS_KEYS_LENGTH_DEFAULT 20
#define DTN_DTLS_KEYS_LIFETIME_DEFAULT 300000000
#define DTN_DTLS_RECONNECT_DEFAULT 50000 // 50ms

#define DTN_DTLS_SSL_ERROR_STRING_BUFFER_SIZE 200

/*----------------------------------------------------------------------------*/

#include <dtn_base/dtn_buffer.h>
#include <dtn_base/dtn_convert.h>
#include <dtn_base/dtn_file.h>
#include <dtn_base/dtn_random.h>
#include <dtn_base/dtn_socket.h>
#include <dtn_base/dtn_utils.h>

#include <openssl/conf.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <openssl/ssl.h>

/*----------------------------------------------------------------------------*/

static dtn_list *dtls_keys = NULL;

/*----------------------------------------------------------------------------*/

struct dtn_dtls {

    dtn_dtls_config config;

    struct {

        SSL_CTX *ctx;

        struct {

            uint32_t key_renew;

        } timer;

    } dtls;

    char fingerprint[DTN_DTLS_FINGERPRINT_MAX];
};

/*----------------------------------------------------------------------------*/

static bool init_dtls_cookie_keys(size_t quantity, size_t length) {

    if ((0 == quantity) || (0 == length))
        return false;

    if (dtls_keys) {
        dtn_list_clear(dtls_keys);
    } else {

        dtls_keys = dtn_list_create(
            (dtn_list_config){.item = dtn_buffer_data_functions()});
    }

    if (!dtls_keys)
        goto error;

    dtn_buffer *buffer = NULL;

    for (size_t i = 0; i < quantity; i++) {

        buffer = dtn_buffer_create(length);
        if (!buffer)
            goto error;

        if (!dtn_list_push(dtls_keys, buffer)) {
            buffer = dtn_buffer_free(buffer);
            goto error;
        }

        if (!dtn_random_bytes(buffer->start, buffer->capacity))
            goto error;

        buffer->length = buffer->capacity;
    }

    return true;
error:
    dtls_keys = dtn_list_free(dtls_keys);
    return false;
}

/*----------------------------------------------------------------------------*/

static bool renew_dtls_keys(uint32_t id, void *data) {

    if (0 == id)
        goto error;

    dtn_dtls *ssl = (dtn_dtls *)data;
    if (!ssl)
        goto error;

    dtls_keys = dtn_list_free(dtls_keys);

    if (!init_dtls_cookie_keys(ssl->config.dtls.keys.quantity,
                               ssl->config.dtls.keys.length)) {

        dtn_log_error("Failed to reinit DTLS key cookies");

        goto error;
    }

    if (!ssl->config.loop || !ssl->config.loop->timer.set ||
        !ssl->config.loop->timer.set(ssl->config.loop,
                                     ssl->config.dtls.keys.lifetime_usec, ssl,
                                     renew_dtls_keys)) {

        dtn_log_error("Failed to reenable DTLS key renew timer");

        goto error;
    }

    return true;
error:
    return false;
}

/*----------------------------------------------------------------------------*/

static bool write_cookie(unsigned char *cookie, unsigned int *cookie_len,
                         const dtn_buffer *key) {

    if (!cookie || !cookie_len || !key)
        return false;

    const char *array[] = {(char *)key->start};

    size_t outlen = DTLS1_COOKIE_LENGTH;

    if (!dtn_hash(DTN_HASH_MD5, array, 1, &cookie, &outlen))
        return false;

    *cookie_len = outlen;
    return true;
}

/*----------------------------------------------------------------------------*/

static bool check_cookie(const unsigned char *cookie, unsigned int cookie_len,
                         dtn_list *list) {

    if (!cookie || cookie_len < 1 || !list)
        return false;

    size_t hlen = DTN_MD5_SIZE;
    uint8_t hash[hlen];
    memset(hash, 0, hlen);

    uint8_t *ptr = hash;

    const char *array[] = {"null"};

    void *next = list->iter(list);
    dtn_buffer *buffer = NULL;

    while (next) {

        next = list->next(list, next, (void **)&buffer);
        if (!buffer)
            return false;

        array[0] = (char *)buffer->start;

        hlen = DTN_MD5_SIZE;
        memset(hash, 0, hlen);

        if (!dtn_hash(DTN_HASH_MD5, array, 1, &ptr, &hlen))
            return false;

        if (0 == memcmp(hash, cookie, hlen))
            return true;
    }

    return false;
}

/*----------------------------------------------------------------------------*/

static int generate_dtls_cookie(SSL *ssl, unsigned char *cookie,
                                unsigned int *cookie_len) {

    if (!ssl || !cookie || !cookie_len)
        goto error;

    /*
     *      To create a DTLS cookie, we choose a random
     *      key from the dtls_keys and concat the port string and
     *      some min length of the host
     *
     *      This way we do have some random UTF8 cookie,
     *      some random cookie selection, but some distinct
     *      identification over port and host.
     *
     *      The amount of random cookies, as well as the default
     *      cookie length is configurable.
     *
     *      write_cookie is an external function to support different
     *      (content) testing purposes.
     *
     */

    if (!dtls_keys) {

        if (!init_dtls_cookie_keys(DTN_DTLS_KEYS_QUANTITY_DEFAULT,
                                   DTN_DTLS_KEYS_LENGTH_DEFAULT))
            goto error;
    }

    srand(time(NULL));
    long int number = rand();
    number = (number * (dtn_list_count(dtls_keys))) / RAND_MAX;

    if (number == 0)
        number = 1;

    dtn_buffer *buffer = dtn_list_get(dtls_keys, number);
    if (!buffer)
        goto error;

    if (!write_cookie(cookie, cookie_len, buffer))
        goto error;

    return 1;
error:
    return 0;
}

/*---------------------------------------------------------------------------*/

static int verify_dtls_cookie(SSL *ssl, const unsigned char *cookie,
                              unsigned int cookie_len) {

    if (!ssl || !cookie || !dtls_keys)
        goto error;

    if (cookie_len < 1)
        goto error;

    if (!dtls_keys)
        goto error;

    if (!check_cookie(cookie, cookie_len, dtls_keys))
        goto error;

    return 1;
error:
    return 0;
}

/*----------------------------------------------------------------------------*/

static bool load_certificates(SSL_CTX *ctx, const dtn_dtls_config *config,
                              const char *type) {

    if (!ctx || !config || !type)
        goto error;

    if (SSL_CTX_use_certificate_chain_file(ctx, config->cert) != 1) {

        dtn_log_error("ICE %s config failure load certificate "
                      "from %s | error %d | %s",
                      type, config->cert, errno, strerror(errno));
        goto error;
    }

    if (SSL_CTX_use_PrivateKey_file(ctx, config->key, SSL_FILETYPE_PEM) != 1) {

        dtn_log_error("ICE %s config failure load key "
                      "from %s | error %d | %s",
                      type, config->key, errno, strerror(errno));
        goto error;
    }

    if (SSL_CTX_check_private_key(ctx) != 1) {

        dtn_log_error("ICE %s config failure private key for\n"
                      "CERT | %s\n"
                      " KEY | %s",
                      type, config->cert, config->key);
        goto error;
    }

    dtn_log_info("DTLS loaded %s certificate \n file %s\n key %s\n", type,
                 config->cert, config->key);

    return true;
error:
    return false;
}

/*----------------------------------------------------------------------------*/

static bool configure_dtls(dtn_dtls *ssl) {

    DTN_ASSERT(ssl);

    if (!ssl)
        goto error;

    if (!ssl->dtls.ctx) {
        ssl->dtls.ctx = SSL_CTX_new(DTLS_server_method());
    }

    if (!ssl->dtls.ctx)
        goto error;

    if (DTN_TIMER_INVALID != ssl->dtls.timer.key_renew) {

        ssl->config.loop->timer.unset(ssl->config.loop,
                                      ssl->dtls.timer.key_renew, NULL);
    }

    ssl->dtls.timer.key_renew = DTN_TIMER_INVALID;

    if (!load_certificates(ssl->dtls.ctx, &ssl->config, "DTLS"))
        goto error;

    if (!init_dtls_cookie_keys(ssl->config.dtls.keys.quantity,
                               ssl->config.dtls.keys.length)) {
        goto error;
    }

    SSL_CTX_set_min_proto_version(ssl->dtls.ctx, DTLS1_2_VERSION);
    SSL_CTX_set_max_proto_version(ssl->dtls.ctx, DTLS1_2_VERSION);
    SSL_CTX_set_cookie_generate_cb(ssl->dtls.ctx, generate_dtls_cookie);

    SSL_CTX_set_cookie_verify_cb(ssl->dtls.ctx, verify_dtls_cookie);

    ssl->dtls.timer.key_renew = ssl->config.loop->timer.set(
        ssl->config.loop, ssl->config.dtls.keys.lifetime_usec, ssl,
        renew_dtls_keys);

    if (DTN_TIMER_INVALID == ssl->dtls.timer.key_renew)
        goto error;

    if (0 ==
        SSL_CTX_set_tlsext_use_srtp(ssl->dtls.ctx, ssl->config.srtp.profile)) {

    } else {

        goto error;
    }

    return true;
error:
    return false;
}

/*
 *      ------------------------------------------------------------------------
 *
 *      #FINGERPRINT
 *
 *      ------------------------------------------------------------------------
 */

static char *fingerprint_format_RFC8122(const char *source, size_t length) {

    /*
    hash-func              =  "sha-1" / "sha-224" / "sha-256" /
                         "sha-384" / "sha-512" /
                         "md5" / "md2" / token
                         ; Additional hash functions can only come
                         ; from updates to RFC 3279

    fingerprint            =  2UHEX *(":" 2UHEX)
                         ; Each byte in upper-case hex, separated
                         ; by colons.

    UHEX                   =  DIGIT / %x41-46 ; A-F uppercase
    */

    char *fingerprint = NULL;

    if (!source)
        return NULL;

    size_t hex_len = 2 * length + 1;
    char hex[hex_len + 1];
    memset(hex, 0, hex_len);
    uint8_t *ptr = (uint8_t *)hex;

    if (!dtn_convert_binary_to_hex((uint8_t *)source, length, &ptr, &hex_len))
        goto error;

    size_t size = hex_len + length;
    fingerprint = calloc(size + 1, sizeof(char));

    for (size_t i = 0; i < length; i++) {

        fingerprint[(i * 3) + 0] = toupper(hex[(i * 2) + 0]);
        fingerprint[(i * 3) + 1] = toupper(hex[(i * 2) + 1]);
        if (i < length - 1)
            fingerprint[(i * 3) + 2] = ':';
    }

    return fingerprint;

error:
    if (fingerprint)
        free(fingerprint);
    return NULL;
}

/*----------------------------------------------------------------------------*/

static char *X509_fingerprint_create(const X509 *cert, dtn_hash_function type) {

    unsigned char mdigest[EVP_MAX_MD_SIZE] = {0};
    unsigned int mdigest_size = 0;

    char *fingerprint = NULL;

    const EVP_MD *func = dtn_hash_function_to_EVP(type);
    if (!func || !cert)
        return NULL;

    if (0 < X509_digest(cert, func, mdigest, &mdigest_size)) {
        fingerprint = fingerprint_format_RFC8122((char *)mdigest, mdigest_size);
    }

    return fingerprint;
}

/*----------------------------------------------------------------------------*/

static bool create_fingerprint_cert(const char *path, dtn_hash_function hash,
                                    char *out) {

    char *x509_fingerprint = NULL;

    if (!path || !out)
        goto error;

    const char *hash_string = dtn_hash_function_to_RFC8122_string(hash);
    if (!hash_string)
        goto error;

    X509 *x = NULL;

    FILE *fp = fopen(path, "r");

    if (!PEM_read_X509(fp, &x, NULL, NULL)) {
        fclose(fp);
        goto error;
    }

    x509_fingerprint = X509_fingerprint_create(x, hash);
    fclose(fp);
    X509_free(x);

    if (!x509_fingerprint)
        goto error;

    size_t size = strlen(x509_fingerprint) + strlen(hash_string) + 2;

    if (size >= DTN_DTLS_FINGERPRINT_MAX)
        goto error;

    memset(out, 0, DTN_DTLS_FINGERPRINT_MAX);

    if (!snprintf(out, DTN_DTLS_FINGERPRINT_MAX, "%s %s", hash_string,
                  x509_fingerprint))
        goto error;

    out[DTN_DTLS_FINGERPRINT_MAX - 1] = 0;
    x509_fingerprint = dtn_data_pointer_free(x509_fingerprint);
    return true;
error:
    x509_fingerprint = dtn_data_pointer_free(x509_fingerprint);
    return false;
}

/*
 *      ------------------------------------------------------------------------
 *
 *      GENERIC FUNCTIONS
 *
 *      ------------------------------------------------------------------------
 */

dtn_dtls_config dtn_dtls_config_from_json(const dtn_item *input) {

    dtn_dtls_config out = (dtn_dtls_config){0};
    if (!input)
        goto error;

    const dtn_item *conf = dtn_item_get(input, "/ssl/dtls");

    if (!conf)
        conf = input;

    /*
     *      We perform a read access on the cert and key,
     *      to ensure the config is valid.
     */

    const char *cert =
        dtn_item_get_string(dtn_item_object_get(conf, "certificate"));

    const char *key = dtn_item_get_string(dtn_item_object_get(conf, "key"));

    if (!cert || !key) {
        dtn_log_error("SSL JSON config without cert or key");
        goto error;
    }

    size_t bytes = 0;

    const char *error = dtn_file_read_check(cert);

    if (error) {
        dtn_log_error("SSL config cannot read certificate "
                      "at %s error %s",
                      cert, error);
        goto error;
    }

    error = dtn_file_read_check(key);

    if (error) {
        dtn_log_error("SSL config cannot read key "
                      "at %s error %s",
                      key, error);
        goto error;
    }

    bytes = snprintf(out.cert, PATH_MAX, "%s", cert);
    if (bytes != strlen(cert))
        goto error;

    bytes = snprintf(out.key, PATH_MAX, "%s", key);
    if (bytes != strlen(key))
        goto error;

    const char *string =
        dtn_item_get_string(dtn_item_object_get(conf, "ca file"));

    if (string) {

        error = dtn_file_read_check(string);

        if (error) {
            dtn_log_error("SSL config cannot read CA FILE "
                          "at %s error %s",
                          string, error);
            goto error;
        }

        bytes = snprintf(out.ca.file, PATH_MAX, "%s", string);
        if (bytes != strlen(string))
            goto error;
    }

    string = dtn_item_get_string(dtn_item_object_get(conf, "ca path"));

    if (string) {

        error = dtn_file_read_check(string);

        if (!error) {

            dtn_log_error("SSL config wrong path for CA PATH "
                          "at %s error %s",
                          string, error);
            goto error;

        } else if (0 != strcmp(error, DTN_FILE_IS_DIR)) {

            dtn_log_error("SSL config wrong path for CA PATH "
                          "at %s error %s",
                          string, error);
            goto error;
        }

        bytes = snprintf(out.ca.path, PATH_MAX, "%s", string);
        if (bytes != strlen(string))
            goto error;
    }

    dtn_item *keys = dtn_item_object_get(conf, "keys");
    if (!keys)
        goto key_defaults;

    out.dtls.keys.quantity =
        dtn_item_get_number(dtn_item_object_get(keys, "quantity"));

    out.dtls.keys.length =
        dtn_item_get_number(dtn_item_object_get(keys, "length"));

    out.dtls.keys.lifetime_usec =
        dtn_item_get_number(dtn_item_object_get(keys, "lifetime"));

key_defaults:

    if (0 == out.dtls.keys.quantity)
        out.dtls.keys.quantity = DTN_DTLS_KEYS_QUANTITY_DEFAULT;

    if (0 == out.dtls.keys.length)
        out.dtls.keys.length = DTN_DTLS_KEYS_LENGTH_DEFAULT;

    if (DTLS1_COOKIE_LENGTH < out.dtls.keys.length)
        out.dtls.keys.length = DTLS1_COOKIE_LENGTH;

    if (0 == out.dtls.keys.lifetime_usec)
        out.dtls.keys.lifetime_usec = DTN_DTLS_KEYS_LIFETIME_DEFAULT;

    return out;

error:
    return (dtn_dtls_config){0};
}

/*----------------------------------------------------------------------------*/

dtn_dtls *dtn_dtls_create(dtn_dtls_config config) {

    dtn_dtls *self = NULL;

    if (!config.loop)
        goto error;

    if (0 == config.cert[0])
        goto error;

    if (0 == config.key[0])
        goto error;

    self = calloc(1, sizeof(dtn_dtls));
    if (!self)
        goto error;

    if (0 == config.srtp.profile[0])
        snprintf(config.srtp.profile, DTN_DTLS_PROFILE_MAX,
                 DTN_DTLS_SRTP_PROFILES);

    if (0 == config.dtls.keys.quantity)
        config.dtls.keys.quantity = DTN_DTLS_KEYS_QUANTITY_DEFAULT;

    if (0 == config.dtls.keys.length)
        config.dtls.keys.length = DTN_DTLS_KEYS_LENGTH_DEFAULT;

    if (DTLS1_COOKIE_LENGTH < config.dtls.keys.length)
        config.dtls.keys.length = DTLS1_COOKIE_LENGTH;

    if (0 == config.dtls.keys.lifetime_usec)
        config.dtls.keys.lifetime_usec = DTN_DTLS_KEYS_LIFETIME_DEFAULT;

    if (0 == config.reconnect_interval_usec)
        config.reconnect_interval_usec = DTN_DTLS_RECONNECT_DEFAULT;

    self->config = config;

    // init openssl
    SSL_library_init();
    SSL_load_error_strings();

    if (!configure_dtls(self))
        goto error;

    if (!create_fingerprint_cert(self->config.cert, DTN_HASH_SHA256,
                                 self->fingerprint))
        goto error;

    return self;
error:
    dtn_dtls_free(self);
    return NULL;
}

/*----------------------------------------------------------------------------*/

dtn_dtls *dtn_dtls_free(dtn_dtls *self) {

    if (!self)
        goto error;

    if (self->dtls.ctx) {
        SSL_CTX_free(self->dtls.ctx);
        self->dtls.ctx = NULL;
    }

    // free used timers
    if (DTN_TIMER_INVALID != self->dtls.timer.key_renew) {
        if (self->config.loop && self->config.loop->timer.unset)
            self->config.loop->timer.unset(self->config.loop,
                                           self->dtls.timer.key_renew, NULL);
    }

    self->dtls.timer.key_renew = DTN_TIMER_INVALID;

    // clean other openssl initializations
    // TBD check if this are all required
    // FIPS_mode_set(0);
    EVP_cleanup();
    CRYPTO_cleanup_all_ex_data();
    ERR_free_strings();
    CONF_modules_finish();
    RAND_cleanup();

    // free global DTLS keys
    dtls_keys = dtn_list_free(dtls_keys);

    self = dtn_data_pointer_free(self);

error:
    return self;
}

/*----------------------------------------------------------------------------*/

const char *dtn_dtls_get_fingerprint(const dtn_dtls *ssl) {

    if (!ssl)
        goto error;

    return ssl->fingerprint;

error:
    return NULL;
}

/*---------------------------------------------------------------------------*/

const char *dtn_dtls_type_to_string(dtn_dtls_type type) {

    const char *out = NULL;

    switch (type) {

    case DTN_DTLS_ACTIVE:
        out = DTN_KEY_ACTIVE;
        break;

    case DTN_DTLS_PASSIVE:
        out = DTN_KEY_PASSIVE;
        break;

    default:
        out = DTN_KEY_UNSET;
        break;
    }

    return out;
}

/*----------------------------------------------------------------------------*/

SSL_CTX *dtn_dtls_get_ctx(dtn_dtls *self) {

    if (!self)
        return NULL;
    return self->dtls.ctx;
}

/*----------------------------------------------------------------------------*/

const char *dtn_dtls_get_srtp_profile(dtn_dtls *self) {

    if (!self)
        return NULL;
    return self->config.srtp.profile;
}

/*----------------------------------------------------------------------------*/
const char *dtn_dtls_get_verify_file(dtn_dtls *self) {

    if (!self)
        return NULL;
    return self->config.ca.file;
}

/*----------------------------------------------------------------------------*/

const char *dtn_dtls_get_verify_path(dtn_dtls *self) {

    if (!self)
        return NULL;
    return self->config.ca.path;
}

/*----------------------------------------------------------------------------*/
