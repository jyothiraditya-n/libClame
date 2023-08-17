/* libClame: Command-line Arguments Made Easy
 * Copyright (C) 2021-2023 Jyothiraditya Nellakra
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even- the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>. */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libClame.h>

/* Instantiate most of the variables externed in the header. */
LC_flag_t *LC_flags = NULL;
size_t LC_flags_length = 0;

char **LC_flagless_args = NULL;
size_t LC_flagless_args_length = 0;

char *LC_prog_name = NULL;

/* We'll hold the arguments in a linked list. */
typedef struct node_s {
	struct node_s *next, *prev;

	/* NULL-terminated C strings my beloved. */
	char *string;

} node_t;

/* The root node is statically-allocated. */
static node_t root = {NULL, NULL, NULL};

/* Helper flags to evaluate long and short flags. These will delete all nodes
 * for values relating to the flag except the node that they take as input. */
static int evaluate_lflag(node_t *node);
static int evaluate_sflags(node_t *node);

/* Set to true if we are processing a long flag, false if it's a short flag. */
static bool processing_lflag;

/* Helper for evaluating all sflags in a node. It returns either LC_OK or this
 * custom non-error status. */
static int evaluate_sflag(node_t *node, char sflag, char *value);
#define LC_OK_VALUE_USED -1

/* This finds a flag that matches either the specified long or short flag. */
static LC_flag_t *find_flag(const char *lflag, char sflag);

/* These two find value or values for a variable based on the flag that it
 * was specified in. If a candidate value for was specified in the same node
 * as the flag was specified, then a pointer to the start of that value string
 * is also passed along. */

/* The two functions will pop() off any nodes containing values that they
 * process, except for the node that they were psased as an input. */
static int get_strings(LC_flag_t *flag, node_t *node, char *value);
static int get_others(LC_flag_t *flag, node_t *node, char *value);

/* This function deletes the next node from the list and returns the string
 * stored in it. It returns NULL if there is no next node. */
static char *pop_node(node_t *node);

/* This function prints the flag in terms of its long and short values. */
static void print_flag(LC_flag_t *flag);

/* Our main function. */
int LC_read(int argc, char **argv) {
	/* Get our program name out. */
	LC_prog_name = argv[0];

	/* Bail if the LC_flags array is not properly set up. */
	if(!LC_flags) return LC_NO_ARGS;

	/* If there's any previously allocated array of flagless arguments,
	 * clear it first to avoid memory leaks. */
	if(LC_flagless_args) {
		free(LC_flagless_args);
		LC_flagless_args = NULL;
	}

	LC_flagless_args_length = 0;

	/* Push the arguments into the list; the root node has argv[0]. */
	node_t *current = &root;
	for(int i = 0; i < argc; i++) {
		current -> string = argv[i];

		// Don't initialise node -> next for the last node.
		if(i == argc - 1) break;

		/* Allocate the memory for the next argument. */
		current -> next = malloc(sizeof(node_t));
		if(!current -> next) return LC_MALLOC_ERR;

		current -> next -> prev = current;
		current -> next -> next = NULL;
		current = current -> next;
	}

	LC_flagless_args_length = argc - 1; // Not including the root node.

	/* Iterate through the flags, process them, and delete them if
	 * appropriate. */

	/* We always look at the node after the node in the iterator. */
	for(node_t *i = &root; i -> next;) {
		/* A `-' by itself is usually used to stand in for stdin or
		 * stdout. */
		if(!strcmp(i -> next -> string, "-")) i = i -> next;

		/* If there's a `--' that's just sitting in the list marks the
		 * end of the flags on the command line. */
		else if(!strcmp(i -> next -> string, "--")) {
			pop_node(i);
			break;
		}

		/* One hyphen for short flags, two for long flags. */
		else if(i -> next -> string[0] == '-') {
			if(i -> next -> string[1] == '-') {
				int ret = evaluate_lflag(i -> next);
				if(ret != LC_OK) return ret;
			}

			else {
				int ret = evaluate_sflags(i -> next);
				if(ret != LC_OK) return ret;
			}

			pop_node(i);
			continue;
		}

		/* Flagless argument. */
		i = i -> next;
	}

	/* The remaining nodes are flagless arguments. */

	/* Calling malloc() with a zero size is not portable. */
	LC_flagless_args = LC_flagless_args_length?
		malloc(sizeof(char *) * LC_flagless_args_length) :
		malloc(sizeof(char *));

	if(!LC_flagless_args) return LC_MALLOC_ERR;

	/* Copy the flagless arguments over. */
	for(size_t i = 0; i < LC_flagless_args_length; i++) {
		LC_flagless_args[i] = pop_node(&root);
		LC_flagless_args_length++; // pop_node() decrements this value.
	}

	/* Return out successfully. */
	return LC_OK;
}

