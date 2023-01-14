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

#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <termios.h>
#include <unistd.h>

#include <LC_lines.h>

char *LCl_buffer;
size_t LCl_length;
bool LCl_sigint;

static struct termios cooked, raw;
static char *inp_buffer;
static size_t first, last;

static size_t insertion_point;
static size_t total_chars;
static bool finished;

static size_t home_i, home_j;
static size_t i, j;

int LCl_bread() {
	int c = ' ';
	size_t i = 0;
	bool clipped = false;

	*LCl_buffer = 0;
	LCl_sigint = false;

	while(c != '\n' && isspace(c)) {
		c = fgetc(stdin);

		if(LCl_sigint) {
			putchar('\n');
			return LCL_INT;
		}
	}

	while(c != '\n') {
		if(i + 1 >= LCl_length) {
			clipped = true;
			break;
		};

		LCl_buffer[i] = (char) c;
		LCl_buffer[i + 1] = 0;
		i++;

		c = fgetc(stdin);

		if(LCl_sigint) {
			putchar('\n');
			return LCL_INT;
		}
	}

	if(!clipped) return LCL_OK;

	while(c != '\n') {
		c = fgetc(stdin);

		if(LCl_sigint) {
			putchar('\n');
			return LCL_CUT_INT;
		}
	}

	return LCL_CUT;
}

int LCl_fread(FILE *file, char *buffer, size_t length) {
	int c = ' ';
	size_t i = 0;
	bool clipped = false;

	*buffer = 0;

	if(feof(file)) return LCL_EOF;

	while(c != '\n' && isspace(c)) {
		c = fgetc(file);
		if(feof(file)) return LCL_EOF;
	}

	while(c != '\n') {
		if(i + 1 >= length) {
			clipped = true;
			break;
		};

		buffer[i] = (char) c;
		buffer[i + 1] = 0;
		i++;

		c = fgetc(file);
		if(feof(file)) return LCL_EOF;
	}

	if(!clipped) return LCL_OK;

	while(c != '\n') {
		c = fgetc(file);
		if(feof(file)) return LCL_CUT_EOF;
	}

	return LCL_CUT;
}

static int cleanup();
static char getch();
static int setij();

static int refresh_noch();
static int refresh_postch();

static int printchar(char ch);
static int readchar();

static int escape_code(char ch);
static int insert_char(char ch);
static int delete_char();

int LCl_read() {
	int ret = tcgetattr(STDIN_FILENO, &cooked);
	if(ret == -1) return LCL_ERR;

	raw = cooked;
	raw.c_lflag &= ~ICANON;
	raw.c_lflag &= ~ECHO;

	raw.c_cc[VINTR] = 3;
	raw.c_lflag |= ISIG;

	ret = tcsetattr(STDIN_FILENO, TCSANOW, &raw);
	if(ret == -1) return LCL_ERR;

	if(!inp_buffer) inp_buffer = malloc(LCl_length * sizeof(char));
	if(!inp_buffer) return cleanup(LCL_ERR);

	ret = setij();
	if(ret != LCL_OK) return cleanup(ret);
	home_i = i; home_j = j;

	*LCl_buffer = 0;
	insertion_point = 0;
	total_chars = 0;

	ret = LCL_OK;
	finished = false;
	LCl_sigint = false;

	while(ret == LCL_OK && !finished) {
		if(total_chars + 1 >= LCl_length) return cleanup(LCL_ERR);
		else ret = readchar();
	}

	return cleanup(ret);
}

static int cleanup(int ret) {
	int ret2 = setij();
	if(ret2 != LCL_OK) return ret2;

	ret2 = tcsetattr(STDIN_FILENO, TCSANOW, &cooked);
	if(ret2 == -1) return LCL_ERR;

	size_t total, a;
	for(total = 0; isspace(LCl_buffer[total]); total++);

	for(a = 0; LCl_buffer[total + a]; a++) {
		LCl_buffer[a] = LCl_buffer[total + a];
	}

	LCl_buffer[a] = 0;
	if(ret == LCL_INT) puts("^C");

	return ret;
}

static char getch() {
	if(!last) return getchar();

	if(first == last) {
		first = 0; last = 0;
		return getchar();
	}

	return inp_buffer[first++];
}

static int setij() {
	printf("\033[6n");
	inp_buffer[last] = getchar();

	while(inp_buffer[last] != '\033') {
		if(last + 1 < LCl_length) last++;
		inp_buffer[last] = getchar();
	}

	int ret = scanf("[%zu;%zuR", &i, &j);
	if(ret != 2) return LCL_ERR;
	else return LCL_OK;
}

static int refresh_noch() {
	printf("\033[%zu;%zuH\033[J", home_i, home_j);

	for(size_t a = 0; a < insertion_point; a++)
		putchar(LCl_buffer[a]);

	printf("\033[s\033[?25l%s", &LCl_buffer[insertion_point]);

	int ret = setij();
	if(ret != LCL_OK) return ret;

	printf("\033[u\033[?25h");
	return LCL_OK;
}

