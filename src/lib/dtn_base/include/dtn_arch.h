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

        @author         Michael J. Beer
        @author         Markus Toepfer

        This header only discovers the architecture we run on
        and sets the DTN_ARCH macro accordingly

        ------------------------------------------------------------------------
*/
#ifndef DTN_ARCH_H
#define DTN_ARCH_H

/*----------------------------------------------------------------------------*/

// Just in case - all our current architectures already use 2's complement
#define DTN_TWOS_COMPLEMENT(x)                                                 \
    _Generic((x),                                                              \
        int: (unsigned int)(x),                                                \
        unsigned int: (x),                                                     \
        long int: (unsigned long int)(x),                                      \
        unsigned long int: (x),                                                \
        long long int: (unsigned long long int)(x),                            \
        unsigned long long int: (x),                                           \
        short int: (unsigned short int)(x),                                    \
        unsigned short int: (short int)(x))

#define DTN_BIG_ENDIAN 128
#define DTN_LITTLE_ENDIAN 1

/**
 * This macro denotes the byte order of the machine.
 * It either resolves to DTN_BIG_ENDIAN or DTN_LITTLE_ENDIAN
 */
#define DTN_BYTE_ORDER __DTN_BYTE_ORDER

/*----------------------------------------------------------------------------*/

#define DTN_POSIX 1
#define DTN_LINUX 2
#define DTN_MACOS 3

/* Just in case someone wants to define DTN_ARCH via command line of
 * DTN_base/constants.h */
#ifndef DTN_ARCH

#define DTN_ARCH __DTN_ARCH

#endif

/*----------------------------------------------------------------------------*/

#define DTN_CLANG 1
#define DTN_GCC 2
#define DTN_COMPILER_UNKNOWN 3

#define DTN_COMPILER __DTN_COMPILER

/******************************************************************************
 *                                 INTERNALS
 ******************************************************************************/

#if defined(__linux__)

#define __DTN_ARCH DTN_LINUX

#elif defined(__APPLE__)

#define __DTN_ARCH DTN_MACOS

#else

#error("Unsupported architecture")

#endif

/******************************************************************************
 *                                 Byte order
 ******************************************************************************/

#if (defined(__BYTE_ORDER) && __BYTE_ORDER == __BIG_ENDIAN) ||                 \
    (defined(BYTE_ORDER) && BYTE_ORDER == BIG_ENDIAN) ||                       \
    (defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__) ||     \
    defined(__BIG_ENDIAN__) || defined(__ARMEB__) || defined(__THUMBEB__) ||   \
    defined(__AARCH64EB__) || defined(_MIBSEB) || defined(__MIBSEB) ||         \
    defined(__MIBSEB__)

#define __DTN_BYTE_ORDER DTN_BIG_ENDIAN

#elif defined(__BYTE_ORDER) && __BYTE_ORDER == __LITTLE_ENDIAN ||              \
    (defined(BYTE_ORDER) && BYTE_ORDER == LITTLE_ENDIAN) ||                    \
    (defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__) ||  \
    defined(__LITTLE_ENDIAN__) || defined(__ARMEL__) ||                        \
    defined(__THUMBEL__) || defined(__AARCH64EL__) || defined(_MIPSEL) ||      \
    defined(__MIPSEL) || defined(__MIPSEL__)

#define __DTN_BYTE_ORDER DTN_LITTLE_ENDIAN

#else

#error("Cannot determine byte-order")

#endif /* byte order */

/*****************************************************************************
                                    compiler
 ****************************************************************************/

#if defined(__clang__)

#define __DTN_COMPILER DTN_CLANG

#elif defined(__GNUC__)

#define __DTN_COMPILER DTN_GCC

#else

#define __DTN_COMPILER DTN_COMPILER_UNKNOWN

#endif /* compiler */

/*----------------------------------------------------------------------------*/
#endif /* DTN_ARCH_H */
