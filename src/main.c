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

/* Create the symbols for the variables that are externed in the header. */
LC_flag_t *LC_flags = NULL;
size_t LC_flags_length = 0;

char **LC_flagless_args = NULL;
size_t LC_flagless_args_length = 0;

char *LC_prog_name = NULL;

int (*LC_err_function)() = NULL;
int LC_function_errno = 0;

/* We'll hold the arguments in a linked list. */
typedef struct node_s {
	struct node_s *next, *prev;

	/* NULL-terminated C strings my beloved. */
	char *string;

} node_t;

/* The root node is statically-allocated and we'll have a pointer to the
 * argument currently being processed. */
static node_t root = {NULL, NULL, NULL};
static node_t *current = &root;

/* Helper flags to evaluate long and short flags. */
static int evaluate_lflag(node_t *node);
static int evaluate_sflags(node_t *node);

/* Helper for evaluating all sflags in a node. It returns either LC_OK or this
 * custom non-error status. */
static int evaluate_sflag(node_t *node, char sflag, char *value);
#define LC_OK_VALUE_USED -1

/* This finds a flag that matches either the specified long or short flag. */
static LC_flag_t *find_flag(const char *lflag, char sflag);

/* These two find value or values for a variable based on the flag that it
 * was specified in and a first value specified on the command line within the
 * node with the flag itself. */
static int get_strings(LC_flag_t *flag, node_t *node, char *value);
static int get_others(LC_flag_t *flag, node_t *node, char *value);

/* This function deletes the next node from the list and returns the string
 * stored in it. */
static char *pop_node(node_t *node);

/* This function prints the flag for when an error has occurred. */
static void print_flag(LC_flag_t *flag);

/* Our main function. */
int LC_read(int argc, char **argv) {
	/* If there's any previously allocated array of flagless arguments,
	 * clear it before we add things to it to prevent a memory leak. */
	if(LC_flagless_args) {
		free(LC_flagless_args);
		LC_flagless_args = NULL;
	}

	/* Bail if the LC_flags array is not properly set up. */
	if(!LC_flags) return LC_NO_ARGS;

	/* Push the arguments onto the root node in from first to last. */
	LC_flagless_args_length = 0;
	current = &root;

	/* Get our program name out. */
	LC_prog_name = argv[0];

	/* Scan in the arguments. */
	for(int i = 0; i < argc; i++) {
		LC_flagless_args_length++;

		/* Get the reference to the argument. */
		current -> string = argv[i];

		/* If there's no more arguments, bail without allocating
		 * memory. */
		if(i == argc - 1) break;

		/* Allocate the memory for the next argument. */
		current -> next = malloc(sizeof(node_t));
		if(!current -> next) return LC_MALLOC_ERR;

		current -> next -> prev = current;
		current -> next -> next = NULL;
		current = current -> next;
	}

	node_t *delete = NULL; // Used to mark for node deletion.

	/* The root node is our program name so we can skip it.  */
	for(node_t *i = root.next; i;) { // Iterate in main loop body.
		/* A `-' by itself is usually used to stand in for stdin or
		 * stdout. */
		if(!strcmp(i -> string, "-")) goto cont;

		/* If there's a `--' that's just sitting in the list marks the
		 * end of the flags on the command line. */
		if(!strcmp(i -> string, "--")) {
			pop_node(i -> prev);
			break;
		}

		/* Two hyphens for long flags, one for short flags. */
		if(i -> string[0] == '-') {
			if(i -> string[1] == '-') {
				int ret = evaluate_lflag(i);
				if(ret != LC_OK) return ret;
			}

			else {
				int ret = evaluate_sflags(i);
				if(ret != LC_OK) return ret;
			}

		}

		else goto cont; // It's a flagless argument; don't delete it.

		/* Mark the current node for deletion and step forward. */
		delete = i;
	cont:	i = i -> next;

		/* Destroy nodes marked for deletion. */
		if(delete == NULL) continue;

		/* Remove the node and then glue together the two halves of the
		 * list. */
		if(delete -> next) delete -> next -> prev = delete -> prev;

		delete -> prev -> next = delete -> next;
		LC_flagless_args_length--;

		free(delete);
		delete = NULL;
	}

	/* At this point all that's left is getting an array for the flagless
	 * arguments and deallocating the list. We first reduce the count by 1
	 * to account for the root node being just the program name. */
	LC_flagless_args_length--;

	/* Calling malloc() with a potential 0 is not portable. */
	LC_flagless_args = LC_flagless_args_length?
		malloc(sizeof(char *) * LC_flagless_args_length) :
		malloc(sizeof(char *));

	if(!LC_flagless_args) return LC_MALLOC_ERR;

	/* Loop through the linked list to copy the references and deallocate
	 * the nodes as we go along. */
	current = root.next;

	for(size_t i = 0; i < LC_flagless_args_length; i++) {
		LC_flagless_args[i] = current -> string;

		/* This process will exit having left current as a NULL
		* pointer, which is nice. */
		node_t *delete = current;
		current = current -> next;

		/* Don't delete the root node, as that's static allocation. */
		if(delete != &root) free(delete);
	}

	/* Return out successfully. */
	return LC_OK;
}

