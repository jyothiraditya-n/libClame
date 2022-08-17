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

#include <stdbool.h>
#include <stdio.h>
#include <stddef.h>

#ifndef LC_LINES_H
#define LC_LINES_H

extern char *LCl_buffer;
extern size_t LCl_length;
extern bool LCl_sigint;

extern int LCl_read();
extern signed char LCl_readch();

extern int LCl_bread();
extern int LCl_fread(FILE *file, char *buffer, size_t length);

#define LCL_ERR -1
#define LCL_OK 0
#define LCL_CUT 1
#define LCL_EOF 2
#define LCL_INT 3
#define LCL_CUT_EOF 4
#define LCL_CUT_INT 5

#define LCLCH_INT -3
#define LCLCH_ERR -2
#define LCLCH_BAD -1
#define LCLCH_OK 0

#endif