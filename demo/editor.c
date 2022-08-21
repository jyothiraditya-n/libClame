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

#include <signal.h>

#include <LC_args.h>
#include <LC_editor.h>
#include <LC_lines.h>
#include <LC_vars.h>

const char *name;

const char *input = NULL;
char output[4096];

char buffer[4096] = "\tYou can type something or the other here. The program "
"will ask you to save it to a file when you're done.";

void about();
void help(int ret);

void help_flag();
void init(int argc, char **argv);

void on_interrupt(int signum);

int main(int argc, char **argv) {
	name = argv[0];
	init(argc, argv);

	signal(SIGINT, on_interrupt);
	if(!input) goto next;
	
	FILE *file = fopen(input, "rb");
	if(!file) {
		fprintf(stderr, "%s: error: can't read file `%s'.\n", name, input);
		exit(2);
	}

	size_t ret2 = fread(buffer, 1, 4096, file);
	if(ret2 == 4096 && !feof(file)) {
		fprintf(stderr, "%s: error: buffer smaller than file `%s'.\n", name, input);
		exit(3);
	}

	memset(buffer + ret2, 0, 4096 - ret2);

	int ret = fclose(file);
	if(ret) {
		fprintf(stderr, "%s: error: can't close file `%s'.\n", name, input);
		exit(4);
	}

next:	LCe_banner = "libClame: Command-line Arguments Made Easy";
	LCe_buffer = buffer;
	LCe_length = 4096;
	LCe_dirty = false;

	ret = LCe_edit();
	if(ret == LCL_ERR) {
		fprintf(stderr, "%s: error: unknown error\n", name);
		exit(1);
	}

	printf("\033[H\033[JSave changes? [Y/n]: ");

	char ret3 = LCl_readch();

	if(ret3 == LCLCH_ERR) exit(1);
	else if((ret3 != 'Y' && ret3 != 'y') || ret3 == LCLCH_INT) exit(0);

	LCl_buffer = output;
	LCl_length = 4096;

	if(input) { strncpy(output, input, 4095); goto end; }
	printf("Filename: ");
	ret = LCl_read();

	switch(ret) {
	case LCL_OK:
		break;

	case LCL_CUT:
		fprintf(stderr, "%s: error: filename too long\n", name);
		exit(1);

	case LCL_INT:
		printf("Cancelled.\n");
		exit(0);

	default:
		fprintf(stderr, "%s: error: unknown error\n", name);
		exit(1);
	}

end:	file = fopen(output, "w");
	if(!file) {
		fprintf(stderr, "%s: error: can't write file `%s'.\n", name, output);
		exit(2);
	}

	ret = fprintf(file, "%s", buffer);
	if(ret < 0) {
		fprintf(stderr, "%s: error: can't write file `%s'.\n", name, output);
		exit(2);
	}

	ret = fclose(file);
	if(ret) {
		fprintf(stderr, "%s: error: can't close file `%s'.\n", name, output);
		exit(4);
	}

	return 0;
}

void about() {
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

void help(int ret) {
	putchar('\n');
	printf("  Usage: %s [OPTIONS] [--] [FILE]\n\n", name);

	puts("  Valid options are:");
	puts("    -a, --about             print the about dialogue");
	puts("    -h, --help              print this help dialogue\n");

	puts("  Happy coding! :)\n");
	exit(ret);
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

	LCa_noflags = &input;
	LCa_max_noflags = 1;

	int ret = LCa_read(argc, argv);
	if(ret != LCA_OK) help(1);
}

void on_interrupt(int signum) {
	if(signum != SIGINT) {
		signal(signum, SIG_DFL);
		return;
	}

	signal(SIGINT, on_interrupt);
	LCe_sigint = true;
	LCl_sigint = true;
}