static int refresh_postch() {
	if(!total_chars) return LCL_OK;
	char ch = LCl_buffer[total_chars - 1];
	LCl_buffer[total_chars - 1] = 0;

	printf("\033[%zu;%zuH\033[J", home_i, home_j);

	for(size_t a = 0; a < insertion_point; a++)
		putchar(LCl_buffer[a]);

	printf("\033[s\033[?25l%s", &LCl_buffer[insertion_point]);

	int ret = printchar(ch);
	if(ret != LCL_OK) return ret;

	printf("\033[u\033[?25h");
	LCl_buffer[total_chars - 1] = ch;
	return LCL_OK;
}

static int printchar(char ch) {
	size_t old_i = i;
	size_t old_j = j;
	putchar(ch);

	int ret = setij();
	if(ret != LCL_OK) return LCL_ERR;

	if(j < old_j && i == old_i) home_i--;
	return LCL_OK;
}

static int readchar() {
	char input = getch();
	if(LCl_sigint) return LCL_INT;

	switch(input) {
	case '\033':
		getch();
		return escape_code(getch());

	case '\n':
		puts(&LCl_buffer[insertion_point]);
		finished = true;
		break;

	case 0x7f:
		return delete_char();

	default:
		if(!isprint(input) && input != '\t') return setij();
		return insert_char(input);
	}

	return LCL_OK;
}

static int escape_code(char ch) {
	switch(ch) {
	case 'D':
		if(!insertion_point) return LCL_OK;

		insertion_point--;
		return refresh_noch();

	case 'C':
		if(insertion_point == total_chars) return LCL_OK;

		insertion_point++;
		return refresh_noch();

	case 'F':
		insertion_point = total_chars;
		return refresh_noch();

	case 'H':
		insertion_point = 0;
		return refresh_noch();

	case 'Z':
		if(!insertion_point) return LCL_OK;
		
		for(insertion_point--; insertion_point; insertion_point--) {
			if(isspace(LCl_buffer[insertion_point])) break;
		}
		return refresh_noch();
	}

	return setij();
}

static int insert_char(char ch) {
	for(size_t a = total_chars; a > insertion_point; a--)
		LCl_buffer[a] = LCl_buffer[a - 1];

	LCl_buffer[insertion_point++] = ch;
	LCl_buffer[++total_chars] = 0;

	if(insertion_point == total_chars) return printchar(ch);
	else return refresh_postch();
}

static int delete_char() {
	if(!insertion_point) return LCL_OK;

	for(size_t a = insertion_point; a < total_chars; a++)
		LCl_buffer[a - 1] = LCl_buffer[a];

	insertion_point--;
	LCl_buffer[--total_chars] = 0;
	return refresh_noch();
}

static signed char cleanup_ch(char ret);
static int flush_ch();

signed char LCl_readch() {
	int ret = tcgetattr(STDIN_FILENO, &cooked);
	if(ret == -1) { puts("*_* ..."); return LCLCH_ERR; }

	raw = cooked;
	raw.c_lflag &= ~ICANON;
	raw.c_lflag &= ~ECHO;

	raw.c_cc[VINTR] = 3;
	raw.c_lflag |= ISIG;

	ret = tcsetattr(STDIN_FILENO, TCSANOW, &raw);
	if(ret == -1) { puts("*_* ..."); return LCLCH_ERR; }

	ret = flush_ch();
	if(ret != LCL_OK) return cleanup_ch(LCLCH_ERR);

	LCl_sigint = false;
	char ch = getchar();

	if(ch == '\033') ret = flush_ch();
	if(ret != LCL_OK) return cleanup_ch(LCLCH_ERR);

	if(LCl_sigint) return cleanup_ch(LCLCH_INT);
	else return cleanup_ch(ch);
}

static signed char cleanup_ch(char ret) {
	int ret2 = tcsetattr(STDIN_FILENO, TCSANOW, &cooked);
	if(ret2 == -1) ret = LCLCH_ERR;

	switch(ret) {
	case LCLCH_ERR:	puts("*_* ..."); break;
	case LCLCH_INT:	puts("^C"); break;
	case '\n':     	puts("<-'"); break;
	case '\t':     	puts("->"); break;
	case ' ':      	puts("[___]"); break;

	default:
		if(!isprint(ret)) return LCLCH_BAD;

		putchar(ret);
		putchar('\n');
	}

	return ret;
}

static int flush_ch() {
	printf("\033[6n");
	char buffer = getchar();
	while(buffer != '\033') buffer = getchar();

	size_t i, j;
	int ret = scanf("[%zu;%zuR", &i, &j);
	if(ret != 2) return LCL_ERR;
	else return LCL_OK;
}