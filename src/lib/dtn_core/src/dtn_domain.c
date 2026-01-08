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
        @file           dtn_domain.c
        @author         Markus Töpfer

        @date           2020-12-18


        ------------------------------------------------------------------------
*/
#include "../include/dtn_domain.h"

#include <dtn_base/dtn_utils.h>
#include <unistd.h>

#include <dtn_base/dtn_dir.h>
#include <dtn_base/dtn_file.h>
#include <dtn_base/dtn_item_json.h>

#include <errno.h>

/*
 *      ------------------------------------------------------------------------
 *
 *      #PRIVATE FUNCTIONS
 *
 *      ------------------------------------------------------------------------
 */

static const dtn_domain init = {.magic_byte = DTN_DOMAIN_MAGIC_BYTE};

/*----------------------------------------------------------------------------*/

static bool load_certificate(SSL_CTX *ctx, const dtn_domain_config *config) {

    if (!ctx || !config)
        goto error;

    errno = 0;

    if (SSL_CTX_use_certificate_chain_file(ctx, config->certificate.cert) !=
        1) {
        dtn_log_error("%.*s failed to load certificate "
                      "from %s | error %d | %s",
                      config->name.length, config->name.start,
                      config->certificate.cert, errno, strerror(errno));
        goto error;
    }

    if (SSL_CTX_use_PrivateKey_file(ctx, config->certificate.key,
                                    SSL_FILETYPE_PEM) != 1) {
        dtn_log_error("%.*s failed to load key "
                      "from %s | error %d | %s",
                      config->name.length, config->name.start,
                      config->certificate.key, errno, strerror(errno));
        goto error;
    }

    if (SSL_CTX_check_private_key(ctx) != 1) {
        dtn_log_error("%.*s failure private key for\n"
                      "CERT | %s\n"
                      " KEY | %s",
                      config->name.length, config->name.start,
                      config->certificate.cert, config->certificate.key);
        goto error;
    }

    dtn_log_debug("DOMAIN %.*s loaded SSL certificate \n file %s\n key %s\n",
                  (int)config->name.length, config->name.start,
                  config->certificate.cert, config->certificate.key);

    return true;
error:
    return false;
}

/*----------------------------------------------------------------------------*/

static bool init_tls_context(dtn_domain *domain) {

    if (!domain)
        goto error;

    if (domain->context.tls) {
        dtn_log_error("TLS context for domain |%.*s| already set",
                      (int)domain->config.name.length,
                      domain->config.name.start);
        goto error;
    }

    domain->context.tls = SSL_CTX_new(TLS_server_method());
    if (!domain->context.tls) {
        dtn_log_error("TLS context for domain |%.*s| - failure",
                      (int)domain->config.name.length,
                      domain->config.name.start);
        goto error;
    }

    SSL_CTX_set_min_proto_version(domain->context.tls, TLS1_2_VERSION);

    if (load_certificate(domain->context.tls, &domain->config))
        return true;

    /* Failure certificate load - cleanup */
    SSL_CTX_free(domain->context.tls);
    domain->context.tls = NULL;

error:
    return false;
}

/*
 *      ------------------------------------------------------------------------
 *
 *      #GENERIC FUNCTIONS
 *
 *      ------------------------------------------------------------------------
 */

bool dtn_domain_init(dtn_domain *domain) {

    if (!domain)
        goto error;

    if (!memcpy(domain, &init, sizeof(dtn_domain)))
        goto error;

    return true;
error:
    return false;
}

/*----------------------------------------------------------------------------*/

void dtn_domain_deinit_tls_context(dtn_domain *domain) {

    if (!domain)
        goto error;

    if (domain->context.tls) {
        SSL_CTX_free(domain->context.tls);
        domain->context.tls = NULL;
    }

error:
    return;
}

/*----------------------------------------------------------------------------*/

bool dtn_domain_array_clean(size_t size, dtn_domain *array) {

    if (!array)
        goto error;

    for (size_t i = 0; i < size; i++) {
        array[i].config = (dtn_domain_config){0};
        dtn_domain_deinit_tls_context(&array[i]);
    }

    return true;
error:
    return false;
}

/*----------------------------------------------------------------------------*/

dtn_domain *dtn_domain_array_free(size_t size, dtn_domain *array) {

    if (!dtn_domain_array_clean(size, array))
        return array;

    return dtn_data_pointer_free(array);
}

/*----------------------------------------------------------------------------*/

