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
#ifndef LC_FILES_H
#define LC_FILES_H 1

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Version Information */
#define LCA_VERSION 1 /* Incremented when backwards compatibility broken. */
#define LCA_SUBVERSION 0 /* Incremented when new features added. */

typedef struct {
	/* Basic data about the variable. */
	const char *name; // We need a unique name to identify the variable.
	void *data; // A pointer to where the variable is.

	/* If the data is an array, then data will be treated as a pointer to
	 * the memory where we should store a pointer to the start of a runtime
	 * allocated array. (data will be transparently treated as a
	 * a variable of type void **) */

	/* We need a pointer to the size_t variable which stores the length of
	 * the array, which we'll set once we've read in our data. */
	size_t *arr_len; // Set to NULL if not an array.

	/* Minimum and maximum lengths, to help verify data. */
	size_t min_arr_len; // Set to 0 if not an array.
	size_t max_arr_len; // Set to 0 if not an array.

	/* The length of the array. */
	size_t var_len; // Set to the length of the variable data type.

} LCf_var_t;

#define LCF_MAGIC 0x11bc1a2e // "libClame" -> "11bc1a2e"
#define LCF_BITS (sizeof(size_t) * 8)

extern char *LCf_program_name;
extern uint8_t LCf_program_ver;
extern uint8_t LCf_program_subver;

extern int LCf_read(const char *filename, LCf_var_t *vars, size_t var_length);
extern int LCf_save(const char *filename, LCf_var_t *vars, size_t var_length);

#define LCF_OK 0
#define LCF_FILEIO_ERR 1
#define LCF_BAD_FORMAT 2
#define LCF_BAD_LCF_VER 3
#define LCF_BAD_PROG_VER 4
#define LCF_BAD_PROG_NAME 5
#define LCF_BAD_ARCH 6
#define LCF_BAD_VAR 7
#define LCF_BAD_LEN 8

/* End Header Guard */
#endif