/* libClame: Command-line Arguments Made Easy
 * Copyright (c) 2021-2022 Jyothiraditya Nellakra
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR a PARTICULAR PURPOSE. See the GNU General Public License for
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

#include <LC_editor.h>

const char *LCe_banner;
char *LCe_buffer;
size_t LCe_length;

bool LCe_sigint;
bool LCe_dirty;

static char *inp_buffer;
static size_t first, last;

static struct termios cooked, raw;
static size_t height, width;
static size_t x, y, scroll;

static char **lines;
static size_t *start_points;
static size_t post_screen;

static size_t total_chars;
static size_t insertion_point;

static bool do_redraw_all;
static bool do_redraw_line;
static bool do_redraw_status;
static bool do_redraw_to_end;

static int cleanup();
static int flush();
static char getch();
static int gethw();

static void refresh_all();
static void refresh_on_tab(size_t *a, size_t *b);

static void redraw();
static void redraw_all();
static void redraw_line();
static void redraw_status();
static void redraw_to_end();

static void cursor_up();
static void cursor_down();
static void cursor_left();
static void cursor_right();
static void cursor_home();
static void cursor_end();

static void get_input();
static void escape_code(char ch);
static void insert(char ch);
static void delete();

static size_t numch(size_t num);

int LCe_edit() {
	int ret = tcgetattr(STDIN_FILENO, &cooked);
	if(ret == -1) return LCE_ERR;

	raw = cooked;
	raw.c_lflag &= ~ICANON;
	raw.c_lflag &= ~ECHO;

	raw.c_cc[VINTR] = 3;
	raw.c_lflag |= ISIG;

	ret = tcsetattr(STDIN_FILENO, TCSANOW, &raw);
	if(ret == -1) return LCE_ERR;

	inp_buffer = malloc(LCe_length * sizeof(char));
	if(!inp_buffer) return cleanup(LCE_ERR);
	first = 0; last = 0;

	ret = gethw();
	if(ret != LCE_OK) return cleanup(ret);

	lines = calloc(height, sizeof(char *));
	if(!lines) return cleanup(LCE_ERR);

	for(size_t i = 0; i < height; i++) {
		lines[i] = calloc(width + 2, sizeof(char));
		if(!lines[i]) return cleanup(LCE_ERR);
	}

	start_points = malloc(height * sizeof(char *));
	if(!start_points) return cleanup(LCE_ERR);

	x = 0; y = 0; scroll = 0;
	LCe_sigint = false;
	total_chars = strlen(LCe_buffer);
	if(insertion_point > total_chars) insertion_point = 0;

	do_redraw_all = false;
	do_redraw_line = false;
	do_redraw_status = false;
	do_redraw_to_end = false;

	refresh_all();
	redraw_all();
	while(!LCe_sigint) {
		get_input();
	}

	return cleanup(LCE_OK);
}

static int cleanup(int ret) {
	if(inp_buffer) free(inp_buffer);
	if(start_points) free(start_points);

	for(size_t i = 0; i < height; i++) {
		if(!lines) break;
		if(lines[i]) free(lines[i]);
		else break;
	}

	int ret2 = tcsetattr(STDIN_FILENO, TCSANOW, &cooked);
	if(ret2 == -1) return LCE_ERR;
	else return ret;
}

static int flush() {
	printf("\033[6n");
	inp_buffer[last] = getchar();

	while(inp_buffer[last] != '\033') {
		if(last + 1 < LCe_length) last++;
		inp_buffer[last] = getchar();
	}

	size_t i, j;
	int ret = scanf("[%zu;%zuR", &i, &j);
	if(ret != 2) return LCE_ERR;
	else return LCE_OK;
}

static char getch() {
	if(!last) return getchar();

	if(first == last) {
		first = 0; last = 0;
		return getchar();
	}

	return inp_buffer[first++];
}

static int gethw() {
	printf("\033[s\033[999;999H\033[6n\033[u");
	inp_buffer[last] = getchar();

	while(inp_buffer[last] != '\033') {
		if(last + 1 < LCe_length) last++;
		inp_buffer[last] = getchar();
	}

	int ret = scanf("[%zu;%zuR", &height, &width);
	if(ret != 2) return LCE_ERR;

	height -= 2;
	return LCE_OK;
}

static void refresh_all() {
	size_t i = 0, j = 0, k;
	for(k = 0; k < total_chars; k++) {
		if(j >= width) { i++; j = 0; }
		if(i >= scroll) break;

		switch(LCe_buffer[k]) {
		case '\t':
			j += 8 - (j % 8);
			continue;

		case '\n':
			i++; j = 0;
			continue;

		default:
			j++;
		}
	}

	for(i = 0; i < height; i++) lines[i][width + 1] = 0;
	i = 0; j = 0;

	for(k = k; k < total_chars; k++) {
		if(j >= width) { lines[i][width] = 0; i++; j = 0; }
		if(i >= height) break;

		if(insertion_point == k) { y = i; x = j; }
		if(j == 0) start_points[i] = k;
		lines[i][j] = LCe_buffer[k];

		switch(LCe_buffer[k]) {
		case '\t':
			refresh_on_tab(&i, &j);
			continue;

		case '\n':
			lines[i][width + 1] = '\n';
			lines[i][j] = 0;
			i++; j = 0;
			continue;

		default:
			j++;
		}
	}

	
	post_screen = k;
	if(insertion_point == total_chars) { y = i; x = j; }
	if(x >= width) { y++; x = 0; }

	while(i < height) {
		while(j < width + 2) lines[i][++j] = 0;
		i++; j = 0; 
	}
}

static void refresh_on_tab(size_t *a, size_t *b) {
	size_t i = *a, j = *b;

	for(size_t tabs = 8 - (j % 8); tabs > 0; tabs--) {
		if(j >= width) { lines[i][width] = 0; i++; j = 0; break; }
		lines[i][j] = ' '; j++;
	}

	*a = i; *b = j;
}

static void redraw() {
	static bool deferred = false;

	if(last != first) {
		deferred = true;
		return;
	}

	if(deferred) {
		deferred = false;
		do_redraw_all = false;
		do_redraw_line = false;
		do_redraw_status = false;
		do_redraw_to_end = false;
		redraw_all();
		return;
	}

	if(do_redraw_all) {
		do_redraw_all = false;
		do_redraw_line = false;
		do_redraw_status = false;
		do_redraw_to_end = false;
		redraw_all();
		return;
	}

	if(do_redraw_to_end) {
		do_redraw_to_end = false;
		redraw_to_end();
	}

	if(do_redraw_line) {
		do_redraw_line = false;
		redraw_line();
	}

	if(do_redraw_status) {
		do_redraw_status = false;
		redraw_status();
	}
}

static void redraw_all() {
	size_t chs_free = LCe_length - total_chars - 1;
	size_t padding = width - 2;
	padding -= strlen(LCe_banner);
	padding -= strlen(" Chars Free");
	padding -= numch(chs_free);

	if(padding > width) {
		padding = width - 2;
		padding -= strlen(" Chars Free");
		padding -= numch(chs_free);

		printf("\033[?25l\033[H\033[J\033[7m ");
		for(size_t i = 0; i < padding; i++) putchar(' ');
		printf("%zu Chars Free \n\033[0m", chs_free);
	}

	else {
		printf("\033[?25l\033[H\033[J\033[7m %s", LCe_banner);
		for(size_t i = 0; i < padding; i++) putchar(' ');
		printf("%zu Chars Free \n\033[0m", chs_free);
	}

	for(size_t i = 0; i < height; i++) printf("\033[K%s\n", lines[i]);

	size_t y_eff = 1, x_eff = 1;
	for(size_t i = 0; i < start_points[0]; i++) {
		if(LCe_buffer[i] == '\n') y_eff++;
	}

	for(size_t i = 0; i < y; i++) {
		if(lines[i][width + 1] == '\n') { y_eff++; x_eff = 0; }
		else x_eff += width;
	}

	x_eff += x;

	padding = width - 3;
	padding -= strlen("Hit ^C to exit.");
	padding -= strlen("Line ") + strlen("Col ");
	padding -= numch(x_eff) + numch(y_eff);

	printf("\033[7m Hit ^C to exit.");
	for(size_t i = 0; i < padding; i++) putchar(' ');
	printf("Line %zu Col %zu \033[0m\033[%zu;%zuH\033[?25h",
		y_eff, x_eff, y + 2, x + 1);
}

static void redraw_line() {
	printf("\033[?25l\033[%zu;1H", y + 2);

	for(size_t i = y; i < height; i++) {
		printf("\033[K%s\n", lines[i]);
		if(lines[i][width + 1] == '\n') break;
	}

	printf("\033[%zu;%zuH\033[?25h", y + 2, x + 1);
}

static void redraw_status() {
	size_t chs_free = LCe_length - total_chars - 1;

	size_t padding = width - 2;
	padding -= strlen(LCe_banner);
	padding -= strlen(" Chars Free");
	padding -= numch(chs_free);

	if(padding > width) {
		padding = width - 2;
		padding -= strlen(" Chars Free");
		padding -= numch(chs_free);

		printf("\033[s\033[?25l\033[H\033[7m ");
		for(size_t i = 0; i < padding; i++) putchar(' ');
		printf("%zu Chars Free ", chs_free);		
	}

	else {
		printf("\033[s\033[?25l\033[H\033[7m %s", LCe_banner);
		for(size_t i = 0; i < padding; i++) putchar(' ');
		printf("%zu Chars Free ", chs_free);
	}

	size_t y_eff = 1, x_eff = 1;
	for(size_t i = 0; i < start_points[0]; i++) {
		if(LCe_buffer[i] == '\n') y_eff++;
	}

	for(size_t i = 0; i < y; i++) {
		if(lines[i][width + 1] == '\n') { y_eff++; x_eff = 0; }
		else x_eff += width;
	}

	x_eff += x;

	padding = width - 3;
	padding -= strlen("Hit ^C to exit.");
	padding -= strlen("Line ") + strlen("Col ");
	padding -= numch(x_eff) + numch(y_eff);

	printf("\033[%zu;0H\033[7m Hit ^C to exit.", height + 2);
	for(size_t i = 0; i < padding; i++) putchar(' ');
	printf("Line %zu Col %zu \033[0m\033[u\033[?25h", y_eff, x_eff);
}

static void redraw_to_end() {
	printf("\033[?25l\033[%zu;1H", y + 2);
	for(size_t i = y; i < height; i++) printf("\033[K%s\n", lines[i]);
	printf("\033[%zu;%zuH\033[?25h", y + 2, x + 1);
}

static void cursor_up() {
	if(!y && !scroll) { cursor_home(); return; }

	if(!y) {
		scroll--;
		refresh_all();
		redraw_all();
	}

	size_t i, j = 0;
	for(i = start_points[y - 1]; i < start_points[y]; i++) {
		if(j >= x) break;

		switch(LCe_buffer[i]) {
		case '\t':
			j += 8 - (j % 8);
			break;

		default:
			j++;
		}
	}

	insertion_point = i < start_points[y] ? i : i - 1;
	refresh_all(); redraw_status();
	printf("\033[%zu;%zuH", y + 2, x + 1);
}

static void cursor_down() {
	if(y >= height - 1 && post_screen >= total_chars) {
		cursor_end();
		return;
	}

	if(y >= height - 1) {
		scroll++;
		refresh_all();
		redraw_all();
	}

	if(!start_points[y + 1]) return;

	size_t limit = y + 2 < height ? start_points[y + 2] : post_screen;
	if(!limit) limit = post_screen;

	size_t i, j = 0;
	for(i = start_points[y + 1]; i < limit; i++) {
		if(j >= x) break;

		switch(LCe_buffer[i]) {
		case '\t':
			j += 8 - (j % 8);
			break;

		default:
			j++;
		}
	}

	insertion_point = i < limit ? i : i - 1;
	refresh_all(); redraw_status();
	printf("\033[%zu;%zuH", y + 2, x + 1);
}

static void cursor_left() {
	if(!insertion_point) return;
	insertion_point--;

	if(insertion_point < start_points[0]) {
		scroll--;
		refresh_all();
		redraw_all();
	}

	else {
		refresh_all();
		redraw_status();
		redraw_to_end();
	}
}

static void cursor_right() {
	if(insertion_point >= total_chars) return;
	insertion_point++;

	if(insertion_point > post_screen) {
		scroll++;
		refresh_all();
		redraw_all();
	}

	else {
		refresh_all();
		redraw_status();
		redraw_to_end();
	}
}

static void cursor_home() {
	insertion_point = start_points[y];

	refresh_all();
	redraw_status();
	redraw_to_end();
}

static void cursor_end() {
	insertion_point = 0;

	if(y + 1 < height) if(start_points[y + 1])
		insertion_point = start_points[y + 1] - 1;

	if(!insertion_point) insertion_point = post_screen;

	refresh_all();
	redraw_status();
	redraw_to_end();
}


static void get_input() {
	char ch = getch();
	if(LCe_sigint) return;

	switch(ch) {
	case '\033':
		getch();
		escape_code(getch());
		break;

	case 0x7f:
		delete();
		break;

	default:
		if(!isprint(ch) && ch != '\t' && ch != '\n') return;
		insert(ch);
	}

	flush();
}

static void escape_code(char ch) {
	switch(ch) {
		case 'A': cursor_up(); break;
		case 'B': cursor_down(); break;
		case 'D': cursor_left(); break;
		case 'C': cursor_right(); break;
		case 'H': cursor_home(); break;
		case 'F': cursor_end(); break;
	}
}

static void insert(char ch) {
	if(total_chars + 1 >= LCe_length) return;
	else LCe_dirty = true;

	for(size_t i = total_chars; i > insertion_point; i--)
		LCe_buffer[i] = LCe_buffer[i - 1];

	LCe_buffer[insertion_point++] = ch;
	LCe_buffer[++total_chars] = 0;

	refresh_all();

	if(y >= height) {
		scroll++;
		refresh_all();
		do_redraw_all = true;
	}

	else {
		if(ch == '\n') printf("\033[K\n");
		do_redraw_to_end = true;
		do_redraw_status = true;
	}

	redraw();
}

static void delete() {
	if(!insertion_point) return;
	else LCe_dirty = true;

	for(size_t i = insertion_point; i < total_chars; i++)
		LCe_buffer[i - 1] = LCe_buffer[i];

	insertion_point--;
	LCe_buffer[--total_chars] = 0;

	if(insertion_point < start_points[0]) {
		scroll--;
		refresh_all();
		do_redraw_all = true;
	}
	
	else if(insertion_point > start_points[y])
	{
		refresh_all();
		do_redraw_status = true;
		do_redraw_line = true;
	}

	else {
		refresh_all();
		do_redraw_status = true;
		do_redraw_to_end = true;
	}

	redraw();
}

static size_t numch(size_t num) {
	size_t digits;
	for(digits = 1; num >= 10; digits++) num /= 10;
	return digits;
}