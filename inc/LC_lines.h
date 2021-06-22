/* libClame: Command-line Arguments Made Easy
 * Copyright (C) 2021 Jyothiraditya Nellakra
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

typedef struct {
        char *data;
        size_t length;

} LCl_t;

extern bool LCl_sigint;

extern int LCl_bread(char *buffer, size_t length);
extern int LCl_fread(FILE *file, char *buffer, size_t length);

extern int LCl_read(LCl_t *line);

#define LCL_ERR -1
#define LCL_OK 0
#define LCL_CUT 1
#define LCL_EOF 2
#define LCL_INT 3
#define LCL_CUT_EOF 4
#define LCL_CUT_INT 5

#endif