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
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include <libClame/args.h>
#include <libClame/vars.h>

static const char *name;

static bool flag = false;
static char message[4096] = "";
static int ints[4096];
static size_t length;

static void about();
static void help(int ret);
static void print_ints();

static void help_flag();
static void init();

int main(int argc, char **argv) {
	name = argv[0];
	init();

	int ret = LCa_read(argc, argv);
	if(ret != LCA_OK) help(1);

	if(flag) printf("The flag was set!\n\n");
	if(message[0]) printf("Message: %s\n\n", message);

	if(length) print_ints();
	exit(0);
}

static void about() {
	printf("  libClame: Command-line Arguments Made Easy\n");
	printf("  Copyright (C) 2021 Jyothiraditya Nellakra\n\n");

	printf("  This program is free software: you can redistribute it and/or modify\n");
	printf("  it under the terms of the GNU General Public License as published by\n");
	printf("  the Free Software Foundation, either version 3 of the License, or\n");
	printf("  (at your option) any later version.\n\n");

	printf("  This program is distributed in the hope that it will be useful,\n");
	printf("  but WITHOUT ANY WARRANTY; without even the implied warranty of\n");
	printf("  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the\n");
	printf("  GNU General Public License for more details.\n\n");

	printf("  You should have received a copy of the GNU General Public License\n");
	printf("  along with this program. If not, see <https://www.gnu.org/licenses/>.\n\n");

	exit(0);
}

static void help(int ret) {
	printf("Usage: %s [OPTIONS]\n\n", name);

	printf("Valid options are:\n");
	printf("  -a, --about		print the about dialogue\n");
	printf("  -h, --help		print this help dialogue\n\n");

	printf("  -f, --flag		set the flag\n");
	printf("  -m, --message MESSAGE	set the message to MESSAGE\n");
	printf("  -i, --ints INTS...	set the ints to INTS\n");

	printf("Happy coding! :)\n");
	exit(ret);
}

static void print_ints() {
	printf("Ints: ");

	for(size_t i = 0; i < length; i++) {
		printf("%d", ints[i]);

		if(i + 1 != length) printf(", ");
	}

	printf("\n");
}

static void help_flag() {
	help(0);
}

static void init() {
	LCa_t *arg = LCa_new();
	arg -> long_flag = "about";
	arg -> short_flag = 'a';
	arg -> pre = about;

	arg = LCa_new();
	arg -> long_flag = "help";
	arg -> short_flag = 'h';
	arg -> pre = help_flag;

	LCv_t *var = LCv_new();
	var -> id = "flag";
	var -> data = &flag;

	arg = LCa_new();
	arg -> long_flag = "flag";
	arg -> short_flag = 'f';
	arg -> var = var;
	arg -> value = true;

	var = LCv_new();
	var -> id = "message";
	var -> fmt = "%4095[^\t\n]";
	var -> data = message;

	arg = LCa_new();
	arg -> long_flag = "message";
	arg -> short_flag = 'm';
	arg -> var = var;

	var = LCv_new();
	var -> id = "ints";
	var -> fmt = "%d";
	var -> data = ints;
	var -> len = &length;
	var -> min_len = 0;
	var -> max_len = 4096;
	var -> size = sizeof(int);

	arg = LCa_new();
	arg -> long_flag = "ints";
	arg -> short_flag = 'i';
	arg -> var = var;
}