static int evaluate_lflag(node_t *node) {
	/* Processing a long flag. */
	processing_lflag = true;

	/* Change the first equal character in the string since to a null byte
	 * since it splits the flag from its (first) value. */
	char *equals_ch = strchr(node -> string, '=');
	if(equals_ch) *equals_ch = 0;

	/* Split the strings and store NULLs if there's no data. */
	char *lflag = &node -> string[2]; // Skip the '--'.
	char *value = equals_ch? equals_ch + 1: NULL;

	/* See if we can find the flag this corresponds to. */
	LC_flag_t *flag = find_flag(lflag, 0);

	if(!flag) {
		fprintf(stderr, "%s: error: unknown flag '--%s'.\n",
			LC_prog_name, lflag
		);

		return LC_BAD_FLAG;
	}

	/* Make sure that the flag isn't being set for the second time. */
	if(flag -> readonly) {
		fprintf(stderr, "%s: error: the flag '--%s' has been set "
			"multiple times.\n", LC_prog_name, lflag
		);

		return LC_VAR_RESET;
	}

	flag -> readonly = true;

	/* If there's no variable but there's a value specified, error out. */
	if(!flag -> var_ptr && value) {
		fprintf(stderr, "%s: error: the flag `--%s' does not take any"
			"values.\n", LC_prog_name, lflag
		);

		return LC_BAD_VAL;
	}

	/* If there's no variable to be dealt with now, skip this section. */
	if(flag -> var_ptr) {
		/* Process the variable. */
		int ret = 0; // Needs to be declared outside of the switch.

		switch(flag -> var_type) {
		case LC_STRING_VAR:
			ret = get_strings(flag, node, value);
			if(ret != LC_OK) return ret;
			break;

		case LC_BOOL_VAR:
			*(bool *) flag -> var_ptr = flag -> value;
			break;

		case LC_OTHER_VAR:
			ret = get_others(flag, node, value);
			if(ret != LC_OK) return ret;
			break;

		default:
			return LC_BAD_VAR_TYPE;
		}
	}

	/* Execute the supplied function if there is one. */
	if(flag -> function) {
		int ret = flag -> function(flag);

		if(ret != LC_OK) {
			/* Save the error information. */
			LC_err_function = flag -> function;
			LC_function_errno = ret;
			return LC_FUNC_ERR;
		}
	}

	return LC_OK;
}

static int evaluate_sflags(node_t *node) {
	/* Processing a long flag. */
	processing_lflag = false;

	/* As long as we have characters to process, loop over the flags.
	 * Also, Ignore the leading `-'. */
	for(size_t i = 1; node -> string[i]; i++) {
		/* If the string has no length, send a NULL instead. */
		int ret = evaluate_sflag(node, node -> string[i],
			node -> string[i + 1]? &node -> string[i + 1]: NULL
		);

		/* Using the value means all the other characters have been
		 * used for that value. Exit out. */
		if(ret == LC_OK_VALUE_USED) return LC_OK;
		else if(ret != LC_OK) return ret;
	}

	return LC_OK;
}

