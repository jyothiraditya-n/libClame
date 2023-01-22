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

/* Begin Header Guard */
#ifndef LC_ARGS_H
#define LC_ARGS_H 1

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Version Information */
#define LCA_VERSION 1 /* Incremented when backwards compatibility broken. */
#define LCA_SUBVERSION 0 /* Incremented when new features added. */

/* Each argument that we process has properties specified by an LCa_flag_t
 * struct. A run down of what all of the properties do and mean is written in
 * the comments within the structure definition itself. */

typedef struct {
	/* The long flag is what you specify when you type `--<something>`
	 * while `-<something> specifies a series of short flags`, each of
	 * which is one character long. */

	const char *long_flag;
	char short_flag;

	/* If you have a function that takes no parameters and returns an
	 * integer, you can point to it here to be run when the flag is found.
	 * If you don't have anything you want to run, set the pointer to NULL
	 * so that we can detect that and avoid causing a segfault. */

	int (*function)();

	/* If the function executed correctly, go ahead and return 0. Any other
	 * value will be treated as an error. When that occurs, we will save
	 * the returned value as well as the function pointer as specified
	 * later on in the header, for you to process through later. */

	#define LCA_FUNCTION_OK 0
	#define LCA_FUNCTION_ERR (!LCA_FUNCTION_OK)

	/* If the flag is used to set a variable in the code, then you can
	 * specify the variable's name as well as its type and location with
	 * these parameters. */

	const char *var_name; // Set this to NULL if you don't have a variable.
	void *var_ptr; // Set this to NULL if you don't have a variable.
	int var_type; // Set this to 0 if you don't have a variable.

	/* If the variable you want to get is a string constant (i.e. a
	 * pointer to the start of a null-terminated set of characters in
	 * memory) we can set the pointer for you directly without needing a
	 * format string to pass to sscanf. In this case, var_ptr is
	 * transparently treated as a (char *)* type. We can also set a
	 * boolean value directly to the value in bool_value, wherein var_ptr
	 * is of type (bool *). */

	#define LCA_STRING_VAR 0
	#define LCA_BOOL_VAR 1
	#define LCA_OTHER_VAR 2

	bool value; // Set this to false if you aren't going to use it.

	/* For all other types of variables, we'll need a format string that'll
	 * get passed as the second argument to sscanf where the first argument
	 * is the string that we've gotten corresponding to the specified data
	 * and the third argument is `var_ptr`. For example, if you want to
	 * get an integer value, you would set var_ptr to &your_integer and
	 * var_ptr would thus transparently be an int* type. */

	const char *fmt_string; // Set this to NULL if you don't have anything.

	/* If you want to get a set of values as a dynamically allocated array
	 * then you'll just need to give us the length of each array member as
	 * well as a size variable to store the final array size in. Then, if
	 * you want an array of typename T values, you'll want to set var_ptr
	 * to be a pointer to a variable of type T*, i.e. var_ptr will
	 * transparently be of type T**. */

	size_t *arr_length; // Set to NULL if it isn't an array.
	size_t var_length; // Set to 0 if it isn't an array.

	/* If you want minimum or maximum length constraints specified, you can
	 * do so with these two. There is however some nuance with the way they
	 * work that you would want to read about in <docs/LC_args.md>. */

	size_t min_arr_length; // Set to 0 to disable checking.
	size_t max_arr_length; // Set to SIZE_MAX to disable checking.

	/* We need a boolean to keep track of whether a variable has already
	 * been written to, so that we don't allow conflicting flags. */

	bool readonly; // Set this to false by default.

} LCa_flag_t;

/* You'll want to set the pointer to point to the start of an array containing
 * the parameters for your programs command-line arguments. Then, set the size
 * variable to make sure we don't run off the end of the array's data range. */

extern LCa_flag_t *LC_args;
extern size_t LC_args_length;

/* Once you have set up the variables above, you can call the LCa_read function
 * to process your command line arguments. */

extern int LCa_read(int argc, char **argv);

/* The following are the menaings of the returned values from LCa_read(). */

#define LCA_OK 0 // No errors occurred.
#define LCA_NO_ARGS 1 // LC_args is a NULL pointer.
#define LCA_MALLOC_ERR 2 // malloc() returned a NULL pointer.

#define LCA_BAD_FLAG 3 // A malformed flag was supplied to the program.
#define LCA_VAR_RESET 4 // A variable was set twice on the command line.
#define LCA_NO_VAL 5 // No value was supplied to a flag that sets a variable.
#define LCA_BAD_VAL 6 // A malformed value was supplied.
#define LCA_LESS_VALS 7 // Fewer values were supplied than the flag accepts.
#define LCA_MORE_VALS 8 // More values were supplied than the flag accepts.
#define LCA_FUNC_ERR 9 // A user-defined function returned an error.

#define LCA_BAD_VAR_TYPE 10 // The specified flag var_type is invalid.

/* These variables are set by the LCa_read() function when it encounters
 * arguments supplied to the program that were not preceded by a flag. */

extern char **LCa_flagless_args;
extern size_t LCa_flagless_args_length;

/* We also get the program name out of argv[0]. */
extern char *LCa_prog_name;

/* These variables are set by the LCa_read() function if a user-defined
 * function returned an error code of some sort. */

extern int (*LCa_err_function)();
extern int LCa_function_errno;

/* End Header Guard */
#endif