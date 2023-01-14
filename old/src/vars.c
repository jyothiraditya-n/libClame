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
#include <stdlib.h>
#include <string.h>

#include <LC_vars.h>

LCv_t *LC_vars;

LCv_t *LCv_new() {
	LCv_t *new = malloc(sizeof(LCv_t));
	if(!new) return NULL;

	new -> next = LC_vars;
	new -> id = "";
	new -> fmt = "";
	new -> data = NULL;

	new -> len = NULL;
	new -> min_len = 0;
	new -> max_len = 0;
	new -> size = 0;

	new -> dirty = false;

	LC_vars = new;
	return LC_vars;
}

LCv_t *LCv_get(const char *id) {
	LCv_t *var = LC_vars;

	while(var) {
		if(!strcmp(var -> id, id)) break;
		else var = var -> next;
	}

	return var;
}

void LCv_clear() {
	LCv_t *var = LC_vars;

	while(var) {
		LCv_t *next = var -> next;
		free(var);
		var = next;
	}

	LC_vars = NULL;
}