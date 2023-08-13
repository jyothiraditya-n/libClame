/* libClame: Command-line Arguments Made Easy
 * Copyright (C) 2021-2023 Jyothiraditya Nellakra
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even- the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>. */

#include <assert.h>

#define LC_REQ_VER 1
#define LC_REQ_SUBVER 0

#include <libClame.h>

int dummy_var = 0;

/* This program is all red flags, lol. */
LC_flag_t flags_1[] = {
	/* The variables are: long_flag, short_flag, function, var_ptr,
	 * var_type, value, fmt_string, arr_length, var_length, min_arr_length,
	 * max_arr_length, readonly. */

	/* --try / -t INT: This flag is wrong, as -1 isn't a valid var_type. */
	{"try", 't', NULL, &dummy_var, -1, 0, "%d", NULL, 0, 0, 0, 0}
};

LC_flag_t flags_2[] = {
	/* --try / -t INT: This flag is incorrect because the format string is
	 * a NULL string. */
	{"try", 't', NULL, &dummy_var, LC_OTHER_VAR, 0, NULL, NULL, 0, 0, 0, 0}
};

int main(int argc, char **argv) {
	/* Call LCa_read() without setting the flags. This should error out. */
	assert(LC_read(argc, argv) == LC_NO_ARGS);

	/* Call the function with the first set of wrong flags. This should
	 * produce the correct error. */
	LC_flags = flags_1;
	LC_flags_length = LC_ARRAY_LENGTH(flags_1);
	assert(LC_read(argc, argv) == LC_BAD_VAR_TYPE);

	/* Call the function with the second set of wrong flags. This should
	 * produce the correct error. */
	LC_flags = flags_2;
	LC_flags_length = LC_ARRAY_LENGTH(flags_2);
	assert(LC_read(argc, argv) == LC_NULL_FORMAT_STR);

	/* Return successfully. */
	return 0;
}
