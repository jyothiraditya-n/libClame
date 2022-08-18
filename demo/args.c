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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <LC_args.h>
#include <LC_vars.h>

static const char *name;

static bool flag = false;
static char message[4096] = "";
static int ints[4096];
static size_t length;

static const char *files[4096];

static void about();
static void help(int ret);
static void print_ints();
static void print_files();

static void help_flag();
static void init(int argc, char **argv);

int main(int argc, char **argv) {
	name = argv[0];
	init(argc, argv);

	putchar('\n');
	if(flag) puts("  The flag was set!\n");
	else puts("  The flag wasn't set.\n");

	if(message[0]) printf("  Message: %s\n\n", message);
	if(length) print_ints();
	if(files[0]) print_files();

	exit(0);
}

static void about() {
	putchar('\n');
	puts("  libClame: Command-line Arguments Made Easy");
	puts("  Copyright (C) 2021-2022 Jyothiraditya Nellakra\n");

	puts("  This program is free software: you can redistribute it and/or modify");
	puts("  it under the terms of the GNU General Public License as published by");
	puts("  the Free Software Foundation, either version 3 of the License, or");
	puts("  (at your option) any later version.\n");

	puts("  This program is distributed in the hope that it will be useful,");
	puts("  but WITHOUT ANY WARRANTY; without even the implied warranty of");
	puts("  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the");
	puts("  GNU General Public License for more details.\n");

	puts("  You should have received a copy of the GNU General Public License");
	puts("  along with this program. If not, see <https://www.gnu.org/licenses/>.\n");

	exit(0);
}

static void help(int ret) {
	putchar('\n');
	printf("  Usage: %s [OPTIONS] [--] [FILES]\n\n", name);

	puts("  Valid options are:");
	puts("    -a, --about             print the about dialogue");
	puts("    -h, --help              print this help dialogue\n");

	puts("    -f, --flag              set the flag");
	puts("    -m, --message MESSAGE   set the message to MESSAGE");
	puts("    -i, --ints INTS... [--] set the ints to INTS\n");

	puts("  Note: A '--' before [FILES] signifies the end of the options. Any");
	puts("        options found after it will be treated as filenames.\n");

	puts("  Note: After INTS, you will need two '--'s, as the optional '--'");
	puts("        directly after INTS only signals the end of the INTS and");
	puts("        not the end of the options.\n");

	puts("  Happy coding! :)\n");
	exit(ret);
}

static void print_ints() {
	printf("  Ints: ");

	for(size_t i = 0; i < length; i++) {
		printf("%d", ints[i]);

		if(i + 1 < length) printf(", ");
	}

	puts("\n");
}

static void print_files() {
	printf("  Files: ");

	size_t max_files;
	for(max_files = 0; max_files < LCa_max_noflags; max_files++)
		if(!files[max_files]) break;

	for(size_t i = 0; i < max_files; i++) {
		printf("%s", files[i]);

		if(i + 1 < max_files) printf(", ");
	}

	puts("\n");
}

static void help_flag() {
	help(0);
}

static void init(int argc, char **argv) {
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

	LCa_noflags = files;
	LCa_max_noflags = 4096;

	int ret = LCa_read(argc, argv);
	if(ret != LCA_OK) help(1);
}