bool dtn_domain_config_verify(const dtn_domain_config *config) {

    if (!config)
        goto error;

    if (0 == config->name.start[0])
        goto error;

    if (!dtn_dir_access_to_path(config->path)) {
        dtn_log_error("Unsufficient access to path|%s| domain |%.*s|",
                      config->path, (int)config->name.length,
                      config->name.start);
        goto error;
    }

    if (0 != dtn_file_read_check(config->certificate.cert)) {
        dtn_log_error("Unsufficient access to certificate |%s| domain |%.*s|",
                      config->certificate.cert, (int)config->name.length,
                      config->name.start);
        goto error;
    }

    if (0 != dtn_file_read_check(config->certificate.key)) {
        dtn_log_error("Unsufficient access to certificate key |%s| domain "
                      "|%.*s|",
                      config->certificate.key, (int)config->name.length,
                      config->name.start);
        goto error;
    }

    if (0 != config->certificate.ca.file[0]) {

        if (0 != dtn_file_read_check(config->certificate.ca.file)) {
            dtn_log_error("Unsufficient access to certificate authority file"
                          " |%s| domain |%.*s|",
                          config->certificate.ca.file, (int)config->name.length,
                          config->name.start);

            goto error;
        }
    }

    if (0 != config->certificate.ca.path[0]) {

        if (!dtn_dir_access_to_path(config->certificate.ca.path)) {
            dtn_log_error("Unsufficient access to certificate authority path"
                          " |%s| domain |%.*s|",
                          config->certificate.ca.path, (int)config->name.length,
                          config->name.start);

            goto error;
        }
    }

    return true;
error:
    return false;
}

/*----------------------------------------------------------------------------*/

struct container1 {

    size_t next;
    size_t max;
    dtn_domain *array;
};

/*----------------------------------------------------------------------------*/

static bool add_config_to_array(const char *key, const dtn_item *value,
                                void *data) {

    if (!key)
        return true;

    dtn_item *val = dtn_item_cast(value);
    if (!val)
        goto error;

    struct container1 *container = (struct container1 *)data;

    DTN_ASSERT(container->next < container->max);

    if (container->next == container->max)
        goto error;

    if (!dtn_domain_init(&container->array[container->next]))
        goto error;

    container->array[container->next].config =
        dtn_domain_config_from_item(value);

    container->next++;
    return true;
error:
    return false;
}

/*----------------------------------------------------------------------------*/

bool dtn_domain_load(const char *path, size_t *array_size,
                     dtn_domain **array_out) {

    dtn_item *conf = NULL;
    dtn_domain *arr = NULL;

    size_t entries = 0;

    if (!path || !array_size || !array_out)
        goto error;

    if (*array_out)
        return false;

    conf = dtn_item_json_read_dir(path, NULL);
    if (!conf) {
        dtn_log_error("Failed to read domain config dir %s", path);
        goto error;
    }

    dtn_log_debug("Reading domains at %s", path);

    entries = dtn_item_count(conf);
    if (0 == entries)
        goto error;

    size_t size = entries * sizeof(dtn_domain);

    arr = calloc(1, size);
    if (!arr)
        goto error;

    struct container1 container = (struct container1){

        .next = 0, .max = entries, .array = arr

    };

    if (!dtn_item_object_for_each(conf, add_config_to_array, &container))
        goto error;

    bool default_selected = false;

    /* Verify the domain configurations load certificates */
    for (size_t i = 0; i < entries; i++) {

        if (!dtn_domain_config_verify(&arr[i].config))
            goto cleanup;

        if (arr[i].config.is_default) {

            if (true == default_selected) {
                dtn_log_error("More than ONE domain configured as default");
                goto cleanup;
            }

            default_selected = true;
        }

        dtn_log_debug("Loaded domain %s", arr[i].config.name.start);

        if (!init_tls_context(&arr[i]))
            goto cleanup;
    }

    conf = dtn_item_free(conf);
    *array_out = arr;
    *array_size = entries;
    return true;

cleanup:

    for (size_t i = 0; i < entries; i++) {
        dtn_domain_deinit_tls_context(&arr[i]);
    }

error:
    arr = dtn_domain_array_free(entries, arr);
    conf = dtn_item_free(conf);
    if (array_size)
        *array_size = 0;
    return false;
}

/*
 *      ------------------------------------------------------------------------
 *
 *      #CONFIG
 *
 *      ------------------------------------------------------------------------
 */

