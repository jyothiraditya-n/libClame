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

#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#include <LC_lines.h>

int LCl_fread(FILE *file, char *buffer, size_t length) {
	int c = ' ';
	size_t i = 0;
	bool clipped = false;

	*buffer = 0;

	if(feof(file)) return LCL_EOF;

	while(c != '\n' && isspace(c)) {
		c = fgetc(file);
		if(feof(file)) return LCL_EOF;
	}

	while(c != '\n') {
		if(i + 1 >= length) {
			clipped = true;
			break;
		};

		buffer[i] = (char) c;
		buffer[i + 1] = 0;
		i++;

		c = fgetc(file);
		if(feof(file)) return LCL_EOF;
	}

	if(!clipped) return LCL_OK;

	while(c != '\n') {
		c = fgetc(file);
		if(feof(file)) return LCL_CUT_EOF;
	}

	return LCL_CUT;
}