/* libClame: Command-line Arguments Made Easy
 * Copyright (C) 2021-2023 Jyothiraditya Nellakra
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>. */

/* Begin Header Guard */
#ifndef LC_H
#define LC_H 1

/* Version Information */
#define LC_VERSION 1 /* Incremented when backwards compatibility broken. */
#define LC_SUBVERSION 0 /* Incremented when new features added. */

/* Useful macros that are used throughout the library. */
/* Get the length of a statically-defined array: */
#define LC_ARRAY_LENGTH(array) (sizeof(array) / sizeof(*array))

/* We can actually mostly figure out the endianness of our processor at compile
 * time, though this is not bulletproof. The below code is from Adam Rosenfield
 * over at Stack Overflow. [1] */

#define LC_BIG_ENDIAN 1
#define LC_LITTLE_ENDIAN 2

#if (defined(__BYTE_ORDER) && __BYTE_ORDER == __BIG_ENDIAN) || \
        defined(__BIG_ENDIAN__) || \
        defined(__ARMEB__) || \
        defined(__THUMBEB__) || \
        defined(__AARCH64EB__) || \
        defined(_MIBSEB) || defined(__MIBSEB) || defined(__MIBSEB__)

#define LC_ENDIANNESS LC_BIG_ENDIAN

#elif (defined(__BYTE_ORDER) && __BYTE_ORDER == __LITTLE_ENDIAN) || \
        defined(__LITTLE_ENDIAN__) || \
        defined(__ARMEL__) || \
        defined(__THUMBEL__) || \
        defined(__AARCH64EL__) || \
        defined(_MIPSEL) || defined(__MIPSEL) || defined(__MIPSEL__)

#define LC_ENDIANNESS LC_LITTLE_ENDIAN

#else // We can determine this at runtime.

/* This function will return either LC_BIG_ENDIAN or LC_LITTLE_ENDIAN. */
extern int LC_endianness();

#define LC_ENDIANNESS LC_endianness()

#endif

/* End Header Guard */
#endif

/* [1]: Adam Rosenfield's answer over at <https://stackoverflow.com/questions/
 *      4239993/determining-endianness-at-compile-time> */