dtn_domain_config dtn_domain_config_from_item(const dtn_item *value) {

    dtn_domain_config cfg = {0};
    if (!value)
        goto error;

    const dtn_item *config = dtn_item_object_get(value, "domain");
    if (!config)
        config = value;

    const char *str = NULL;
    size_t bytes = 0;

    /* domain name */

    str = dtn_item_get_string(dtn_item_object_get(config, "name"));
    if (str) {

        /* NOTE this will work for ASCII domains, but not non ascii */

        cfg.name.length = strlen(str);
        if (cfg.name.length > PATH_MAX)
            goto error;

        strncpy((char *)cfg.name.start, str, cfg.name.length + 1);
    }

    /* document root */

    str = dtn_item_get_string(dtn_item_object_get(config, "path"));
    if (str) {

        bytes = snprintf(cfg.path, PATH_MAX, "%s", str);
        if ((bytes < 1) || (bytes == PATH_MAX))
            goto error;
    }

    /* default */

    if (dtn_item_is_true(dtn_item_object_get(config, "default")))
        cfg.is_default = true;

    /* certificate */

    dtn_item *cert = dtn_item_object_get(config, "certificate");
    if (!cert)
        goto done;

    str = dtn_item_get_string(dtn_item_object_get(cert, "file"));
    if (!str)
        goto error;

    bytes = snprintf(cfg.certificate.cert, PATH_MAX, "%s", str);
    if ((bytes < 1) || (bytes == PATH_MAX))
        goto error;

    str = dtn_item_get_string(dtn_item_object_get(cert, "key"));
    if (!str)
        goto error;

    bytes = snprintf(cfg.certificate.key, PATH_MAX, "%s", str);
    if ((bytes < 1) || (bytes == PATH_MAX))
        goto error;

    /* (optional) Authority */
    dtn_item *auth = dtn_item_object_get(cert, "ca");
    if (!auth)
        goto done;

    str = dtn_item_get_string(dtn_item_object_get(auth, "file"));
    if (str) {

        bytes = snprintf(cfg.certificate.ca.file, PATH_MAX, "%s", str);
        if ((bytes < 1) || (bytes == PATH_MAX))
            goto error;
    }

    str = dtn_item_get_string(dtn_item_object_get(auth, "path"));
    if (str) {

        bytes = snprintf(cfg.certificate.ca.path, PATH_MAX, "%s", str);
        if ((bytes < 1) || (bytes == PATH_MAX))
            goto error;
    }

done:
    return cfg;
error:
    return (dtn_domain_config){0};
}

/*----------------------------------------------------------------------------*/

dtn_item *dtn_domain_config_to_item(dtn_domain_config config) {

    dtn_item *out = NULL;
    dtn_item *val = NULL;

    out = dtn_item_object();
    if (!out)
        goto error;

    /* domain name */

    if (0 == config.name.start[0]) {
        val = dtn_item_null();
    } else {
        val = dtn_item_string((char *)config.name.start);
    }

    if (!dtn_item_object_set(out, "name", val))
        goto error;

    /* document root */

    if (0 == config.path[0]) {
        val = dtn_item_null();
    } else {
        val = dtn_item_string(config.path);
    }

    if (!dtn_item_object_set(out, "path", val))
        goto error;

    /* default */

    if (config.is_default) {
        val = dtn_item_true();
        if (!dtn_item_object_set(out, "default", val))
            goto error;
    }

    /* certificate */

    val = dtn_item_object();
    if (!dtn_item_object_set(out, "certificate", val))
        goto error;

    dtn_item *cert = val;
    val = NULL;

    if (0 == config.certificate.cert[0]) {
        val = dtn_item_null();
    } else {
        val = dtn_item_string(config.certificate.cert);
    }

    if (!dtn_item_object_set(cert, "file", val))
        goto error;

    if (0 == config.certificate.key[0]) {
        val = dtn_item_null();
    } else {
        val = dtn_item_string(config.certificate.key);
    }

    if (!dtn_item_object_set(cert, "key", val))
        goto error;

    val = dtn_item_object();
    if (!dtn_item_object_set(cert, "ca", val))
        goto error;

    dtn_item *auth = val;
    val = NULL;

    if (0 == config.certificate.ca.file[0]) {
        val = dtn_item_null();
    } else {
        val = dtn_item_string(config.certificate.ca.file);
    }

    if (!dtn_item_object_set(auth, "file", val))
        goto error;

    if (0 == config.certificate.ca.path[0]) {
        val = dtn_item_null();
    } else {
        val = dtn_item_string(config.certificate.ca.path);
    }

    if (!dtn_item_object_set(auth, "path", val))
        goto error;

    return out;
error:
    out = dtn_item_free(out);
    val = dtn_item_free(val);
    return NULL;
}
