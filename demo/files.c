/* libClame: Command-line Arguments Made Easy
 * Copyright (C) 2021-2023 Jyothiraditya Nellakra
 * Demonstration Program for <LC_files.h>
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
#include <string.h>

#include <LC.h>
#include <LC_args.h>
#include <LC_files.h>

/* We have some different variables to demonstrate the things that the library
 * can handle, as well as the way that someone might go about implementing
 * the library to handle them into their codebase. */

/* Note: This is mainly modified from `demo/args.c` so some comments have been
 * omitted for brevity. Go see `demo/args.c` if you'd like more insight. */

/* The flag and the message are cool because they're datatypes that we handle
 * directly: booleans and null-terminated strings. */
bool flag = false;
char *message = NULL;

/* This demonstrates how simple arrays work. */
int *ints = NULL;
size_t ints_length = 0;

/* It's the same with floats, but on top of demonstrating how arrays work, we
 * can also specify a minimum and maximum array length. */
double *coords = NULL;
size_t coords_length = 0;

/* Input and output file names and whether to store descriptive strings. */
char *input = NULL;
char *output = NULL;
bool strings = false;

/* These helper functions are specified as part of the arguments structure. */
int about_flag();
int help_flag();

/* Whether we're printing the help and exiting normally or printing the help
 * because we encountered an error, we are basically printing the same thing so
 * it's useful to have the code shared in this function. */
void help_and_return(int ret);

/* We want an array of the structure for the arguments. This can be specified,
 * as shown here, within C syntax. */
LCa_flag_t args[] = {
	/* The variables are: long_flag, short_flag, function, var_ptr,
	 * var_type, value, fmt_string, arr_length, var_length, min_arr_length,
	 * max_arr_length, readonly. */

	/* --about, -a: prints the about dialogue. */
	{"about", 'a', about_flag, NULL, 0, 0, NULL, NULL, 0, 0, 0, 0},
	
	/* --help, -h: prints the help dialogue. */
	{"help", 'h', help_flag, NULL, 0, 0, NULL, NULL, 0, 0, 0, 0},
	
	/* --flag, -f: sets the flag to true. */
	{"flag", 'f', NULL, &flag, LCA_BOOL_VAR, true, NULL, NULL, 0, 0, 0,
		false},
	
	/* --message, -m MESSAGE: sets the message. */
	{"message", 'm', NULL, &message, LCA_STRING_VAR, 0, NULL, NULL, 0, 0,
		0, false},

	/* --ints, -i INTS: set the ints. */
	{"ints", 'i', NULL, &ints, LCA_OTHER_VAR, 0, "%d", &ints_length,
		sizeof(int), 0, SIZE_MAX, false},

	/* --coords, -c COORDS: set the coords. (2 or 3 values only.) */
	{"coords", 'c', NULL, &coords, LCA_OTHER_VAR, 0, "%lf", &coords_length,
		sizeof(double), 2, 3, false},
	
	/* --input, -I FILE: sets the input file. */
	{"input", 'I', NULL, &input, LCA_STRING_VAR, 0, NULL, NULL, 0, 0, 0,
		false},
	
	/* --output, -O FILE: sets the output message. */
	{"output", 'O', NULL, &output, LCA_STRING_VAR, 0, NULL, NULL, 0, 0, 0,
		false},
	
	/* --strings, -s: we should store descriptive stings in the file. */
	{"strings", 's', NULL, &strings, LCA_BOOL_VAR, true, NULL, NULL, 0, 0,
		0, false}
};

/* We want an array of the structure for the variables. This can be specified,
 * as shown here, within C syntax. */
LCf_var_t vars[] = {
	/* The variables are: name, id, description, type_length, data_type,
	 * allocation_type, data_ptr, static_arr_length, dynamic_arr_length,
	 * min_arr_Length, max_arr_length */

	{"flag", 1, "a boolean serving as an example for a program flag.",
		sizeof(bool), LCF_BOOL_VAR, LCF_STATIC_VAR, &flag, 0, 0, 0, 0},

	{"message", 2, "a null-terminated value serving as an example for a "
		"stored message in a program.", 0, LCF_STRING_VAR,
		LCF_STATIC_VAR, &message, 0, 0, 0, 0},

	{"ints", 3, "an array of unsigned integers similar to what might be "
		"stored in a program.", sizeof(int), LCF_SIGNED_VAR,
		LCF_DYNAMIC_ARR, &ints, 0, &ints_length, 0, SIZE_MAX},

	{"coords", 4, "an array of fudgeable floating point values with some "
		"constraints on how many elements can be present, as an "
		"example of what you might find in a program.", sizeof(double),
		LCF_APPROX_FLOAT, LCF_DYNAMIC_ARR, &coords, 0, &coords_length,
		2, 3}
};

