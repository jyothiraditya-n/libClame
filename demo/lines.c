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
#include <LC_lines.h>
#include <LC_vars.h>

static const char *name;
static char message[4096] = "";
static bool no_ansi;

static void about();
static void help(int ret);

static void help_flag();
static void init();

static void read_message();
static void on_interrupt(int signum);

int main(int argc, char **argv) {
	name = argv[0];
	init();

	int ret = LCa_read(argc, argv);
	if(ret != LCA_OK) help(1);

	LCl_buffer = message;
	LCl_length = 4096;

	while(true) read_message();
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

	puts("    -n, --no-ansi           disables the use of ANSI escape codes.\n");

	puts("  Happy coding! :)\n");
	exit(ret);
}

static void help_flag() {
	help(0);
}

static void init() {
	signal(SIGINT, on_interrupt);

	LCa_t *arg = LCa_new();
	arg -> long_flag = "about";
	arg -> short_flag = 'a';
	arg -> pre = about;

	arg = LCa_new();
	arg -> long_flag = "help";
	arg -> short_flag = 'h';
	arg -> pre = help_flag;

	LCv_t *var = LCv_new();
	var -> id = "no-ansi";
	var -> data = &no_ansi;

	arg = LCa_new();
	arg -> long_flag = "no-ansi";
	arg -> short_flag = 'n';
	arg -> var = var;
	arg -> value = true;
}

static void read_message() {
	int ret;

	printf("Type a message!> ");
	if(no_ansi) ret = LCl_bread();
	else ret = LCl_read();

	switch(ret) {
	case LCL_OK:
		if(strlen(message)) printf("  You typed: %s\n", message);
		return;

	case LCL_CUT:
		fprintf(stderr, "%s: error: input too long\n", name);
		return;

	case LCL_INT:
		if(no_ansi) exit(0);
		else printf("  Exit? [Y/n]: ");

		char ret = LCl_readch();

		if(ret == LCLCH_ERR) exit(1);
		else if(ret == 'Y' || ret == 'y' || ret == LCLCH_INT) exit(0);
		else break;

	default:
		fprintf(stderr, "%s: error: unknown error\n", name);
		exit(1);
	}
}

static void on_interrupt(int signum) {
	if(signum != SIGINT) {
		signal(signum, SIG_DFL);
		return;
	}

	signal(SIGINT, on_interrupt);
	LCl_sigint = true;
}