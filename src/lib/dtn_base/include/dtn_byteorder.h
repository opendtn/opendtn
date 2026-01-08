/***
        ------------------------------------------------------------------------

        Copyright 2017 German Aerospace Center DLR e.V. (GSOC)

        Licensed under the Apache License, Version 2.0 (the "License");
        you may not use this file except in compliance with the License.
        You may obtain a copy of the License at

                http://www.apache.org/licenses/LICENSE-2.0

        Unless required by applicable law or agreed to in writing, software
        distributed under the License is distributed on an "AS IS" BASIS,
        WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
        See the License for the specific language governing permissions and
        limitations under the License.

        This file is part of the openvocs project. http://openvocs.org

        ------------------------------------------------------------------------
*//**
        @author         Michael Beer

        @date           2020-05-21


        ------------------------------------------------------------------------
*/
#ifndef dtn_byteorder_h
#define dtn_byteorder_h

#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "dtn_arch.h"

/**
 * Swaps 2 bytes
 */
#define DTN_SWAP_16(x) __DTN_SWAP_16(x)

/**
 * Swaps 4 bytes
 */
#define DTN_SWAP_32(x) __DTN_SWAP_32(x)

/**
 * Swaps 8 bytes
 */
#define DTN_SWAP_64(x) __DTN_SWAP_64(x)

/*****************************************************************************
                                   BIG ENDIAN
 ****************************************************************************/

/**
 * Converts 16 bit int from BIG ENDIAN to host byte order
 */
#define DTN_BE16TOH(x) __DTN_BE16TOH(x)

/**
 * Converts 32 bit int from BIG ENDIAN to host byte order
 */
#define DTN_BE32TOH(x) __DTN_BE32TOH(x)

/**
 * Converts 64 bit int from BIG ENDIAN to host bytes order
 */
#define DTN_BE64TOH(x) __DTN_BE64TOH(x)

/**
 * Converts 16 bit int from host bytes order to BIG ENDIAN
 */
#define DTN_H16TOBE(x) __DTN_H16TOBE(x)

/**
 * Converts 32 bit int from host bytes order to BIG ENDIAN
 */
#define DTN_H32TOBE(x) __DTN_H32TOBE(x)
/**
 * Converts 64 bit int from host bytes order to BIG ENDIAN
 */
#define DTN_H64TOBE(x) __DTN_H64TOBE(x)

/*****************************************************************************
                               LITTLE ENDIAN
 ****************************************************************************/

/**
 * Converts 16 bit int from LITTLE ENDIAN to host byte order
 */
#define DTN_LE16TOH(x) __DTN_LE16TOH(x)

/**
 * Converts 32 bit int from LITTLE ENDIAN to host byte order
 */
#define DTN_LE32TOH(x) __DTN_LE32TOH(x)

/**
 * Converts 64 bit int from LITTLE ENDIAN to host byte order
 */
#define DTN_LE64TOH(x) __DTN_LE64TOH(x)

/**
 * Converts 16 bit int from host bytes order to LITTLE ENDIAN
 */
#define DTN_H16TOLE(x) __DTN_H16TOLE(x)

/**
 * Converts 32 bit int from host bytes order to LITTLE ENDIAN
 */
#define DTN_H32TOLE(x) __DTN_H32TOLE(x)
/**
 * Converts 64 bit int from host bytes order to LITTLE ENDIAN
 */
#define DTN_H64TOLE(x) __DTN_H64TOLE(x)

/*---------------------------------------------------------------------------*/

/**
        Runs over an array of 16 bit words. Swaps the 2 bytes of each single
        16 bit word. Operates directly on the array given as argument.

        NOTE on error, content of the array might have become corrupted.

        @return true on success, false else.
 */
bool dtn_byteorder_swap_bytes_16_bit(int16_t *array,
                                     size_t number_16_bit_words);

/*---------------------------------------------------------------------------*/

