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
#include <stdio.h>

#include <termios.h>
#include <unistd.h>

#include <LC_lines.h>

static struct termios cooked, raw;

static signed char cleanup(char ret);
static int flush();

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

	ret = flush();
	if(ret != LCL_OK) return cleanup(LCLCH_ERR);

	LCl_sigint = false;
	char ch = getchar();

	if(ch == '\e') ret = flush();
	if(ret != LCL_OK) return cleanup(LCLCH_ERR);

	if(LCl_sigint) return cleanup(LCLCH_INT);
	else return cleanup(ch);
}

static signed char cleanup(char ret) {
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

static int flush() {
	printf("\e[6n");
	char buffer = getchar();
	while(buffer != '\e') buffer = getchar();

	size_t i, j;
	int ret = scanf("[%zu;%zuR", &i, &j);
	if(ret != 2) return LCL_ERR;
	else return LCL_OK;
}