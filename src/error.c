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

#include <libClame.h>

int (*LC_err_function)() = NULL;
int LC_function_errno = 0;

/* Get error strings. */
const char *LC_strerror(int error) {
	/* Return the compile-time constant's name. */
	switch(error) {
		case LC_OK: return "LC_OK";
		case LC_NO_ARGS: return "LC_NO_ARGS";
		case LC_MALLOC_ERR: return "LC_MALLOC_ERR";

		case LC_BAD_FLAG: return "LC_BAD_FLAG";
		case LC_VAR_RESET: return "LC_VAR_RESET";
		case LC_NO_VAL: return "LC_NO_VAL";
		case LC_BAD_VAL: return "LC_BAD_VAL";
		case LC_LESS_VALS: return "LC_LESS_VALS";
		case LC_MORE_VALS: return "LC_MORE_VALS";
		case LC_FUNC_ERR: return "LC_FUNC_ERR";

		case LC_BAD_VAR_TYPE: return "LC_BAD_VAR_TYPE";
		case LC_NULL_FORMAT_STR: return "LC_NULL_FORMAT_STR";
	}

	/* We have an invalid error number. */
	return "LC_UNKNOWN_ERR";
}
