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

#include <libClame/args.h>
#include <libClame/vars.h>

LCa_t *LC_args;

static int proc_cmd();

static int proc_lflag();
static int proc_sflag(char c);

static int proc_arg(LCa_t *arg);
static int proc_var(LCa_t *arg);

static int get_val(LCv_t *var);
static int get_arr(LCv_t *var);

static int ac;
static char **av;
static int ai;
static size_t aj;

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

	for(ai = 1; ai < argc; ai++) {
	       int ret = proc_cmd();
	       if(ret != LCA_OK) return ret;
	}

	return LCA_OK;
}

static int proc_cmd() {
	size_t len = strlen(av[ai]);

	if(len < 2 || av[ai][0] != '-') {
		fprintf(stderr, "%s: error: "
			"'%s' is not a command.\n",
			av[0], av[ai]);

		return LCA_BAD_CMD;
	}

	if(av[ai][1] == '-') return proc_lflag();

	int i = ai;

	for(aj = 1; aj < len; aj++) {
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
		fprintf(stderr, "%s: error: "
			"unknown command '%s'.\n",
			av[0], av[ai]);

		return LCA_BAD_CMD;
	}

	return proc_arg(arg);
}

static int proc_sflag(char c) {
	LCa_t *arg;
	for(arg = LC_args; arg; arg = arg -> next) {
		if(c == arg -> short_flag) break;
	}

	if(!arg) {
		fprintf(stderr, "%s: error: "
			"unknown command '%c'.\n",
			av[0], c);

		return LCA_BAD_CMD;
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
		fprintf(stderr, "%s: error: "
			"variable '%s' has already been set.\n",
			av[0], var -> id);

		return LCA_BAD_CMD;
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
	if(ai + 1 == ac) return LCA_BAD_CMD;

	ai++;
	int ret = sscanf(av[ai], var -> fmt, var -> data);
	if(ret != 1) return LCA_BAD_CMD;

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

	ai++;

	size_t len = k - ai;

	if(len < var -> min_len) return LCA_BAD_CMD;
	if(len > var -> max_len) return LCA_BAD_CMD;

	*var -> len = len;

	for(int l = ai; l < k; l++) {
		sscanf(av[l], var -> fmt,
			var -> data + (l - ai) * var -> size);
	}

	if(end_marked) k++;

	ai = k - 1;
	var -> dirty = true;
	return LCA_OK;
}