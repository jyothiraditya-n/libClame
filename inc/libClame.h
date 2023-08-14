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
#ifndef LIBCLAME_H
#define LIBCLAME_H 1

/* Version Information */
#define LC_VERSION 1 /* Incremented when backwards compatibility broken. */
#define LC_SUBVERSION 2 /* Incremented when new features added. */

/* Bad version number. */
#ifdef LC_REQ_VER
#if LC_REQ_VER != LC_VERSION
#error "Incorrect libClame version."
#endif
#endif

/* Bad subversion number. */
#ifdef LC_REQ_SUBVER
#if LC_REQ_SUBVER > LC_SUBVERSION
#error "Incorrect libClame subversion."
#endif
#endif

/* Get the length of a statically-defined array: */
#define LC_ARRAY_LENGTH(array) (sizeof(array) / sizeof(*array))

/* Standard Library Includes */
#include <inttypes.h> // Needed for SIZE_MAX
#include <stdbool.h>
#include <stddef.h>

/* Flag Information Struct */
typedef struct LC_flag_s {
	/* String name and character abbreviations for the user. */
	const char *long_flag;
	char short_flag;

	/* Callback function post flag processing. */
	int (*function)(struct LC_flag_s *flag);

	/* Expected return types for the callback function. */
	#define LC_FUNCTION_OK 0
	#define LC_FUNCTION_ERR (!LC_FUNCTION_OK)

	/* Pointer to variable and it's type. */
	void *var_ptr;
	int var_type;

	/* Expected variable types. */
	#define LC_STRING_VAR 1
	#define LC_BOOL_VAR 2
	#define LC_OTHER_VAR 3

	/* Value to set boolean variables to. */
	bool value;

	/* Format string for other variables. */
	const char *fmt_string;

	/* Variable length and (optional) array length. */
	size_t *arr_length;
	size_t var_length;

	/* Array size bounds. */
	size_t min_arr_length;
	size_t max_arr_length;

	/* Boolean to pevent multiple variable definitions. */
	bool readonly;

} LC_flag_t;

/* Flags array and its length. */
extern LC_flag_t *LC_flags;
extern size_t LC_flags_length;

/* Command to begin command-line argument processing. */
extern int LC_read(int argc, char **argv);

/* Return values for LC_read(). */
#define LC_OK 0
#define LC_NO_ARGS 1
#define LC_MALLOC_ERR 2

#define LC_BAD_FLAG 3
#define LC_VAR_RESET 4
#define LC_NO_VAL 5
#define LC_BAD_VAL 6
#define LC_LESS_VALS 7
#define LC_MORE_VALS 8
#define LC_FUNC_ERR 9

#define LC_BAD_VAR_TYPE 10
#define LC_NULL_FORMAT_STR 11

/* Get an error string. */
extern const char *LC_strerror(int error);

/* Non-flag variables encountered during processing. */
extern char **LC_flagless_args;
extern size_t LC_flagless_args_length;

/* Program name set via argv[0]. */
extern char *LC_prog_name;

/* Set when a flag callback function errors out. */
extern int (*LC_err_function)();
extern int LC_function_errno;

/* End Header Guard */
#endif
