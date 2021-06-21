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

static size_t insertion_point;
static size_t total_chars;
static bool finished;

static int home_i, home_j;
static int i, j;

static int backspace();
static int cleanup();
static int refresh();
static int setij();

static int cursor(char ch);
static int putch(char ch);
static int readch();

static int push(char ch);
static int pull();

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
	if(ret != LCL_OK) return cleanup(ret);

	home_i = i;
	home_j = j;

	*data = 0;
	insertion_point = 0;
	total_chars = 0;

	ret = LCL_OK;
	finished = false;
	LCl_sigint = false;

	while(ret == LCL_OK && !finished) {
		if(total_chars >= length - 1) return cleanup(LCL_ERR);
		else ret = readch();
	}

	return cleanup(ret);
}

static int backspace() {
	printf("\e[%d;%dH\e[J", home_i, home_j);

	for(size_t a = 0; a < insertion_point; a++)
		putchar(data[a]);

	printf("\e[s\e[?25l%s", &data[insertion_point]);

	int ret = setij();
	if(ret != LCL_OK) return ret;

	printf("\e[u\e[?25h");
	return LCL_OK;
}

static int cleanup(int ret) {
	int ret2 = tcsetattr(STDIN_FILENO, TCSANOW, &cooked);
	if(ret2 == -1) return LCL_ERR;

	size_t total, a;
	for(total = 0; isspace(data[total]); total++);

	for(a = 0; data[total + a]; a++) {
		data[a] = data[total + a];
	}

	data[a] = 0;

	if(ret == LCL_INT) puts("^C");
	return ret;
}

static int refresh() {
	if(!total_chars) return LCL_OK;
	char ch = data[total_chars - 1];
	data[total_chars - 1] = 0;

	printf("\e[%d;%dH\e[J", home_i, home_j);

	for(size_t a = 0; a < insertion_point; a++)
		putchar(data[a]);

	printf("\e[s\e[?25l%s", &data[insertion_point]);

	int ret = putch(ch);
	if(ret != LCL_OK) return ret;

	printf("\e[u\e[?25h");
	data[total_chars - 1] = ch;

	return LCL_OK;
}

static int setij() {
	printf("\e[6n");
	char buffer = getchar();
	while(buffer != '\e') buffer = getchar();

	int ret = scanf("[%d;%dR", &i, &j);
	if(ret != 2) return LCL_ERR;
	else return LCL_OK;
}

static int cursor(char ch) {
	switch(ch) {
	case 'D':
		if(!insertion_point) return LCL_OK;

		insertion_point--;
		return backspace();

	case 'C':
		if(insertion_point == total_chars) return LCL_OK;

		insertion_point++;
		return backspace();
	}

	return setij();
}

static int putch(char ch) {
	int old_i = i;
	int old_j = j;

	putchar(ch);

	int ret = setij();
	if(ret != LCL_OK) return LCL_ERR;

	if(j < old_j && i == old_i) home_i--;
	return LCL_OK;
}

static int readch() {
	char input = getchar();
	if(LCl_sigint) return LCL_INT;

	switch(input) {
	case '\e':
		getchar();
		return cursor(getchar());

	case '\n':
		putchar('\n');
		
		finished = true;
		break;

	case 0x7f:
		return pull();

	default:
		if(!isprint(input) && input != '\t') return setij();
		return push(input);
	}

	return LCL_OK;
}

static int push(char ch) {
	for(size_t a = total_chars; a > insertion_point; a--)
		data[a] = data[a - 1];

	data[insertion_point] = ch;

	insertion_point++;
	total_chars++;

	data[total_chars] = 0;

	if(insertion_point == total_chars) return putch(ch);
	else return refresh();
}

static int pull() {
	if(!insertion_point) return LCL_OK;

	for(size_t a = insertion_point; a < total_chars; a++)
		data[a - 1] = data[a];

	insertion_point--;
	total_chars--;

	data[total_chars] = 0;
	return backspace();
}