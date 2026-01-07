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
        @file           dtn_key_store.c
        @author         Töpfer, Markus

        @date           2026-01-03


        ------------------------------------------------------------------------
*/
#include "../include/dtn_key_store.h"

#include <errno.h>
#include <dirent.h>
#include <unistd.h>
#include <libgen.h>
#include <fcntl.h>
#include <sys/stat.h>

#include <dtn_base/dtn_dict.h>
#include <dtn_base/dtn_file.h>
#include <dtn_base/dtn_string.h>
#include <dtn_base/dtn_thread_lock.h>

struct dtn_key_store {

    dtn_key_store_config config;

    dtn_thread_lock lock;
    dtn_dict *data;

};

static bool init_config(dtn_key_store_config *config){

    if (!config) goto error;

    if (0 == config->path[0])
        strncpy(config->path, DTN_DEFAULT_KEY_PATH, PATH_MAX);

    if (0 == config->limits.threadlock_timeout_usec)
        config->limits.threadlock_timeout_usec = 100000;

    return true;
error:
    return false;
}

/*
 *      ------------------------------------------------------------------------
 *
 *      GENERIC FUNCTIONS
 *
 *      ------------------------------------------------------------------------
 */

dtn_key_store *dtn_key_store_create(dtn_key_store_config config){

    dtn_key_store *self = NULL;

    if (!init_config(&config)) goto error;

    self = calloc(1, sizeof(dtn_key_store));
    if (!self) goto error;

    self->config = config;

    dtn_dict_config d_config = dtn_dict_string_key_config(255);
    d_config.value.data_function.free = dtn_buffer_free;

    self->data = dtn_dict_create(d_config);
    if (!self->data) goto error;

    if (!dtn_thread_lock_init(&self->lock, 
        self->config.limits.threadlock_timeout_usec)) goto error;

    return self;
error:
    dtn_key_store_free(self);
    return NULL;
}

/*----------------------------------------------------------------------------*/

dtn_key_store *dtn_key_store_free(dtn_key_store *self){

    if (!self) return NULL;

    dtn_thread_lock_clear(&self->lock);
    self->data = dtn_dict_free(self->data);

    self = dtn_data_pointer_free(self);
    return NULL;

}

/*
 *      ------------------------------------------------------------------------
 *
 *      Persistance FUNCTIONS
 *
 *      ------------------------------------------------------------------------
 */


static bool add_file_key(dtn_key_store *self, const char *filepath, 
    const char *filename){

    uint8_t *buf = NULL;
    dtn_buffer *buffer = NULL;
    char *key = NULL;

    if (!self || !filename || !filepath) goto error;

    size_t len = 0;

    if (DTN_FILE_SUCCESS != dtn_file_read(filepath, &buf, &len)) goto error;

    buffer = dtn_buffer_create(len);
    if (!dtn_buffer_push(buffer, buf, len)) goto error;

    key = dtn_string_dup(filename);

    if (!dtn_dict_set(self->data, key, buffer, NULL)) goto error;

    dtn_log_debug("loaded key for %s", filename);

    buf = dtn_data_pointer_free(buf);
    return true;
error:
    buf = dtn_data_pointer_free(buf);
    key = dtn_data_pointer_free(key);
    buffer = dtn_buffer_free(buffer);
    return false;
}

/*----------------------------------------------------------------------------*/

static bool read_sub_dir(dtn_key_store *self, const char *path){

    if (!self || !path) goto error;

    errno = 0;

    DIR *dp = NULL;
    struct dirent *ep = NULL;
    
    size_t dirlen = strlen(path);
    
    char filename[PATH_MAX + 1];
    memset(filename, 0, PATH_MAX + 1);

    char name[PATH_MAX + 1];
    memset(name, 0, PATH_MAX + 1);
    
    int len, i;
    
    dp = opendir(path);
    
    if (dp == NULL) {
    
      dtn_log_debug("KEY LOAD,"
                   "could not open dir %s ERRNO %i | %s",
                   path, errno, strerror(errno));
      goto error;
    }
    
    while ((ep = readdir(dp))) {

        memset(filename, 0, PATH_MAX);
        memset(name, 0, PATH_MAX);
        
        /*
         *  Do not try to read /. or /..
         */
        
        if (0 == strcmp(ep->d_name, ".") || (0 == strcmp(ep->d_name, "..")))
            continue;

        strcpy(filename, path);

        if (path[dirlen] != '/')
            strncat(filename, "/", PATH_MAX);
        
        strcat(filename, ep->d_name);

        len = strlen(ep->d_name);
        i = len;
    
        while (i > 0) {
            if (ep->d_name[i] == '.')
                break;
            i--;
        }
    
        if (i != 0)
            continue;

        snprintf(name, PATH_MAX, "%s/%s", basename((char*)path), ep->d_name);
        
        if (!add_file_key(self, filename, name)) goto error;
        
    }

    (void)closedir(dp);
    return true;

error:
    if (dp) closedir(dp);
    return false;
}

