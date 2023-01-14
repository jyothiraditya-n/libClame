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
#include <LC_vars.h>

#ifndef LC_ARGS_H
#define LC_ARGS_H 1

typedef struct LCa_s {
	struct LCa_s *next;
	const char *long_flag;
	char short_flag;

	void (*pre)(void);
	void (*post)(void);

	LCv_t *var;
	bool value;

} LCa_t;

extern LCa_t *LC_args;
extern const char **LCa_noflags;
extern size_t LCa_max_noflags;

extern LCa_t *LCa_new();
extern int LCa_read(int argc, char **argv);

#define LCA_OK 0
#define LCA_BAD_FLAG 1
#define LCA_NO_FLAG 2
#define LCA_VAR_RESET 3
#define LCA_NO_VAL 4
#define LCA_BAD_VAL 5
#define LCA_LESS_VALS 6
#define LCA_MORE_VALS 7

#endif