





static size_t scroll;
static size_t *x, *y;

static int cleanup();
static int flush();
static void refresh();
static int getdim();

static void redraw_all();
static void redraw_line();
static void redraw_status();
static void redraw_to_end();

static void read_input();

static void escape_code(char ch);

static void cursor_up();
static void cursor_down();
static void cursor_home();
static void cursor_end();

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

	ret = getdim();
	if(ret != LCE_OK) return cleanup(ret);

	total_chars = strlen(LCe_buffer);
	insertion_point = 0;
	scroll = 0;

	x = malloc(width * height * sizeof(size_t));
	y = malloc(width * height * sizeof(size_t));
	if(!x || !y) cleanup(LCE_ERR);

	refresh();
	redraw_all();
	LCe_sigint = false;

	while(!LCe_sigint) read_input();
	return cleanup(LCE_OK);
}

static int cleanup(int ret) {
	if(x) free(x);
	if(y) free(y);

	int ret2 = tcsetattr(STDIN_FILENO, TCSANOW, &cooked);
	if(ret2 == -1) return LCE_ERR;

	printf("\e[H\e[J");
	return ret;
}

static int flush() {
	printf("\e[6n");
	char buffer = getchar();
	while(buffer != '\e') buffer = getchar();

	size_t i, j;
	int ret = scanf("[%zu;%zuR", &i, &j);
	if(ret != 2) return LCE_ERR;
	else return LCE_OK;
}

static void refresh() {
	size_t i = 0;
	size_t j = 0;

	for(size_t k = 0; k < total_chars; k++) {
		if(j >= width) { i++; j = 0; }
		x[k] = j; y[k] = i;

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
}

static int getdim() {
	printf("\e[999;999H\e[6n");
	char buffer = getchar();
	while(buffer != '\e') buffer = getchar();

	int ret = scanf("[%zu;%zuR", &height, &width);
	if(ret != 2) return LCE_ERR;

	height -= 2;
	return LCE_OK;
}

static void redraw_all() {
	size_t chs_free = LCe_length - total_chars - 1;

	size_t padding = width - 2;
	padding -= strlen(LCe_banner);
	padding -= strlen(" Chars Free");
	padding -= numch(chs_free);

	printf("\e[?25l\e[H\e[J\e[7m %s", LCe_banner);
	for(size_t i = 0; i < padding; i++) putchar(' ');
	printf("%zu Chars Free \n\e[0m", chs_free);
	
	size_t i;
	for(i = 0; y[i] < scroll; i++);

	for(; i < total_chars; i++) {
		if(y[i] >= height + scroll) break;
		putchar(LCe_buffer[i]);
	}

	padding = width - strlen(" Hit ^k to exit.");
	size_t y_pos = y[insertion_point] - scroll + 2;
	size_t x_pos = x[insertion_point] + 1;

	printf("\e[%zu;%zuH\e[7m Hit ^k to exit.", height + 2, width - width);
	for(i = 0; i < padding; i++) putchar(' ');
	printf("\e[0m\e[%zu;%zuH\e[?25h", y_pos, x_pos);
}

static void redraw_line() {
	printf("\e[s\e[?25l\e[1G\e[K");

	for(size_t i = insertion_point; y[i] == y[insertion_point]; i++)
		putchar(LCe_buffer[i]);

	printf("\e[u\e[?25h");
}

static void redraw_status() {
	size_t chs_free = LCe_length - total_chars - 1;

	size_t padding = width - 2;
	padding -= strlen(LCe_banner);
	padding -= strlen(" Chars Free");
	padding -= numch(chs_free);

	printf("\e[s\e[?25l\e[H\e[7m %s", LCe_banner);
	for(size_t i = 0; i < padding; i++) putchar(' ');
	printf("%zu Chars Free \n\e[0m\e[u\e[?25h", chs_free);
}

static void redraw_to_end() {
	printf("\e[s\e[?25l\e[1G\e[0J");
	size_t j = y[insertion_point];

	for(size_t i = insertion_point; i < total_chars; i++) {
		if(y[i] >= height + scroll) break;
		putchar(LCe_buffer[i]);
		j = y[i] ? y[i] : j;
	}

	printf("\e[u\e[?25h");
}

static void read_input() {
	char input = getchar();
	if(LCe_sigint) return;

	switch(input) {
	case '\e':
		getchar();
		escape_code(getchar());
		break;

	case 0x7f:
		delete();
		break;

	default:
		if(!isprint(input) && input != '\t' && input != '\n') {
			flush();
			return;
		}

		insert(input);
	}
}

static void escape_code(char ch) {
	flush();

	switch(ch) {
	case 'i':
		cursor_up();
		break;

	case 'j':
		cursor_down();
		break;

	case 'D':
		if(!insertion_point) return;

		insertion_point--;
		break;

	case 'k':
		if(insertion_point == total_chars) return;

		insertion_point++;
		break;

	case 'H':
		cursor_home();
		break;

	case 'F':
		cursor_end();
		break;

	case 'Z':
		if(!insertion_point) return;
		
		for(insertion_point--; insertion_point; insertion_point--) {
			if(isspace(LCe_buffer[insertion_point])) break;
		}

		break;
	}

	while(y[insertion_point] < scroll) scroll--;
	while(y[insertion_point] >= height + scroll) scroll++;

	size_t y_pos = y[insertion_point] - scroll + 2;
	size_t x_pos = x[insertion_point] + 1;

	printf("\e[%zu;%zuH", y_pos, x_pos);
}

static void insert(char ch) {
	if(total_chars == LCe_length) return;

	for(size_t i = total_chars; i > insertion_point; i--) {
		LCe_buffer[i] = LCe_buffer[i - 1];
		x[i] = x[i - 1];
		y[i] = y[i - 1];
	}

	LCe_buffer[insertion_point] = ch;

	insertion_point++;
	total_chars++;

	LCe_buffer[total_chars] = 0;

	refresh();
	while(y[insertion_point] < scroll) scroll--;
	while(y[insertion_point] >= height + scroll) scroll++;
	redraw_all();
}

static void delete() {
	if(!insertion_point) return;

	for(size_t i = insertion_point; i < total_chars; i++) {
		LCe_buffer[i - 1] = LCe_buffer[i];
		x[i - 1] = x[i];
		y[i - 1] = y[i];
	}

	insertion_point--;
	total_chars--;

	LCe_buffer[total_chars] = 0;

	refresh();
	redraw_all();
}


static void cursor_up() {

}

static void cursor_down() {

}

static void cursor_home() {

}

static void cursor_end() {

}


static size_t numch(size_t num) {
	size_t digits;
	for(digits = 1; num >= 10; digits++) num /= 10;
	return digits;
}