static int evaluate_sflag(node_t *node, char sflag, char *value) {
	/* See if we can find the flag this corresponds to. */
	LC_flag_t *flag = find_flag(NULL, sflag);

	if(!flag) {
		fprintf(stderr, "%s: error: unknown flag '-%c'.\n",
			LC_prog_name, sflag
		);

		return LC_BAD_FLAG;
	}

	/* Make sure that the flag isn't being set for the second time. */
	if(flag -> readonly) {
		fprintf(stderr, "%s: error: the flag '-%c' has been set "
			"multiple times.\n", LC_prog_name, sflag
		);

		return LC_VAR_RESET;
	}

	flag -> readonly = true;

	/* If there's no variable to be dealt with now, skip this section. */
	if(flag -> var_ptr) {
		/* Process the variable. */
		int ret = 0; // Needs to be declared outside of the switch.

		switch(flag -> var_type) {
		case LC_STRING_VAR:
			ret = get_strings(flag, node, value);
			if(ret != LC_OK) return ret;
			break;

		case LC_BOOL_VAR:
			*(bool *) flag -> var_ptr = flag -> value;
			break;

		case LC_OTHER_VAR:
			ret = get_others(flag, node, value);
			if(ret != LC_OK) return ret;
			break;

		default:
			return LC_BAD_VAR_TYPE;
		}
	}

	/* Execute the supplied function if there is one. */
	if(flag -> function) {
		int ret = flag -> function(flag);

		/* Save and bail on errors. */
		if(ret != LC_OK) {
			LC_err_function = flag -> function;
			LC_function_errno = ret;
			return LC_FUNC_ERR;
		}
	}

	/* Bool setting doesn't take a value on the command line, but other
	 * types of variables do. */
	return flag -> var_ptr && flag -> var_type != LC_BOOL_VAR?
		LC_OK_VALUE_USED: LC_OK;
}

static LC_flag_t *find_flag(const char *lflag, char sflag) {
	/* Only check long flags if the short flag doesn't match. */
	for(size_t i = 0; i < LC_flags_length; i++) {
		if(sflag == LC_flags[i].short_flag) return &LC_flags[i];

		if(!lflag) continue; // Don't pass null pointers to strcmp.
		if(!LC_flags[i].long_flag) continue;

		if(!strcmp(lflag, LC_flags[i].long_flag)) return &LC_flags[i];
	}

	/* Return NULL if we found nothing. */
	return NULL;
}

