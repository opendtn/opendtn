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

        This file is part of the opendtn project. https://opendtn.com

        ------------------------------------------------------------------------
*//**
        @file           dtn_aes_key_wrap.c
        @author         Töpfer, Markus

        @date           2025-12-30


        ------------------------------------------------------------------------
*/
#include "../include/dtn_aes_key_wrap.h"

#include <string.h>
#include <arpa/inet.h>
#include <openssl/evp.h>
#include <openssl/err.h>

static const unsigned char iv[8] = { 0xa6, 
                                     0xa6, 
                                     0xa6, 
                                     0xa6,
                                     0xa6, 
                                     0xa6, 
                                     0xa6, 
                                     0xa6
};

/*----------------------------------------------------------------------------*/

bool dtn_aes_key_wrap(
    uint8_t *plaintext, 
    size_t plaintext_size, 
    uint8_t *out,
    size_t *out_size,
    uint8_t *key, 
    size_t key_size){

    EVP_CIPHER_CTX *ctx = NULL;   
    const EVP_CIPHER *cipher = NULL;    

    if (!plaintext || !out || !key) goto error;

    size_t keysize = key_size * 8;

    switch(keysize)
    {
        case 128:
            cipher = EVP_aes_128_wrap();
            break;
        case 192:
            cipher = EVP_aes_192_wrap();
            break;
        case 256:
            cipher = EVP_aes_256_wrap();
            break;
        default:
            goto error;
    }

    int len = *out_size;

    if (!(ctx = EVP_CIPHER_CTX_new())) goto error;

    if (!EVP_EncryptInit_ex(ctx, cipher, NULL, key, iv)){
        EVP_CIPHER_CTX_free(ctx);
        goto error;
    }

    EVP_CIPHER_CTX_set_padding(ctx, 0);

    if (!EVP_EncryptUpdate(ctx,
                           (unsigned char*) out,
                           &len,
                           plaintext,
                           plaintext_size)){
        EVP_CIPHER_CTX_free(ctx);
        goto error;
    }

    *out_size = len;

    if (!EVP_EncryptFinal_ex(ctx,
                             out,
                             &len)){
        EVP_CIPHER_CTX_free(ctx);
        goto error;
    }

    EVP_CIPHER_CTX_free(ctx);

    return true;
error:
    return false;
}

/*----------------------------------------------------------------------------*/

bool dtn_aes_key_unwrap(
    uint8_t *plaintext, 
    size_t plaintext_size, 
    uint8_t *out,
    size_t *out_size,
    uint8_t *key, 
    size_t key_size){

    EVP_CIPHER_CTX *ctx = NULL;   
    const EVP_CIPHER *cipher = NULL;    

    if (!plaintext || !out || !key) goto error;

    size_t keysize = key_size * 8;

    switch(keysize)
    {
        case 128:
            cipher = EVP_aes_128_wrap();
            break;
        case 192:
            cipher = EVP_aes_192_wrap();
            break;
        case 256:
            cipher = EVP_aes_256_wrap();
            break;
        default:
            goto error;
    }

    int len = *out_size;

    if (!(ctx = EVP_CIPHER_CTX_new())) goto error;

    if (!EVP_DecryptInit_ex(ctx, cipher, NULL, key, iv)){
        EVP_CIPHER_CTX_free(ctx);
        goto error;
    }

    EVP_CIPHER_CTX_set_padding(ctx, 0);

    if (!EVP_DecryptUpdate(ctx,
                           (unsigned char*) out,
                           &len,
                           plaintext,
                           plaintext_size)){
        EVP_CIPHER_CTX_free(ctx);
        goto error;
    }

    *out_size = len;

    if (!EVP_DecryptFinal_ex(ctx,
                             out,
                             &len)){
        EVP_CIPHER_CTX_free(ctx);
        goto error;
    }

    EVP_CIPHER_CTX_free(ctx);

    return true;
error:
    return false;
}