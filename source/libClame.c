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

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libClame.h>

static LC_entry_t *start = NULL;
static LC_entry_t *last = NULL;

static void clear(LC_entry_t *entry) {
	entry -> next = NULL;
	entry -> instr = NULL;

	entry -> func = NULL;
	entry -> data = NULL;

	entry -> array_min = 0;
	entry -> array_max = 0;
	entry -> array_len = 0;
}

LC_entry_t *LC_new_entry() {
	if(!start) {
		start = malloc(sizeof(LC_entry_t));
		if(!start) return 0;

		last = start;
		clear(start);
		return start;
	}

	last -> next = malloc(sizeof(LC_entry_t));
	if(!last -> next) return 0;

	last = last -> next;
	clear(last);
	return last;
}

int LC_parse(int argc, char **argv) {
	if(!start) return LC_NO_ENTRIES;

	for(int i = 1; i < argc; i++) {
		LC_entry_t *entry = start;

		while(entry) {
			if(!strcmp(argv[i], entry -> instr)) break;
			else entry = entry -> next;
		}

		if(!entry) {
			fprintf(stderr,"%s: unrecognized option '%s'\n",
				argv[0], argv[i]);

			return LC_BAD_INSTR;
		}

		if(entry -> func) {
			entry -> func();
			continue;
		}

		if(!entry -> data) return LC_BAD_ENTRY;

		if(entry -> array_min <= 0 || entry -> array_max <= 0) {
			if(i + 1 == argc) {
				fprintf(stderr,
					"%s: too few arguments for '%s'\n",
					argv[0], argv[i]);

				return LC_BAD_INSTR;
			}

			i++;
			entry -> data[0] = argv[i];
			continue;
		}

		if(entry -> array_min == entry -> array_max) {
			int len = entry -> array_min;

			if(i + len >= argc) {
				fprintf(stderr,
					"%s: too few arguments for '%s'\n",
					argv[0], argv[i]);

				return LC_BAD_INSTR;
			}

			for(int j = 0; j < len; j++) {
				i++;
				entry -> data[j] = argv[i];
			}

			continue;
		}

		int limit;
		i++;

		for(limit = i; limit < argc; limit++) {
			if(!strcmp(argv[limit], "--")) break;
		}

		if(limit - i < entry -> array_min) {
			fprintf(stderr, "%s: too few arguments for '%s'\n",
				argv[0], argv[i - 1]);

			return LC_BAD_INSTR;
		}

		if(limit - i > entry -> array_max) {
			fprintf(stderr, "%s: too many arguments for '%s'\n",
				argv[0], argv[i - 1]);

			return LC_BAD_INSTR;
		}

		for(int j = 0; j + i < limit; j++) {
			entry -> data[j] = argv[i + j];
		}

		*entry -> array_len = limit - i;
		i = limit;
		continue;
	}

	return LC_OK;
}