static int evaluate_lflag(node_t *node) {
	/* Match the first equals-to sign we find within the string, in case
	 * we have a string of the format `--param=value'. */
	char *equals_ch = strchr(node -> string, '=');

	/* Change it to a NULL char if we found it. */
	if(equals_ch) *equals_ch = 0;

	/* Split the strings and store NULLs if there's no data there. */
	char *lflag = &node -> string[2];
	char *value = equals_ch? equals_ch + 1: NULL;

	/* See if we can find the flag this corresponds to. */
	LC_flag_t *flag = find_flag(lflag, 0);

	if(!flag) {
		fprintf(stderr, "%s: error: unknown flag '%s'.\n",
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
			/* Set the flag variable and exit out. */
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

		/* Save error information and bail if need be. */
		if(ret != LC_OK) {
			LC_err_function = flag -> function;
			LC_function_errno = ret;
			return LC_FUNC_ERR;
		}
	}

	return LC_OK;
}

static int evaluate_sflags(node_t *node) {
	/* As long as we have characters to process, loop over the flags.
	 * Ignore the leading `-', though. */
	for(size_t i = 1; node -> string[i]; i++) {
		int ret = evaluate_sflag(node, node -> string[i],
			&node -> string[i + 1]
		);

		if(ret == LC_OK_VALUE_USED) return LC_OK;
		else if(ret != LC_OK) return ret;
	}

	return LC_OK;
}

static int evaluate_sflag(node_t *node, char sflag, char *value) {
	/* Check if we actually have a value. */
	bool has_value = strlen(value);
	if(!has_value) value = NULL;

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
			/* Set the flag variable and exit out. */
			*(bool *) flag -> var_ptr = flag -> value;
			has_value = false;
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

	return has_value && flag -> var_ptr? LC_OK_VALUE_USED: LC_OK;
}

static LC_flag_t *find_flag(const char *lflag, char sflag) {
	/* Only check long flags if the short flag doesn't match. */
	for(size_t i = 0; i < LC_flags_length; i++) {
		if(sflag == LC_flags[i].short_flag) return &LC_flags[i];

		if(!lflag) continue; // Don't pass null pointer to strcmp.
		if(!LC_flags[i].long_flag) continue;

		if(!strcmp(lflag, LC_flags[i].long_flag)) return &LC_flags[i];
	}

	/* Return NULL if we found nothing. */
	return NULL;
}

static int get_strings(LC_flag_t *flag, node_t *node, char *value) {
	/* If we're dealing with a single string and that's already provided,
	 * then that makes our lives quite simple. */
	if(!flag -> arr_length && value) {
		*(char **) flag -> var_ptr = value;
		return LC_OK;
	}

	/* If there's only one value and it's not already been provided, we
	 * need to get it from the next node. */
	else if(!flag -> arr_length) {
		/* Make sure the node exists. */
		if(!node -> next) {
			fprintf(stderr, "%s: error: the flag ", LC_prog_name);
			print_flag(flag);
			fprintf(stderr, " needs an additional argument.\n");

			return LC_NO_VAL;
		}

		/* Get the value and remove the node. */
		*(char **) flag -> var_ptr = pop_node(node);
		return LC_OK;
	}

	/* Since we have an array, we're going to need to figure out how many
	 * variables there are and how much space to allocate for them. */
	*(flag -> arr_length) = value? 1: 0;

	for(node_t *i = node -> next; i; i = i -> next) {
		if(strcmp(i -> string, "--")) ++*(flag -> arr_length);
		else {
			/* We can simply get rid of the `--'. */
			pop_node(i -> prev);
			break;
		}
	}

	/* If the array has already been allocated, de-allocate it and let's
	 * just hope the user hasn't made it static allocation. */
	if(*(char ***) flag -> var_ptr) free(*(char ***) flag -> var_ptr);

	/* Cannot portably call malloc() with a size of zero. */
	/* Allocate the memory for the array. Also, yes, the type is cursed. */
	*(char ***) flag -> var_ptr = *(flag -> arr_length)?
		malloc(*(flag -> arr_length) * sizeof(char *)) :
		malloc(sizeof(char *));

	if(!*(char ***) flag -> var_ptr) return LC_MALLOC_ERR;

	/* Copy value passed to us first. */
	if(value) (*(char ***) flag -> var_ptr)[0] = value;

	/* Loop over and copy the other values. */
	for(size_t i = value? 1: 0; i < *(flag -> arr_length); i++) {
		(*(char ***) flag -> var_ptr)[i] = pop_node(node);
	}

	/* Let's go ahead and verify that the array length is correct. */
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
	/* If we're dealing with a single string and that's already provided,
	 * then that makes our lives quite simple. */
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

	/* If there's only one value and it's not already been provided, we
	 * need to get it from the next node. */
	else if(!flag -> arr_length) {
		/* Make sure the node exists. */
		if(!node -> next) {
			fprintf(stderr, "%s: error: the flag ", LC_prog_name);
			print_flag(flag);
			fprintf(stderr, " needs an additional argument.\n");

			return LC_NO_VAL;
		}

		/* Get the value and remove the node. */
		value = pop_node(node);

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
	 * variables there are and how much space to allocate for them. */

	/* We need a temporary variable to store the data as sscanf() tries to
	 * read it. */
	char testing_area[flag -> var_length];

	if(!flag -> fmt_string) return LC_NULL_FORMAT_STR;
	size_t fmt_len = strlen(flag -> fmt_string);

	char fmt_debug[fmt_len + 4];
	fmt_debug[0] = 0;

	strncat(fmt_debug, flag -> fmt_string, fmt_len);
	strncat(fmt_debug, "%zn", 3);

	size_t bytes = 0;
	size_t len = value? strlen(value): 0;
	int ret = value? sscanf(value, fmt_debug, testing_area, &bytes): 1;

	/* Error out if sscanf() can't read the value. */
	if(ret != 1 || bytes != len) {
		fprintf(stderr, "%s: error: the string `%s' is invalid for "
			"the flag", LC_prog_name, value
		);
		print_flag(flag);
		fprintf(stderr, ".\n");

		return LC_BAD_VAL;
	}

	*(flag -> arr_length) = value? 1: 0;

	for(node_t *i = node -> next; i; i = i -> next) {
		if(!strcmp(i -> string, "--")) {
			/* We want to get rid of the "--". */
			pop_node(i -> prev);
			break;
		}

		ret = sscanf(i -> string, fmt_debug, testing_area, &bytes);
		if(ret != 1 || bytes != strlen(i -> string)) break;

		++*(flag -> arr_length);
	}

	/* If the array has already been allocated, de-allocate it and let's
	 * just hope the user hasn't made it static allocation. */
	if(*(void **) flag -> var_ptr) free(*(void **) flag -> var_ptr);

	/* Allocate the memory for the array. Also, yes, another cured type. */
	/* Cannot portably call malloc() with a size of zero. */
	*(void **) flag -> var_ptr = *(flag -> arr_length)?
		malloc(*(flag -> arr_length) * flag -> var_length) :
		malloc(sizeof(void *));

	if(!*(void **) flag -> var_ptr) return LC_MALLOC_ERR;

	/* If we were passed a value directly, then go ahead and scan that over
	 * first. */
	if(value) {
		if(!flag -> fmt_string) return LC_NULL_FORMAT_STR;
		sscanf(value, flag -> fmt_string, *(void **) flag -> var_ptr);
	}

	/* Loop over and copy the other values. */
	for(size_t i = value? 1: 0; i < *(flag -> arr_length); i++) {
		/* We need to use char ** here to stop the compiler complaining
		 * about doing pointer arithmetic with void *. */
		if(!flag -> fmt_string) return LC_NULL_FORMAT_STR;
		sscanf(pop_node(node), flag -> fmt_string, 
			*(char **) flag -> var_ptr + i * flag -> var_length
		);
	}

	/* Let's go ahead and verify that the array length is correct. */
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

	/* Get the string out of the node we're going to delete and then delete
	 * it. */
	char *string = node -> next -> string;
	free(node -> next);

	/* Bridge next to previous. */
	node -> next = next_to_next;

	LC_flagless_args_length--;
	return string;
}

static void print_flag(LC_flag_t *flag) {
	/* If both the long and short flags are set, then print them both. */
	if(flag -> long_flag && isprint(flag -> short_flag)) {
		fprintf(stderr, "'--%s' / '-%c'",
			flag -> long_flag, flag -> short_flag
		);
	}

	/* If only one or the other is set, then print the appropriate one. */
	else if(flag -> long_flag) {
		fprintf(stderr, "'--%s'", flag -> long_flag);
	}

	else {
		fprintf(stderr, "'-%c'", flag -> short_flag);
	}
}

/* Get error strings. */
const char *LC_strerror(int error) {
	/* Return the compile-time constant's name. */
	switch(error) {
		case LC_OK: return "LC_OK";
		case LC_NO_ARGS: return "LC_NO_ARGS";
		case LC_MALLOC_ERR: return "LC_MALLOC_ERR";

		case LC_BAD_FLAG: return "LC_BAD_FLAG";
		case LC_VAR_RESET: return "LC_VAR_RESET";
		case LC_NO_VAL: return "LC_NO_VAL";
		case LC_BAD_VAL: return "LC_BAD_VAL";
		case LC_LESS_VALS: return "LC_LESS_VALS";
		case LC_MORE_VALS: return "LC_MORE_VALS";
		case LC_FUNC_ERR: return "LC_FUNC_ERR";

		case LC_BAD_VAR_TYPE: return "LC_BAD_VAR_TYPE";
		case LC_NULL_FORMAT_STR: return "LC_NULL_FORMAT_STR";
	}

	/* We have an invalid error number. */
	return "LC_UNKNOWN_ERR";
}
