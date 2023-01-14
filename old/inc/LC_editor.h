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
#include <stddef.h>

#ifndef LC_EDITOR_H
#define LC_EDITOR_H

extern const char *LCe_banner;
extern char *LCe_buffer;
extern size_t LCe_length;

extern bool LCe_sigint;
extern bool LCe_dirty;

extern int LCe_edit();

#define LCE_OK 0
#define LCE_ERR 1

#endif