static int get_strings(LC_flag_t *flag, node_t *node, char *value) {
	/* A single string with a given value can be set easily. */
	if(!flag -> arr_length && value) {
		*(char **) flag -> var_ptr = value;
		return LC_OK;
	}

	/* If it's just a single variable, get the value from the next node. */
	if(!flag -> arr_length && !value) {
		/* Get the value and remove the node. */
		*(char **) flag -> var_ptr = pop_node(node);

		if(!*(char **) flag -> var_ptr) {
			fprintf(stderr, "%s: error: the flag ", LC_prog_name);
			print_flag(flag);
			fprintf(stderr, " needs an additional argument.\n");

			return LC_NO_VAL;
		}

		return LC_OK;
	}

	/* Since we have an array, check how many values we have in total. */
	*(flag -> arr_length) = value? 1: 0;

	for(node_t *i = node -> next; i; i = i -> next) {
		/* Check that the value isn't `--'. */
		if(!strcmp(i -> string, "--")) {
			/* Get rid of the `--', which marks the end of the
			 * array. Break out. */
			pop_node(i -> prev);
			break;
		}

		++*(flag -> arr_length);
	}

	/* If the array has already been allocated, de-allocate it. */
	if(*(char ***) flag -> var_ptr) free(*(char ***) flag -> var_ptr);

	/* Allocate the memory for the array. We cannot portably call malloc()
	 * with a size of zero. (This type is cursed.) */
	*(char ***) flag -> var_ptr = *(flag -> arr_length)?
		malloc(*(flag -> arr_length) * sizeof(char *)) :
		malloc(sizeof(char *));

	if(!*(char ***) flag -> var_ptr) return LC_MALLOC_ERR;

	/* Copy or move the values over. */
	if(value) (*(char ***) flag -> var_ptr)[0] = value;

	for(size_t i = value? 1: 0; i < *(flag -> arr_length); i++) {
		(*(char ***) flag -> var_ptr)[i] = pop_node(node);
	}

	/* Let's go ahead and verify that the array length is appropriate. */
	if(*(flag -> arr_length) < flag -> min_arr_length) {
		fprintf(stderr, "%s: error: the flag ", LC_prog_name);
		print_flag(flag);
		fprintf(stderr, " has too few arguments provided.\n");

		return LC_LESS_VALS;
	}

	if(*(flag -> arr_length) > flag -> max_arr_length) {
		fprintf(stderr, "%s: error: the flag ", LC_prog_name);
		print_flag(flag);
		fprintf(stderr, " has too many arguments provided.\n");

		return LC_MORE_VALS;
	}

	return LC_OK;
}
static int get_others(LC_flag_t *flag, node_t *node, char *value) {
	/* A single string with a given value can be set easily. */
	if(!flag -> arr_length && value) {
		/* We need to verify that they are the correct format and that
		 * sscanf() didn't choke on the input. */
		if(!flag -> fmt_string) return LC_NULL_FORMAT_STR;

		/* We want the format string to end with %n, which makes sscanf
		 * give us the number of bytes processed. */
		size_t fmt_len = strlen(flag -> fmt_string);

		char fmt_debug[fmt_len + 4];
		fmt_debug[0] = 0;

		strncat(fmt_debug, flag -> fmt_string, fmt_len);
		strncat(fmt_debug, "%zn", 3);

		size_t bytes = 0;
		int ret = sscanf(value, fmt_debug, flag -> var_ptr, &bytes);
		
		if(ret != 1 || bytes != strlen(value)) {
			fprintf(stderr, "%s: error: the string `%s' is "
				"invalid for the flag", LC_prog_name, value
			);
			print_flag(flag);
			fprintf(stderr, ".\n");

			return LC_BAD_VAL;
		}

		return LC_OK;
	}

	/* If it's just a single variable, get the value from the next node. */
	if(!flag -> arr_length && !value) {
		/* Get the string. */
		value = pop_node(node);

		if(!value) {
			fprintf(stderr, "%s: error: the flag ", LC_prog_name);
			print_flag(flag);
			fprintf(stderr, " needs an additional argument.\n");

			return LC_NO_VAL;
		}

		/* Inject our tests into the format string and process the
		 * value. */
		if(!flag -> fmt_string) return LC_NULL_FORMAT_STR;
		size_t fmt_len = strlen(flag -> fmt_string);

		char fmt_debug[fmt_len + 4];
		fmt_debug[0] = 0;

		strncat(fmt_debug, flag -> fmt_string, fmt_len);
		strncat(fmt_debug, "%zn", 3);

		size_t bytes = 0;
		int ret = sscanf(value, fmt_debug, flag -> var_ptr, &bytes);
		
		if(ret != 1 || bytes != strlen(value)) {
			fprintf(stderr, "%s: error: the string `%s' is "
				"invalid for the flag", LC_prog_name, value
			);
			print_flag(flag);
			fprintf(stderr, ".\n");

			return LC_BAD_VAL;
		}

		return LC_OK;
	}

	/* Since we have an array, we're going to need to figure out how many
	 * values there are in total. */

	/* We need a temporary variable to store the data as sscanf() tries to
	 * read it. */
	char testing_area[flag -> var_length];

	/* Do our standard sscanf tests, reading the value provided to us. */
	if(!flag -> fmt_string) return LC_NULL_FORMAT_STR;
	size_t fmt_len = strlen(flag -> fmt_string);

	char fmt_debug[fmt_len + 4];
	fmt_debug[0] = 0;

	strncat(fmt_debug, flag -> fmt_string, fmt_len);
	strncat(fmt_debug, "%zn", 3);

	size_t bytes = 0;
	size_t len = value? strlen(value): 0;
	int ret = value? sscanf(value, fmt_debug, testing_area, &bytes): 1;

	/* Error out if sscanf() can't read the value provided to us. */
	if(ret != 1 || bytes != len) {
		fprintf(stderr, "%s: error: the string `%s' is invalid for "
			"the flag", LC_prog_name, value
		);
		print_flag(flag);
		fprintf(stderr, ".\n");

		return LC_BAD_VAL;
	}

	/* Since we have an array, check how many values we have in total. */
	*(flag -> arr_length) = value? 1: 0;

	for(node_t *i = node -> next; i; i = i -> next) {
		/* Check that the value isn't `--'. */
		if(!strcmp(i -> string, "--")) {
			/* Get rid of the `--', which marks the end of the
			 * array. Break out. */
			pop_node(i -> prev);
			break;
		}

		/* Check if this is a valid value for the given datatype. */
		ret = sscanf(i -> string, fmt_debug, testing_area, &bytes);
		if(ret != 1 || bytes != strlen(i -> string)) break;

		++*(flag -> arr_length);
	}

	/* If the array has already been allocated, de-allocate it. */
	if(*(void **) flag -> var_ptr) free(*(void **) flag -> var_ptr);

	/* Allocate the memory for the array. We cannot portably call malloc()
	 * with a size of zero. (This type is cursed.) */
	*(void **) flag -> var_ptr = *(flag -> arr_length)?
		malloc(*(flag -> arr_length) * flag -> var_length) :
		malloc(sizeof(void *));

	if(!*(void **) flag -> var_ptr) return LC_MALLOC_ERR;

	/* Scan over the value passed directly to us first. */
	if(value) {
		sscanf(value, flag -> fmt_string, *(void **) flag -> var_ptr);
	}

	/* Loop over and copy all the other values. */
	for(size_t i = value? 1: 0; i < *(flag -> arr_length); i++) {
		/* We need to use char ** here to stop the compiler complaining
		 * about doing pointer arithmetic with void *. */
		sscanf(pop_node(node), flag -> fmt_string,
			*(char **) flag -> var_ptr + i * flag -> var_length
		);
	}

	/* Let's go ahead and verify that the array length is acceptable. */
	if(*(flag -> arr_length) < flag -> min_arr_length) {
		fprintf(stderr, "%s: error: the flag ", LC_prog_name);
		print_flag(flag);
		fprintf(stderr, " has too few arguments provided.\n");

		return LC_LESS_VALS;
	}

	if(*(flag -> arr_length) > flag -> max_arr_length) {
		fprintf(stderr, "%s: error: the flag ", LC_prog_name);
		print_flag(flag);
		fprintf(stderr, " has too many arguments provided.\n");

		return LC_MORE_VALS;
	}

	return LC_OK;
}

static char *pop_node(node_t *node) {
	/* Break out early if someone calls us without any data to pop. */
	if(!node) return NULL;
	if(!node -> next) return NULL;

	/* Bridge previous to next. */
	node_t *next_to_next = node -> next -> next;
	if(next_to_next) next_to_next -> prev = node;

	/* Get the string out of the node and free it. */
	char *string = node -> next -> string;
	free(node -> next);

	/* Bridge next to previous. */
	node -> next = next_to_next;

	LC_flagless_args_length--;
	return string;
}

static void print_flag(LC_flag_t *flag) {
	/* If we're processing a long flag, print long flags preferentially,
	 * and vice versa. One or the other must be set already for us to have
	 * been processing it as a flag. */
	if(processing_lflag) {
		if(flag -> long_flag) {
			fprintf(stderr, "'--%s'", flag -> long_flag);
		}

		else if(isprint(flag -> short_flag)) {
			fprintf(stderr, "'-%c'", flag -> short_flag);
		}
	}

	else {
		if(isprint(flag -> short_flag)) {
			fprintf(stderr, "'-%c'", flag -> short_flag);
		}

		else if(flag -> long_flag) {
			fprintf(stderr, "'--%s'", flag -> long_flag);
		}
	}
}