int main(int argc, char **argv) {
	/* Set the LC_flags array to the one that we have, and get the number of
	 * entries automatically using sizeof. */
	LC_flags_length = LC_ARRAY_LENGTH(args);
	LC_flags = args;
	
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

	/* Fetch the count of how many flagless arguments the library has
	 * collected for us. If there are any, then issue an error. */
	if(LCa_flagless_args_length > 0) {
		printf("%s: error: arguments specified without appropriate "
			"flags.\n", LCa_prog_name
		);

		help_and_return(1);

	};

	/* Let us set up our parametes for LCF. */
	LCf_program_name = "libClame/files";
	LCf_program_id = 0x1cfde201; // LCF Demo 1

	LCf_program_description = "An example program for storing and reading "
		"data with libClame.";

	LCf_program_ver = 1;
	LCf_program_subver = 0;

	LCf_file_t file_data = {
		NULL, NULL,   // filename, file,
		false, false, // use_names, save_descriptions,

		vars, LC_ARRAY_LENGTH(vars), // vars, vars_length

		0, 0, 0, 0,    // magic, metadata, version, subversion
		NULL, 0, NULL, // program name, id, description
		0, 0,          // program version, subversion
		0, 0,          // file endianness, bits

		0, 0 // same_endianness, same_bits
	};

	/* If we have an input file, go ahead and read it, overwriting our
	 * values from the command line flags. */
	if(input) {
		file_data.filename = input;
		ret = LCf_read(&file_data);

		/* Check if there's any errors. */
		if(ret != LCF_OK) {
			printf("%s: error: LCf_read() returned %d (%s):\n\n"

				"file information:\n"
				"\tmagic: 0x%" PRIx32 "\n"
				"\tmetadata: 0x%" PRIx8 "\n"
				"\tversion: %" PRIu8 ".%" PRIu8 "\n\n"

				"information about the authoring program:\n"
				"\tname: %s\n"
				"\tid: 0x%" PRIx32 "\n"
				"\tdescription: %s\n"
				"\tversion: %" PRIu8 ".%" PRIu8 "\n\n"

				"general information:\n"
				"\tbits: %d\n\n",

				LCa_prog_name, ret, LCf_error_string(ret),
				file_data.magic, file_data.metadata,
				file_data.version, file_data.subversion,

				file_data.program_name, file_data.program_id,
				file_data.program_description,
				file_data.program_ver,
				file_data.program_subver,

				file_data.bits
			);

			exit(ret);
		}

		/* Assuming there were no errors, we can go ahead and free
		 * any heap-allocated variables generated by the function left
		 * for debugging purposes. */
		if(file_data.program_name) free(file_data.program_name);

		if(file_data.program_description) {
			free(file_data.program_description);
		}
	}

	/* The flag is set to either true or false, so set or unset. The
	 * message is just a boring old string. */
	printf("\n  Flag: %s\n", flag? "set": "unset");

	/* Printf has its own check if the message pointer is NULL. */
	printf("  Message: %s\n", message);
	
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

	puts("\n");

	/* If the message, the ints, or the coords were not set, we need to
	 * make sure the pointers still point to a null piece of memory else
	 * LCF will complain that we're passing it NULL pointers. */
	size_t nothing = 0;

	if(!message) message = (char *) &nothing;
	if(!ints) ints = (int *) &nothing;
	if(!coords) coords = (double *) &nothing;

	/* If we have an output file, go ahead and write to it, overwriting our
	 * values from the command line flags. */
	if(output) {
		/* Save strings if we've been told to do so. */
		file_data.use_names = file_data.save_descriptions = strings;

		/* Pass on our file data. */
		file_data.filename = output;
		ret = LCf_save(&file_data);

		if(ret != LCF_OK) {
			printf("%s: error: LCf_read() returned %d (%s):\n\n",
				LCa_prog_name, ret, LCf_error_string(ret)
			);
		}
	}

	exit(0);
}

int about_flag() {
	putchar('\n');
	puts("  libClame: Command-line Arguments Made Easy");
	puts("  Copyright (C) 2021-2023 Jyothiraditya Nellakra");
	puts("  Demonstration Program for <LC_files.h>\n");

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
	printf("  Usage: %s [OPTIONS] [--]\n\n", LCa_prog_name);

	puts("  Valid options are:");
	puts("    -a, --about  print the about dialogue");
	puts("    -h, --help   print this help dialogue\n");

	puts("    -f, --flag                  set the flag");
	puts("    -m, --message MESSAGE       set the message to MESSAGE");
	puts("    -i, --ints INTS... [--]     set the ints to INTS");
	puts("    -c, --coords COORDS... [--] set the coords to COORDS\n");

	puts("    -I, --input FILE            set the file to read data from.");
	puts("    -O, --output FILE           set the file to save data to.\n");

	puts("  Note: You can have either 2 coords or 3 coords, (Let's pretend you");
	puts("        can't have 1D or 4+D coordinates for simplicity, lol.\n");

	puts("  Happy coding! :)\n");
	exit(ret);
}