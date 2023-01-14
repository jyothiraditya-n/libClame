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

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <LC_args.h>
#include <LC_vars.h>

LCa_t *LC_args;
const char **LCa_noflags;
size_t LCa_max_noflags;

static int proc_cmd();

static int proc_lflag();
static int proc_noflag();
static int proc_sflag(char c);

static int proc_arg(LCa_t *arg);
static int proc_var(LCa_t *arg);

static int get_val(LCv_t *var);
static int get_arr(LCv_t *var);

static int ac;
static char **av;
static int ai;
static size_t aj, aj_max;

static size_t noflags;

LCa_t *LCa_new() {
	LCa_t *new = malloc(sizeof(LCa_t));
	if(!new) return NULL;

	new -> next = LC_args;
	new -> short_flag = 0;
	new -> long_flag = "";

	new -> pre = NULL;
	new -> post = NULL;
	new -> var = NULL;
	new -> value = false;

	LC_args = new;
	return new;
}

int LCa_read(int argc, char **argv) {
	ac = argc;
	av = argv;

	bool no_more_flags = false;

	for(ai = 1; ai < argc; ai++) {
		if(!no_more_flags && !strcmp(av[ai], "--")) {
			no_more_flags = true;
			continue;
		}

		int ret = no_more_flags ? proc_noflag() : proc_cmd();
		if(ret != LCA_OK) return ret;
	}

	return LCA_OK;
}

static int proc_cmd() {
	aj_max = strlen(av[ai]);
	
	if(!strcmp(av[ai], "-")) return proc_noflag();

	if(av[ai][0] != '-') return proc_noflag();
	if(av[ai][1] == '-') return proc_lflag();

	int i = ai;

	for(aj = 1; aj < aj_max; aj++) {
		int ret = proc_sflag(av[i][aj]);
		if(ret != LCA_OK) return ret;
	}

	aj = 0;
	return LCA_OK;
}

static int proc_lflag() {
	LCa_t *arg;
	for(arg = LC_args; arg; arg = arg -> next) {
		if(!strcmp(av[ai] + 2, arg -> long_flag)) break;
	}

	if(!arg) {
		fprintf(stderr, "%s: error: unknown flag '%s'.\n",
			av[0], av[ai]);

		return LCA_BAD_FLAG;
	}

	return proc_arg(arg);
}

static int proc_noflag() {
	if(!LCa_noflags) {
		fprintf(stderr, "%s: error: argument '%s' does not have a "
			"flag.\n", av[0], av[ai]);

		return LCA_NO_FLAG;
	}

	if(noflags < LCa_max_noflags) {
		LCa_noflags[noflags] = av[ai];
		noflags++;

		return LCA_OK;
	}

	fprintf(stderr, "%s: error: too many flagless arguments.\n", av[0]);
	return LCA_NO_FLAG;
}

static int proc_sflag(char c) {
	LCa_t *arg;
	for(arg = LC_args; arg; arg = arg -> next) {
		if(c == arg -> short_flag) break;
	}

	if(!arg) {
		fprintf(stderr, "%s: error: unknown flag '%c'.\n", av[0], c);
		return LCA_BAD_FLAG;
	}

	return proc_arg(arg);
}

static int proc_arg(LCa_t *arg) {
	if(arg -> pre) arg -> pre();

	if(arg -> var) {
		int ret = proc_var(arg);
		if(ret != LCA_OK) return ret;
	}

	if(arg -> post) arg -> post();
	return LCA_OK;
}

static int proc_var(LCa_t *arg) {
	LCv_t *var = arg -> var;

	if(var -> dirty) {
		fprintf(stderr, "%s: error: variable '%s' has already "
			"been set.\n", av[0], var -> id);

		return LCA_VAR_RESET;
	}

	if(!strcmp(var -> fmt, "") && !var -> len) {
		*(bool *) var -> data = arg -> value;
		
		var -> dirty = true;
		return LCA_OK;
	}

	int ret = get_val(var);
	if(ret != LCA_OK) return ret;

	return LCA_OK;
}

static int get_val(LCv_t *var) {
	if(var -> len) return get_arr(var);
	int ret = 0;

	if(aj && aj + 1 < aj_max) {
		ret = sscanf(av[ai] + aj + 1, var -> fmt, var -> data);
		aj = aj_max;
		goto end;
	}

	if(ai + 1 == ac) {
		fprintf(stderr, "%s: error: value for variable '%s' "
			"not provided.\n", av[0], var -> id);

		return LCA_NO_VAL;
	}

	ai++;
	ret = sscanf(av[ai], var -> fmt, var -> data);

end:	if(ret != 1) {
		fprintf(stderr, "%s: error: '%s' is not a valid value "
			"for variable '%s'.\n", av[0], av[ai] , var -> id);

		return LCA_BAD_VAL;
	}

	var -> dirty = true;
	return LCA_OK;
}

static int get_arr(LCv_t *var) {
	bool end_marked = false;

	int k;
	for(k = ai + 1; k < ac; k++) {
		if(!strcmp(av[k], "--")) {
			end_marked = true;
			break;
		}

		int ret = sscanf(av[k], var -> fmt, var -> data);
		if(ret != 1) break;
	}

	ai++; size_t len = k - ai;

	if(len < var -> min_len) {
		fprintf(stderr, "%s: error: too few values for array '%s'.\n",
			av[0], var -> id);

		return LCA_MORE_VALS;
	}

	if(len > var -> max_len) {
		fprintf(stderr, "%s: error: too many arguments "
			"for array '%s'.\n", av[0], var -> id);

		return LCA_LESS_VALS;
	}

	*var -> len = len;

	for(int l = ai; l < k; l++) {
		sscanf(av[l], var -> fmt, (void *)
			((char *) var -> data + (l - ai) * var -> size));
	}

	if(end_marked) k++;

	ai = k - 1;
	var -> dirty = true;
	return LCA_OK;
}