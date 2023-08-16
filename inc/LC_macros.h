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
#ifndef LC_MACROS_H
#define LC_MACROS_H 1

/* Main Program Header. */
#include <libClame.h>

/* The LC_flag_t variables are: long_flag, short_flag, function, var_ptr,
 * var_type, value, fmt_string, arr_length, var_length, min_arr_length,
 * max_arr_length, readonly. */

/* Flag to call a function. */
#define LC_MAKE_CALL(lflag, sflag, func) \
	{lflag, sflag, func, NULL, 0, 0, NULL, NULL, 0, 0, 0, 0}

/* Flag to set a boolean to a given value. */
#define LC_MAKE_BOOL(lflag, sflag, var, val) \
	{lflag, sflag, NULL, &var, LC_BOOL_VAR, val, NULL, NULL, 0, 0, 0, \
		false}

#define LC_MAKE_BOOL_F(lflag, sflag, var, val, func) \
	{lflag, sflag, func, &var, LC_BOOL_VAR, val, NULL, NULL, 0, 0, 0, \
		false}

/* Flags to get config string(s). */
#define LC_MAKE_STRING(lflag, sflag, var) \
	{lflag, sflag, NULL, &var, LC_STRING_VAR, 0, NULL, NULL, 0, 0, 0, \
		false}

#define LC_MAKE_STRING_F(lflag, sflag, var, func) \
	{lflag, sflag, func, &var, LC_STRING_VAR, 0, NULL, NULL, 0, 0, 0, \
		false}

#define LC_MAKE_STRING_ARR(lflag, sflag, arr, len) \
	{lflag, sflag, NULL, &arr, LC_STRING_VAR, 0, NULL, &len, 0, 0, \
		SIZE_MAX, false}

#define LC_MAKE_STRING_ARR_F(lflag, sflag, arr, len, func) \
	{lflag, sflag, func, &arr, LC_STRING_VAR, 0, NULL, &len, 0, 0, \
		SIZE_MAX, false}

#define LC_MAKE_STRING_ARR_BOUNDED(lflag, sflag, arr, len, min_len, max_len) \
	{lflag, sflag, NULL, &arr, LC_STRING_VAR, 0, NULL, &len, 0, min_len, \
		max_len, false}

#define LC_MAKE_STRING_ARR_BOUNDED_F(lflag, sflag, arr, len, min, max, func) \
	{lflag, sflag, func, &arr, LC_STRING_VAR, 0, NULL, &len, 0, min, \
		max, false}

/* Flags to get variables or arrays of other types. */
#define LC_MAKE_VAR(lflag, sflag, var, fmt) \
	{lflag, sflag, NULL, &var, LC_OTHER_VAR, 0, fmt, NULL, sizeof(var), \
		0, SIZE_MAX, false}

#define LC_MAKE_VAR_F(lflag, sflag, var, fmt, func) \
	{lflag, sflag, NULL, &var, LC_OTHER_VAR, 0, fmt, NULL, sizeof(var), \
		0, SIZE_MAX, false}

#define LC_MAKE_ARR(lflag, sflag, arr, fmt, len) \
	{lflag, sflag, NULL, &arr, LC_OTHER_VAR, 0, fmt, &len, sizeof(*arr), \
		0, SIZE_MAX, false}

#define LC_MAKE_ARR_F(lflag, sflag, arr, fmt, len, func) \
	{lflag, sflag, func, &arr, LC_OTHER_VAR, 0, fmt, &len, sizeof(*arr), \
		0, SIZE_MAX, false}

#define LC_MAKE_ARR_BOUNDED(lflag, sflag, arr, fmt, len, min_len, max_len) \
	{lflag, sflag, NULL, &arr, LC_OTHER_VAR, 0, fmt, &len, sizeof(*arr), \
		min_len, max_len, false}

#define LC_MAKE_ARR_BOUNDED_F(lflag, sflag, arr, fmt, len, min, max, func) \
	{lflag, sflag, func, &arr, LC_OTHER_VAR, 0, fmt, &len, sizeof(*arr), \
		min, max, false}

/* End Header Guard */
#endif
