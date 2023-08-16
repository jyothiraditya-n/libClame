#define LICENCE_TEXT "\n" \
"  libClame: Command-line Arguments Made Easy\n" \
"  Copyright (C) 2021-2023 Jyothiraditya Nellakra\n" \
"  Demonstration Program for <LC_args.h>\n\n" \
\
"  This program is free software: you can redistribute it and/or modify\n" \
"  it under the terms of the GNU General Public License as published by\n" \
"  the Free Software Foundation, either version 3 of the License, or\n" \
"  (at your option) any later version.\n\n" \
\
"  This program is distributed in the hope that it will be useful,\n" \
"  but WITHOUT ANY WARRANTY; without even the implied warranty of\n" \
"  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the\n" \
"  GNU General Public License for more details.\n\n" \
\
"  You should have received a copy of the GNU General Public License\n" \
"  along with this program. If not, see <https://www.gnu.org/licenses/>.\n\n"

#define HELP_TEXT "\n" \
"  Usage: %s [OPTIONS] [--] [FILES]\n\n" \
\
"  Valid options are:\n" \
"    -a, --about  print the about dialogue\n" \
"    -h, --help   print this help dialogue\n\n" \
\
"    -f, --flag                  set the flag\n" \
"    -m, --message MESSAGE       set the message to MESSAGE\n" \
"    -s, --secret INT            set the secret to INT\n" \
"    -i, --ints INTS... [--]     set the ints to INTS\n" \
"    -c, --coords COORDS... [--] set the coords to COORDS\n\n" \
\
"  Note: A '--' before [FILES] signifies the end of the options. Any\n" \
"        options found after it will be treated as filenames.\n\n" \
\
"  Note: After INTS, you will need two '--'s, as the optional '--'\n" \
"        directly after INTS only signals the end of the INTS and\n" \
"        not the end of the options. The same goes for COORDS.\n\n" \
\
"  Note: You can have either 2 coords or 3 coords, (Let's pretend you\n" \
"        can't have 1D or 4+D coordinates for simplicity, lol.\n\n" \
\
"  Happy coding! :)\n\n"

#include <stdio.h>
#include <stdlib.h>

#define LC_REQ_VER 1
#define LC_REQ_SUBVER 1

#include <libClame.h>
#include <LC_macros.h>

/* We have some different variables to demonstrate the things that the library
 * can handle, as well as the way that someone might go about implementing
 * the library to handle them into their codebase. */

/* The flag and the message are cool because they're datatypes that we handle
 * directly: booleans and null-terminated strings. */
bool flag = false;
char *message = NULL;

/* We need to pass ints through sscanf() and so this will get entered a little
 * differently. However, it also demonstrates how arrays work. */
int secret = 0;
int *ints = NULL;
size_t ints_length = 0;

/* It's the same with floats, but on top of demonstrating how arrays work, we
 * can also specify a minimum and maximum array length. Let's say you can't
 * have less than 2 dimensions or more than 3 dimensions. */
double *coords = NULL;
size_t coords_length = 0;

/* And this is to get all the args on the command line that were specified
 * without preceding flags. */
char **files;
size_t files_length;

/* These helper functions are specified as part of the arguments structure. */
int about_func(LC_flag_t *flag);
int help_func(LC_flag_t *flag);

/* We want an array of the structure for the arguments. This can be specified,
 * as shown here, within C syntax. */
LC_flag_t args[] = {
	/* --about, -a: prints the about dialogue. */
	LC_MAKE_CALL("about", 'a', about_func),
	
	/* --help, -h: prints the help dialogue. */
	LC_MAKE_CALL("help", 'h', help_func),
	
	/* --flag, -f: sets the flag to true. */
	LC_MAKE_BOOL("flag", 'f', flag, true),

	/* --message, -m MESSAGE: sets the message. */
	LC_MAKE_STRING("message", 'm', message),

	/* --secret, -s INT: set the secret number. */
	LC_MAKE_VAR("secret", 's', secret, "%d"),

	/* --ints, -i INTS: set the ints. */
	LC_MAKE_ARR("ints", 'i', ints, "%d", ints_length),

	/* --coords, -c COORDS: set the coords. (2 or 3 values only.) */
	LC_MAKE_ARR_BOUNDED("coords", 'c', coords, "%lf", coords_length, 2, 3)
};

int main(int argc, char **argv) {
	/* Set the LC_flags array to the one that we have, and get the number of
	 * entries automatically using sizeof. */
	LC_flags_length = LC_ARRAY_LENGTH(args);
	LC_flags = args;
	
	/* Get the library to process our args and print out the help dialogue
	 * on a non-system (usage) error. If there is a memory error, then go
	 * ahead and print out an appropriate error message. */
	int ret = LC_read(argc, argv);
	
	switch(ret) {
	case LC_OK: break;

	case LC_MALLOC_ERR:
		printf("%s: error allocating memory.\n", LC_prog_name);
		perror(LC_prog_name); // Cstdlib error report.
		exit(ret);

	default:
		printf("\n" HELP_TEXT, LC_prog_name);
		exit(ret);
	}

	/* Fetch the flagless arguments that the library has collected for us
	 * along with the count of how many there are. */
	files = LC_flagless_args;
	files_length = LC_flagless_args_length;

	/* The flag is set to either true or false, so set or unset. The
	 * message is just a boring old string. */
	printf("\n  Flag: %s\n", flag? "set": "unset");

	/* Check that the message is not NULL as we don't want to pass a NULL
	 * to printf if the message has not been set. */
	if(message) printf("  Message: %s\n", message);

	/* Don't say that we have a secret variable if nobody set it! */
	if(secret) printf("  Secret: %d\n", secret);
	
	/* The integers, coords and files all follow the same pattern, where
	 * we just want to print them out as a list following a header and then
	 * put a couple new lines after them to pad it before the next set. */

	#define print_arr(header, fmt, arr, arr_length) { \
		printf("\n  " header ": "); \
		for(size_t i = 0; i < arr_length; i++) {\
			printf(fmt, arr[i]); \
			\
			/* No comma after the last item. */ \
			if(i + 1 < arr_length) printf(", "); \
		} \
	} \

	print_arr("Ints", "%d", ints, ints_length);
	print_arr("Coords", "%.2lf", coords, coords_length);
	print_arr("Files", "%s", files, files_length);

	/* All done! */
	puts("\n"); // Two extra newlines for padding.
	exit(0);
}

int about_func(LC_flag_t *flag) {
	(void) flag; // We don't use this variable.

	printf(LICENCE_TEXT);

	exit(0);
	return 0; // Never actually returns.
}

int help_func(LC_flag_t *flag) {
	(void) flag; // We don't use this variable.

	printf(HELP_TEXT, LC_prog_name);

	exit(0);
	return 0; // Never actually returns.
}
