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

#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include <termios.h>
#include <unistd.h>

#include <LC_lines.h>

bool LCl_sigint;

static struct termios cooked, raw;

static char *data;
static size_t length;
static size_t i;

static int home_i, home_j;
static int current_i, current_j;

static int next();
static int _next();

#define ERR_CHR -1

static int reset();
static int setij();

static int next() {
	int input = _next();
	while(!input) input = _next();
	return input;
}

static int _next() {
	int old_i = current_i;
	int old_j = current_j;

	unsigned char input = getchar();
	int ret;

	switch(input) {
	case 0x7f:
		if(!i) return 0;
		data[--i] = 0;

		printf("\e[%d;%dH\e[J%s", home_i, home_j, data);

		ret = setij();
		if(ret != LCL_OK) return ERR_CHR;
		else return 0;

	default:
		if(!isprint(input) && input != '\t' && input != '\n')
			return setij();

		putchar(input);

		ret = setij();
		if(ret != LCL_OK) return ERR_CHR;

		if(current_j < old_j && current_i == old_i) home_i--;
	}

	return input;
}

static int reset(int ret) {
	int ret2 = tcsetattr(STDIN_FILENO, TCSANOW, &cooked);
	if(ret2 == -1) return LCL_ERR;

	if(ret == LCL_INT || ret == LCL_CUT_INT) puts("^C");
	return ret;
}

static int setij() {
	printf("\e[6n");
	char buffer = getchar();
	while(buffer != '\e') buffer = getchar();

	int ret = scanf("[%d;%dR", &current_i, &current_j);
	if(ret != 2) return LCL_ERR;
	else return LCL_OK;
}

int LCl_read(LCl_t *line) {
	data = line -> data;
	length = line -> length;

	int ret = tcgetattr(STDIN_FILENO, &cooked);
	if(ret == -1) return LCL_ERR;

	raw = cooked;
	raw.c_lflag &= ~ICANON;
	raw.c_lflag &= ~ECHO;
	raw.c_cc[VINTR] = 3;
	raw.c_lflag |= ISIG;

	ret = tcsetattr(STDIN_FILENO, TCSANOW, &raw);
	if(ret == -1) return LCL_ERR;

	ret = setij();
	if(ret != LCL_OK) return ret;

	home_i = current_i;
	home_j = current_j;

	LCl_sigint = false;
	*data = 0;
	i = 0;

	if(LCl_sigint) return reset(LCL_INT);

	int c = ' ';
	bool clipped = false;

	while(true) {
		ret = next();
		if(ret == ERR_CHR) return reset(LCL_ERR);

		c = ret;
		if(c == '\n') break;
		if(LCl_sigint) return reset(LCL_INT);

		if(i + 1 >= length) {
			clipped = true;
			break;
		};

		data[i] = (char) c;
		data[i + 1] = 0;
		i++;
	}

	size_t total, j;
	for(total = 0; isspace(data[total]); total++);

	for(j = 0; data[total + j]; j++) {
		data[j] = data[total + j];
	}

	data[j] = 0;

	if(!clipped) return reset(LCL_OK);

	while(c != '\n') {
		c = next();
		if(LCl_sigint) return reset(LCL_CUT_INT);
	}

	return reset(LCL_CUT);
}