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
#include <string.h>

#include <signal.h>

#include <LC_args.h>
#include <LC_editor.h>
#include <LC_vars.h>

static const char *name;
static char message[4096] = "\tYou can type something or the other here. "
"The program will print it out when it exits. (Unless the program has broken "
"down horribly or something else has gone very wrong.)\n\n"

"\tLorem ipsum dolor sit amet, consectetur "
"adipiscing elit. Morbi in augue tempor, pretium ante quis, condimentum nisi. "
"Proin ornare auctor elit, a consectetur risus mattis sit amet. Mauris et "
"vulputate neque. Vestibulum ante ipsum primis in faucibus orci luctus et "
"ultrices posuere cubilia curae; Ut sed eros consectetur, ullamcorper purus "
"sed, efficitur sem. Fusce suscipit suscipit lobortis. Fusce imperdiet "
"ultricies risus a laoreet. Quisque sapien nibh, congue eu est in, "
"sollicitudin scelerisque orci. Interdum et malesuada fames ac ante ipsum "
"primis in faucibus. Sed rutrum in enim non iaculis. Cras vulputate elit "
"commodo tempor blandit. Integer convallis at turpis id scelerisque. Nullam "
"eu turpis in dolor porttitor elementum. Morbi et risus mauris. Vivamus a erat "
"ut ipsum lobortis porta vitae a felis.\n\n"

"\tAenean quis tempus libero. Praesent elit ipsum, mollis quis laoreet et, "
"malesuada at odio. Aliquam nisi turpis, mollis vel eleifend quis, efficitur "
"pulvinar quam. Sed vehicula euismod nisl quis accumsan. Donec cursus sapien "
"ex, at commodo arcu accumsan nec. Vivamus vehicula ex nec tellus tempor "
"malesuada. Cras accumsan libero mi, vitae luctus felis posuere quis.\n\n"

"\tVivamus in risus quam. Praesent molestie ut tellus in gravida. Suspendisse "
"dolor nisi, pharetra facilisis faucibus at, dictum non enim. Vestibulum "
"commodo, diam ac volutpat gravida, nisl mauris malesuada lacus, vitae "
"scelerisque urna ligula eu eros. Aliquam a purus quam. Maecenas mattis, "
"lacus et venenatis mollis, justo massa pulvinar mi, in commodo dui dolor ut "
"elit. Vestibulum porta neque quis consequat tincidunt. Cras gravida, nibh "
"nec mollis convallis, purus purus vehicula purus, et rutrum felis mi vitae "
"ex. Nam sagittis nec libero a molestie. Morbi sit amet aliquam mi.\n\n"

"\tIn elit metus, condimentum non ipsum eget, finibus scelerisque massa. "
"Pellentesque aliquet sapien eu ligula venenatis, et congue leo vehicula. "
"Morbi id tincidunt mauris. Integer nec accumsan tellus, vitae molestie "
"turpis. Pellentesque habitant morbi tristique senectus et netus et malesuada "
"fames ac turpis egestas. Maecenas ut aliquam sapien. Nulla in odio quis "
"diam sodales finibus ac sed lorem. Integer volutpat dolor sed egestas "
"bibendum. Morbi volutpat eros vehicula dolor porta, sit amet aliquet eros "
"auctor. Quisque vulputate in justo ut lacinia. Pellentesque non nibh "
"malesuada diam fringilla ullamcorper in quis nulla. Donec sodales "
"condimentum risus, quis vehicula tellus porta in. Nunc sit amet dapibus "
"justo. Ut dolor ipsum, fringilla sit amet suscipit eget, consequat et "
"sapien. Vestibulum ante ipsum primis in faucibus orci luctus et ultrices "
"posuere cubilia curae; Duis id venenatis nisi.";

static void about();
static void help(int ret);

static void help_flag();
static void init();

static void on_interrupt(int signum);

int main(int argc, char **argv) {
	name = argv[0];
	init();

	int ret = LCa_read(argc, argv);
	if(ret != LCA_OK) help(1);

	LCe_banner = "libClame: Command-line Arguments Made Easy";
	LCe_buffer = message;
	LCe_length = 4096;
	LCe_dirty = false;

	ret = LCe_edit();
	switch(ret) {
	case LCE_OK:
		if(LCe_dirty) printf("\e[H\e[J%s\n", message);
		else printf("\e[H\e[J");
		exit(0);

	case LCE_ERR:
		fprintf(stderr, "%s: error: unknown error\n", name);
		exit(1);
	}
}

static void about() {
	putchar('\n');
	puts("  libClame: Command-line Arguments Made Easy");
	puts("  Copyright (C) 2021 Jyothiraditya Nellakra\n");

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
}

static void on_interrupt(int signum) {
	if(signum != SIGINT) {
		signal(signum, SIG_DFL);
		return;
	}

	LCe_sigint = true;
}