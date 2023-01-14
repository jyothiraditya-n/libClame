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
#include <LC_files.h>
#include <LC_vars.h>

const char *name;

int ints[4096];
size_t length;
bool flag;

const char *input;
char output[4096];

void about();
void help(int ret);
void print_ints();
void print_files();

void help_flag();
void init(int argc, char **argv);

int main(int argc, char **argv) {
	name = argv[0];
	init(argc, argv);
	LCv_clear();

	LCv_t *var = LCv_new();
	var -> id = "flag";
	var -> data = &flag;
	var -> size = sizeof(bool);

	var = LCv_new();
	var -> id = "ints";
	var -> data = ints;
	var -> len = &length;
	var -> min_len = 0;
	var -> max_len = 4096;
	var -> size = sizeof(int);

	LCf_program_name = "LCf_demo";
	LCf_program_ver = 1;
	LCf_program_subver = 0;

	if(input) {
		int ret = LCf_read(input);
		if(ret != LCF_OK) {
			fprintf(stderr, "%s: error: error reading file `%s'.\n",
				name, input);
			exit(1);
		}
	}

	putchar('\n');
	if(flag) puts("  The flag was set!\n");
	else puts("  The flag wasn't set.\n");

	if(length) print_ints();

	if(strlen(output)) {
		int ret = LCf_save(output);
		if(ret != LCF_OK) {
			fprintf(stderr, "%s: error: error saving file `%s'.\n",
				name, output);
			exit(1);
		}
	}

	exit(0);
}

void about() {
	putchar('\n');
	puts("  libClame: Command-line Arguments Made Easy");
	puts("  Copyright (C) 2021-2022 Jyothiraditya Nellakra");
	puts("  Demonstration Program for <LC_files.h>\n");

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

void help(int ret) {
	putchar('\n');
	printf("  Usage: %s [OPTIONS] [--] [FILE]\n\n", name);

	puts("  Valid options are:");
	puts("    -a, --about             print the about dialogue");
	puts("    -h, --help              print this help dialogue\n");

	puts("    -f, --flag              set the flag");
	puts("    -o, --output FILE       set the output file");
	puts("    -i, --ints INTS... [--] set the ints\n");

	puts("  Note: The file specified without a flag is the input file and -o specifies");
	puts("        the output file.\n");

	puts("  Happy coding! :)\n");
	exit(ret);
}

void print_ints() {
	printf("  Ints: ");

	for(size_t i = 0; i < length; i++) {
		printf("%d", ints[i]);

		if(i + 1 < length) printf(", ");
	}

	puts("\n");
}

void help_flag() {
	help(0);
}

void init(int argc, char **argv) {
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
	var -> id = "output";
	var -> fmt = "%4095[^\t\n]";
	var -> data = output;

	arg = LCa_new();
	arg -> long_flag = "output";
	arg -> short_flag = 'o';
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

	LCa_noflags = &input;
	LCa_max_noflags = 1;

	int ret = LCa_read(argc, argv);
	if(ret != LCA_OK) help(1);
}