/***
        ------------------------------------------------------------------------

        Copyright (c) 2019 German Aerospace Center DLR e.V. (GSOC)

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
        @file           dtn_file.h
        @author         Markus Toepfer
        @author         Michael Beer

        @date           2019-07-23

        @ingroup        dtn_base

        @brief          Definition of some standard base file access functions.


        ------------------------------------------------------------------------
*/
#ifndef dtn_file_h
#define dtn_file_h

#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

/*----------------------------------------------------------------------------*/

#define DTN_FILE_IS_DIR "file is directory"
#define DTN_FILE_COULD_NOT_READ "could not read"
#define DTN_FILE_UNKNOWN_ERROR "unknown error"
#define DTN_FILE_ACCESS_FAILED "no access"

/*----------------------------------------------------------------------------*/

typedef enum {

    DTN_FILE_ERROR,        // general error (unspecific, no log)
    DTN_FILE_FAILURE,      // failure during processing (err log)
    DTN_FILE_NOT_FOUND,    // not found
    DTN_FILE_NO_ACCESS,    // no access
    DTN_FILE_MEMORY_ERROR, // (e.g. input to small, out of memory)
    DTN_FILE_SUCCESS

} dtn_file_handle_state;

/*----------------------------------------------------------------------------*/

/**
 * Checks whether `path` is an entity in the file system or not.
 * It needs not be a file, just anything that lives within the file system
 * identified by `path`.
 */
bool dtn_file_exists(char const *path);

/*----------------------------------------------------------------------------*/

/**
        Check if a file is readable.

        @param path     path to file
        @returns        NULL on success
                        errormsg on error
*/
const char *dtn_file_read_check(const char *restrict path);

/*----------------------------------------------------------------------------*/

/**
        Check if a file is readable,
        and return bytes of the file.

        @param path     path to file
        @returns        bytes of file on success
                        -1 on error
*/
ssize_t dtn_file_read_check_get_bytes(const char *restrict path);

/*----------------------------------------------------------------------------*/

/**
        Read the RAW file content to buffer of size.

        This function operations in FILL / CREATE mode.

                (A)     If some buffer is provided at *buffer,
                        the buffer will be filled,
                        if *size is big enough for the file content.

                (B)     If *buffer is NULL the buffer will be created
                        to hold the file size content.

        @param path     (mandatory) path to file to read
        @param buffer   (mandatory) pointer to buffer
        @param size     (mandatory) pointer to size of buffer, will be
                        set to content size of the file

        @return dtn_FILE_SUCCESS if the full file was read.
*/
dtn_file_handle_state dtn_file_read(const char *path, uint8_t **buffer,
                                    size_t *size);

/*----------------------------------------------------------------------------*/

/**
        Read the RAW file content to buffer of size.

        This function operations in FILL / CREATE mode.

                (A)     If some buffer is provided at *buffer,
                        the buffer will be filled,
                        if *size is big enough for the file content.

                (B)     If *buffer is NULL the buffer will be created
                        to hold the file size content.

        @param path     (mandatory) path to file to read
        @param buffer   (mandatory) pointer to buffer
        @param size     (mandatory) pointer to size of buffer, will be
                        set to content size of the file

        @return dtn_FILE_SUCCESS if the full file was read.
*/
dtn_file_handle_state dtn_file_read_partial(const char *path, uint8_t **buffer,
                                            size_t *size, size_t from,
                                            size_t to, size_t *all);

/*----------------------------------------------------------------------------*/

/**
        Write some RAW content to a file.

        @param path     path to write to
        @param buffer   buffer to write
        @param size     size of the buffer to write
        @param mode     write mode to use

        @return dtn_FILE_SUCCESS if the full file was written.
*/
dtn_file_handle_state dtn_file_write(const char *path, const uint8_t *buffer,
                                     size_t size, const char *mode);

/******************************************************************************
 *                                Low-Level IO
 ******************************************************************************/

typedef enum {

    DTN_FILE_RAW,
    DTN_FILE_LITTLE_ENDIAN,
    DTN_FILE_BIG_ENDIAN,
    DTN_FILE_SWAP_BYTES

} dtn_file_byteorder;

/**
 * Read a 16 bit value from rd_ptr.
 *
 * @param out pointer to write 16 bit value to
 * @param rd_ptr pointer to read raw bytes from. Will be propagated when read.
 * @param length number of bytes readable from rd_ptr. Will be decreased by
 * number of read bytes.
 * @param byte_order
 */
bool dtn_file_get_16(uint16_t *out, uint8_t **rd_ptr, size_t *length,
                     dtn_file_byteorder byte_order);

/*----------------------------------------------------------------------------*/

/**
 * Read a 32 bit value from rd_ptr.
 *
 * @param out pointer to write 16 bit value to
 * @param rd_ptr pointer to read raw bytes from. Will be propagated when read.
 * @param length number of bytes readable from rd_ptr. Will be decreased by
 * number of read bytes.
 * @param byte_order
 */
bool dtn_file_get_32(uint32_t *out, uint8_t **rd_ptr, size_t *length,
                     dtn_file_byteorder byte_order);

/*----------------------------------------------------------------------------*/

/**
 * Write a 16 bit value to write_ptr
 *
 * It does not take a byteorder argument, because it is easy to coerce
 * with dtn_SWAP_16 / dtn_H16TOLE / dtn_H16TOBE macros:
 *
 * // Write 126 as 16 bit BIG ENDIAN
 * dtn_file_write_16(&ptr, &length, dtn_H16TOBE(126));
 *
 */
bool dtn_file_write_16(uint8_t **write_ptr, size_t *length, uint16_t value);

/*----------------------------------------------------------------------------*/

/**
 * Write a 32 bit value to write_ptr
 *
 * It does not take a byteorder argument, because it is easy to coerce
 * with dtn_SWAP_32 / dtn_H32TOLE / dtn_H32TOBE macros:
 *
 * // Write 126 as 32 bit BIG ENDIAN
 * dtn_file_write_32(&ptr, &length, dtn_H32TOBE(126));
 */
bool dtn_file_write_32(uint8_t **write_ptr, size_t *length, uint32_t value);

/*----------------------------------------------------------------------------*/

#endif /* dtn_file_h */
