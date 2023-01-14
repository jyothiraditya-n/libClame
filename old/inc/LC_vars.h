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

#ifndef LC_VARS_H
#define LC_VARS_H 1

typedef struct LCv_s {
	struct LCv_s *next;

	const char *id;
	const char *fmt;
	void *data;

	size_t *len;
	size_t min_len;
	size_t max_len;
	size_t size;

	bool dirty;

} LCv_t;

extern LCv_t *LC_vars;

extern LCv_t *LCv_new();
extern LCv_t *LCv_get(const char *id);

extern void LCv_clear();

#endif