/*----------------------------------------------------------------------------*/

static bool read_dir(dtn_key_store *self, const char *path){

    errno = 0;

    DIR *dp = NULL;
    struct dirent *ep = NULL;
    
    size_t dirlen = strlen(path);
    
    char filename[PATH_MAX + 1];
    memset(filename, 0, PATH_MAX + 1);
    
    int len, i;
    
    dp = opendir(path);
    
    if (dp == NULL) {
    
      dtn_log_debug("KEY LOAD,"
                   "could not open dir %s ERRNO %i | %s",
                   path, errno, strerror(errno));
      goto error;
    }
    
    while ((ep = readdir(dp))) {
        
        struct stat st;

        memset(filename, 0, PATH_MAX);
        
        /*
         *  Do not try to read /. or /..
         */
        
        if (0 == strcmp(ep->d_name, ".") || (0 == strcmp(ep->d_name, "..")))
            continue;

        if (fstatat(dirfd(dp), ep->d_name, &st, 0) < 0)
        {
            continue;
        }

        strcpy(filename, path);

        if (path[dirlen] != '/')
            strncat(filename, "/", PATH_MAX);
        
        strcat(filename, ep->d_name);

        if (S_ISDIR(st.st_mode)) {

            read_sub_dir(self, filename);
        
        } else {

            len = strlen(ep->d_name);
            i = len;
    
            while (i > 0) {
                if (ep->d_name[i] == '.')
                    break;
                i--;
            }
    
            if (i != 0)
                continue;
        
            if (!add_file_key(self, filename, ep->d_name)) goto error;

        }
        
    }

    (void)closedir(dp);
    return true;

error:
    if (dp) closedir(dp);
    return false;
}

/*----------------------------------------------------------------------------*/

bool dtn_key_store_load(dtn_key_store *self, const char *optional_path){

    if (!self) goto error;

    const char *path = optional_path;
    if (!path) path = self->config.path;

    if (!dtn_thread_lock_try_lock(&self->lock)) goto error;

    bool result = read_dir(self, path);

    if (!dtn_thread_lock_unlock(&self->lock)){

        dtn_log_error("failed to unlock keystore");
    }

    return result;
error:
    return false;
}

/*----------------------------------------------------------------------------*/

struct container {

    dtn_key_store *self;
    const char *path;
};

/*----------------------------------------------------------------------------*/

static bool write_file(const void *key, void *val, void *data){

    if (!key) return true;

    const char *name = (const char*) key;
    dtn_buffer *buffer = dtn_buffer_cast(val);
    struct container *container = (struct container*) data;

    char path[PATH_MAX] = {0};
    snprintf(path, PATH_MAX, "%s/%s", container->path, name);

    if (DTN_FILE_SUCCESS != dtn_file_write(path, 
        buffer->start, 
        buffer->length, 
        "wr"))
        return false;

    return true;
}

/*----------------------------------------------------------------------------*/

static bool write_dir(dtn_key_store *self, const char *path){

    if (!self || !path) goto error;

    struct container container = (struct container){
        .self = self,
        .path = path
    };

    return dtn_dict_for_each(
        self->data,
        &container,
        write_file);

error:
    return false;
}

/*----------------------------------------------------------------------------*/

bool dtn_key_store_save(dtn_key_store *self, const char *optional_path){

    if (!self) goto error;

    const char *path = optional_path;
    if (!path) path = self->config.path;

    if (!dtn_thread_lock_try_lock(&self->lock)) goto error;

    bool result = write_dir(self, path);

    if (!dtn_thread_lock_unlock(&self->lock)){

        dtn_log_error("failed to unlock keystore");
    }

    return result;
error:
    return false;
}

/*
 *      ------------------------------------------------------------------------
 *
 *      KEY ACCESS FUNCTIONS
 *
 *      ------------------------------------------------------------------------
 */

dtn_buffer *dtn_key_store_get(
    dtn_key_store *self, 
    const char *destination){

    dtn_buffer *out = NULL;

    if (!self || !destination) goto error;

    if (!dtn_thread_lock_try_lock(&self->lock)) goto error;

    dtn_buffer *data = dtn_dict_get(self->data, destination);
    
    if (data)
        dtn_buffer_copy((void**)&out, data);

    if (!dtn_thread_lock_unlock(&self->lock)){

        dtn_log_error("failed to unlock keystore");
    }

    return out;

error:
    return NULL;
}

/*----------------------------------------------------------------------------*/

bool dtn_key_store_set(
    dtn_key_store *self, 
    const char *destination, 
    dtn_buffer *key){

    if (!self || !destination || !key) goto error;

    if (!dtn_thread_lock_try_lock(&self->lock)) goto error;

    char *k = dtn_string_dup(destination);
    bool result = dtn_dict_set(self->data, k, key, NULL);

    if (!dtn_thread_lock_unlock(&self->lock)){

        dtn_log_error("failed to unlock keystore");
    }

    if (!result)
        k = dtn_data_pointer_free(k);

    return result;
error:
    return false;
}