/**
        Converts an array of 16 bit words from little endian to
        machine byte order. Operates directly on the array.

        NOTE on error, content of the array might have become corrupted.

        @return true on success, false else.
 */
bool dtn_byteorder_from_little_endian_16_bit(int16_t *array,
                                             size_t number_16_bit_words);

/*---------------------------------------------------------------------------*/

/**
        Converts an array of 16 bit wordsfrom big endian to machine byte order.
        Operates directly on the array.

        NOTE on error, content of the array might have become corrupted.

        @return true on success, false else.
 */
bool dtn_byteorder_from_big_endian_16_bit(int16_t *array,
                                          size_t number_16_bit_words);

/*---------------------------------------------------------------------------*/

/**
        Converts an array of 16 bit words from machine byte order to
        little endian. Operates directly on the array.

        NOTE on error, content of the array might have become corrupted.

        @return true on success, false else.
 */
bool dtn_byteorder_to_little_endian_16_bit(int16_t *array,
                                           size_t number_16_bit_words);

/*---------------------------------------------------------------------------*/

/**
        Converts an array of 16 bit words from machine byte order to big endian.
        Operates directly on the array.

        NOTE on error, content of the array might have become corrupted.

        @return true on success, false else.
 */
bool dtn_byteorder_to_big_endian_16_bit(int16_t *array,
                                        size_t number_16_bit_words);

/*----------------------------------------------------------------------------*/
/* 'INTERNALS'                                                                */
/*----------------------------------------------------------------------------*/
/******************************************************************************
 *                        BYTE ORDER CONVERSION MACROS
 ******************************************************************************/

#if DTN_BYTE_ORDER == DTN_BIG_ENDIAN

#define __DTN_BE16TOH(x) (x)
#define __DTN_BE32TOH(x) (x)
#define __DTN_BE64TOH(x) (x)

#define __DTN_LE16TOH(x) __DTN_SWAP_16(x)
#define __DTN_LE32TOH(x) __DTN_SWAP_32(x)
#define __DTN_LE64TOH(x) __DTN_SWAP_64(x)

#define __DTN_H16TOBE(x) (x)
#define __DTN_H32TOBE(x) (x)
#define __DTN_H64TOBE(x) (x)

#define __DTN_H16TOLE(x) __DTN_SWAP_16(x)
#define __DTN_H32TOLE(x) __DTN_SWAP_32(x)
#define __DTN_H64TOLE(x) __DTN_SWAP_64(x)

#else

#define __DTN_BE16TOH(x) __DTN_SWAP_16(x)
#define __DTN_BE32TOH(x) __DTN_SWAP_32(x)
#define __DTN_BE64TOH(x) __DTN_SWAP_64(x)

#define __DTN_LE16TOH(x) (x)
#define __DTN_LE32TOH(x) (x)
#define __DTN_LE64TOH(x) (x)

#define __DTN_H16TOBE(x) __DTN_SWAP_16(x)
#define __DTN_H32TOBE(x) __DTN_SWAP_32(x)
#define __DTN_H64TOBE(x) __DTN_SWAP_64(x)

#define __DTN_H16TOLE(x) (x)
#define __DTN_H32TOLE(x) (x)
#define __DTN_H64TOLE(x) (x)

#endif

/*---------------------------------------------------------------------------*/

#if DTN_ARCH == DTN_LINUX

#include <byteswap.h>

#define __DTN_SWAP_16(x) bswap_16(x)
#define __DTN_SWAP_32(x) bswap_32(x)
#define __DTN_SWAP_64(x) bswap_64(x)

#elif DTN_ARCH == DTN_MACOS

#include <libkern/OSByteOrder.h>

#define __DTN_SWAP_16(x) OSSwapInt16(x)
#define __DTN_SWAP_32(x) OSSwapInt32(x)
#define __DTN_SWAP_64(x) OSSwapInt64(x)

#else

#error("Unsupported architecture");

#endif

#endif /* dtn_byteorder_h */
