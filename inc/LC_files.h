/* libClame: Command-line Arguments Made Easy
 * Copyright (C) 2021-2022 Jyothiraditya Nellakra
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

#include <stdint.h>

#ifndef LC_FILES_H
#define LC_FILES_H 1

#define LCF_MAGIC 0x11bc1a2e
#define LCF_BITS (sizeof(size_t) * 8)
#define LCF_VERSION 1
#define LCF_SUBVERSION 0

extern char *LCf_program_name;
extern uint8_t LCf_program_ver;
extern uint8_t LCf_program_subver;

extern int LCf_read(const char *filename);
extern int LCf_save(const char *filename);

#define LCF_OK 0
#define LCF_FILEIO_ERR 1
#define LCF_BAD_FORMAT 2
#define LCF_BAD_LCF_VER 3
#define LCF_BAD_PROG_VER 4
#define LCF_BAD_PROG_NAME 5
#define LCF_BAD_ARCH 6
#define LCF_BAD_VAR 7
#define LCF_BAD_LEN 8

#endif