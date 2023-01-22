/* libClame: Command-line Arguments Made Easy
 * Copyright (C) 2021-2023 Jyothiraditya Nellakra
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

#include <stdio.h>
#include <stdlib.h>

#include <LC_args.h>

/* We have some different variables to demonstrate the things that the library
 * can handle, as well as the way that someone might go about implementing
 * the library to handle them into their codebase. */

/* The flag and the message are cool because they're datatypes that we handle
 * directly: booleans and null-terminated strings. */
bool flag = false;
char *message = NULL;

/* We need to pass ints through sscanf() and so this will get entered a little
 * differently. However, it also demonstrates how arrays work. */
int *ints = NULL;
size_t ints_length = 0;

/* It's the same with floats, but on top of demonstrating how arrays work, we
 * can also specify a minimum and maximum array length. */
double *coords = NULL;
size_t coords_length = 0;

/* And this is to get all the args on the command line that were specified
 * without preceding flags. */
char **files;
size_t files_length;

/* These helper functions are specified as part of the arguments structure. */
int about_flag();
int help_flag();

/* Whether we're printing the help and exiting normally or printing the help
 * because we encountered an error, we are basically printing the same thing so
 * it's useful to have the code shared in this function. */
void help_and_return(int ret);

/* We want an array of the structure for the arguments. This can be specified,
 * as shown here, within c syntax. */
LCa_flag_t args[] = {
	/* The variables are: long_flag, short_flag, function, var_name,
	 * var_ptr, var_type, value, fmt_string, arr_length, var_length,
	 * min_arr_length, max_arr_length, readonly. */

	/* --about, -a: prints the about dialogue. */
	{"about", 'a', about_flag, NULL, NULL, 0, false, NULL, NULL, 0, 0, 0,
		false},
	
	/* --help, -h: prints the help dialogue. */
	{"help", 'h', help_flag, NULL, NULL, 0, false, NULL, NULL, 0, 0, 0,
		false},
	
	/* --flag, -f: sets the flag to true. */
	{"flag", 'f', NULL, "flag", &flag, LCA_BOOL_VAR, true, NULL, NULL, 0,
		0, 0, false},
	
	/* --message, -m MESSAGE: sets the message. */
	{"message", 'm', NULL, "message", &message, LCA_STRING_VAR, false,
		NULL, NULL, 0, 0, 0, false},

	/* --ints, -i INTS: set the ints. */
	{"ints", 'i', NULL, "ints", &ints, LCA_OTHER_VAR, false, "%d",
		&ints_length, sizeof(int), 0, SIZE_MAX, false},

	/* --coords, -c COORDS: set the coords. (2 or 3 values only.) */
	{"coords", 'c', NULL, "coords", &coords, LCA_OTHER_VAR, false, "%lf",
		&coords_length, sizeof(double), 2, 3, false}
};

int main(int argc, char **argv) {
	/* Set the LC_args array to the one that we have, and get the number of
	 * entries automatically using sizeof. */
	LC_args_length = sizeof(args) / sizeof(LCa_flag_t);
	LC_args = args;
	
	/* Get the library to process our args and print out the help dialogue
	 * on a non-system (usage) error. If there is a memory error, then go
	 * ahead and print out an appropriate error message. */
	int ret = LCa_read(argc, argv);
	
	switch(ret) {
	case LCA_OK: break;

	case LCA_MALLOC_ERR:
		printf("%s: error allocating memory.\n", LCa_prog_name);
		perror(LCa_prog_name); // More precise report from cstdlib.
		break;

	default:
		help_and_return(1);
	}

	/* Fetch the flagless arguments that the library has collected for us
	 * along with the count of how many there are. */
	files = LCa_flagless_args;
	files_length = LCa_flagless_args_length;

	/* The flag is set to either true or false, so set or unset. The
	 * message is just a boring old string. */
	printf("\n  Flag: %s\n", flag? "set": "unset");

	/* Check that the message is not NULL as we don't want to pass a NULL
	 * to printf if the message has not been set. */
	if(message) printf("  Message: %s\n", message);
	
	/* The integers, coords and files all follow the same pattern, where
	 * we just want to print them out as a list following a header and then
	 * put a couple new lines after them to pad it before the next set. */

	printf("  Ints: ");
	for(size_t i = 0; i < ints_length; i++) {
		printf("%d", ints[i]);

		/* Don't print a comma after the last item. */
		if(i + 1 < ints_length) printf(", ");
	}

	printf("\n  Coords: ");
	for(size_t i = 0; i < coords_length; i++) {
		printf("%lf", coords[i]);
		if(i + 1 < coords_length) printf(", ");
	}

	printf("\n  Files: ");
	for(size_t i = 0; i < files_length; i++) {
		printf("%s", files[i]);
		if(i + 1 < files_length) printf(", ");
	}

	puts("\n");
	exit(0);
}

int about_flag() {
	putchar('\n');
	puts("  libClame: Command-line Arguments Made Easy");
	puts("  Copyright (C) 2021-2023 Jyothiraditya Nellakra");
	puts("  Demonstration Program for <LC_args.h>\n");

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
	return 0; // Never actually returns.
}

int help_flag() {
	help_and_return(0);
	return 0; // Never actually returns.
}

void help_and_return(int ret) {
	putchar('\n');
	printf("  Usage: %s [OPTIONS] [--] [FILES]\n\n", LCa_prog_name);

	puts("  Valid options are:");
	puts("    -a, --about  print the about dialogue");
	puts("    -h, --help   print this help dialogue\n");

	puts("    -f, --flag                  set the flag");
	puts("    -m, --message MESSAGE       set the message to MESSAGE");
	puts("    -i, --ints INTS... [--]     set the ints to INTS");
	puts("    -c, --coords COORDS... [--] set the coords to COORDS\n");

	puts("  Note: A '--' before [FILES] signifies the end of the options. Any");
	puts("        options found after it will be treated as filenames.\n");

	puts("  Note: After INTS, you will need two '--'s, as the optional '--'");
	puts("        directly after INTS only signals the end of the INTS and");
	puts("        not the end of the options. The same goes for COORDS.\n");

	puts("  Note: You can have either 2 coords or 3 coords, (Let's pretend you");
	puts("        can't have 1D or 4+D coordinates for simplicity, lol.\n");

	puts("  Happy coding! :)\n");
	exit(ret);
}