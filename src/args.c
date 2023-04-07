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

#include <LC_args.h>

/* Create the symbols for the variables that are externed in the header; set
 * counts to zero and pointers to NULL. */

LCa_flag_t *LC_flags = NULL;
size_t LC_flags_length = 0;

char **LCa_flagless_args = NULL;
size_t LCa_flagless_args_length = 0;

char *LCa_prog_name = NULL;

int (*LCa_err_function)() = NULL;
int LCa_function_errno = 0;

/* We'll hold the arguments in a linked list and remove them as we process them
 * so that only the flagless arguments are left in the end. */

typedef struct node_s {
	/* We'll want a doubly linked list for convenience. */
	struct node_s *next, *prev;

	/* The type of a command-line argument is a pointer to a null-
	 * terminated string. */
	char *string; 

} node_t;

/* We'll need a root node for our linked list, and we'll want a pointer to the
 * node we are currently processing, as that'll need to be referenced and
 * modified across multiple subroutines. */

static node_t root = {NULL, NULL, NULL};
static node_t *current = &root;

/* There is only one function of ours that's visible to things outside of this
 * translation unit. We'll want some helper functions to reduce the complexity
 * of the beast, but we'll declare them here and define them later. */

/* These two evaluate functions deal with inputs that are either long flags or
 * a series of short flags respectively. */
static int evaluate_lflag(node_t *node);
static int evaluate_sflags(node_t *node);

/* This is a helper function for evaluate_sflags() which tells us if we're
 * still processing single-character flags or if used the rest of them as input
 * args for one flag. */
static int evaluate_sflag(node_t *node, char sflag, char *value);
#define LCA_OK_VALUE_USED -1

/* This finds a flag that matches either the specified long or short flag. */
static LCa_flag_t *find_flag(const char *lflag, char sflag);

/* These two find value or values for a variable based on the flag that it
 * was specified in and a first value specified on the command line within the
 * node with the flag itself. */
static int get_strings(LCa_flag_t *flag, node_t *node, char *value);
static int get_others(LCa_flag_t *flag, node_t *node, char *value);

/* This function deletes the next node from the list and return the string
 * stored in it. */
static char *pop_node(node_t *node);

/* This function prints the flag for which an error has occurred, as this
 * unfortunately needs some validness checking that would be too annoying to do
 * in the other functions that are already too deeply nested. */
static void print_flag(LCa_flag_t *flag);

int LCa_read(int argc, char **argv) {
	/* If there's any previously allocated array of flagless arguments,
	 * clear it before we add things to it to prevent a memory leak. */
	if(LCa_flagless_args) {
		free(LCa_flagless_args);
		LCa_flagless_args = NULL; // Prevent a double-free.
	}

	/* Bail if the LC_flags array is not properly set up. */
	if(!LC_flags) return LCA_NO_ARGS;

	/* Push the arguments onto the root node in from first to last. */
	LCa_flagless_args_length = 0;
	current = &root;

	/* Get our program name out. */
	LCa_prog_name = argv[0];

	for(int i = 0; i < argc; i++) {
		LCa_flagless_args_length++;

		/* Get the reference to the argument. */
		current -> string = argv[i];

		/* If there's no more arguments, bail without allocating
		 * memory. */
		if(i == argc - 1) break;

		/* Allocate the memory for the next argument. */
		current -> next = malloc(sizeof(node_t));
		if(!current -> next) return LCA_MALLOC_ERR;

		current -> next -> prev = current;
		current -> next -> next = NULL;
		current = current -> next;
	}

	/* As we loop through our arguments, we'll delete them once we're done
	 * processing them. We delete the arguments that we see, but our sub-
	 * functions evaluate_lflag() and evaluate_sflags() will delete
	 * whatever they process independently of us as well. (Notably, what
	 * they get rid of, we'll never see in this function's for loop.) */
	node_t *delete = NULL;

	/* The root node is our program name so we can skip it.  */
	for(node_t *i = root.next; i; i = i -> next) {
		/* Don't pass a null pointer to free. */
		if(delete != NULL) {
			/* Remove the node and then glue together the two
			 * halves of the list. */
			if(delete -> next) {
				delete -> next -> prev = delete -> prev;
			}

			delete -> prev -> next = delete -> next;

			LCa_flagless_args_length--;
			free(delete);

			/* Prevent any double free()s. */
			delete = NULL;
		}

		/* A `-' by itself is usually used to stand in for stdin or
		 * stdout within most programs so we're not gonna think it's a
		 * flag for any reason. */
		if(!strcmp(i -> string, "-")) continue;

		/* If there's a `--' that's just sitting in the list and it
		 * hasn't been removed by either evaluate_lflag() or
		 * evaluate_sflags(), that means it's marking the end of the
		 * flags on the command line, so we should bail out now. */
		if(!strcmp(i -> string, "--")) {
			/* We want to get rid of the "--" as it isn't a
			 * flagless argument, it's just marks the end of the
			 * array. */
			pop_node(i -> prev);
			break;
		}

		if(i -> string[0] == '-') {
			if(i -> string[1] == '-') {
				int ret = evaluate_lflag(i);
				if(ret != LCA_OK) return ret;
			}

			else {
				int ret = evaluate_sflags(i);
				if(ret != LCA_OK) return ret;
			}
		}

		else continue; // It's a flagless argument; don't delete it.

		/* Mark the current node for deletion */
		delete = i;
	}

	/* We need to delete the last node that we processed. */
	if(delete != NULL) {
		/* Remove the node and then glue together the two halves of the
		 * list. */
		if(delete -> next) delete -> next -> prev = delete -> prev;
		delete -> prev -> next = delete -> next;

		LCa_flagless_args_length--;
		free(delete);
	}

	/* At this point all that's left is getting an array for the flagless
	 * arguments and deallocating the list. We first reduce the count by 1
	 * to account for the root node being just the program name. */
	LCa_flagless_args_length--;

	LCa_flagless_args = malloc(sizeof(char *)
		* LCa_flagless_args_length);
	if(!LCa_flagless_args) return LCA_MALLOC_ERR;

	/* Loop through the linked list to copy the references and deallocate
	 * the nodes as we go along. Again, we can skip the program name or the
	 * first node. */
	delete = root.next;
	current = root.next;

	for(size_t i = 0; i < LCa_flagless_args_length; i++) {
		LCa_flagless_args[i] = current -> string;

		/* This process will exit having left current as a NULL
		 * pointer, which is nice. */
		delete = current;
		current = current -> next;

		/* Don't delete the root node, as that's static allocation. */
		if(delete != &root) free(delete);
	}

	/* Return out successfully. */
	return LCA_OK;
}

static int evaluate_lflag(node_t *node) {
	/* Let's cache this constant value to avoid performance penalties. */
	size_t string_length = strlen(node -> string);

	/* Match the first equals-to sign we find within the string, in case
	 * we have a string of the format `--param=value'. */
	size_t equals_idx = 0;

	for(equals_idx = 0; equals_idx < string_length; equals_idx++) {
		if(node -> string[equals_idx] == '=') break;
	}

	/* Check if we actually found anything. */
	bool has_value = (equals_idx < string_length)? true: false;

	/* Replace the equals symbol with a null character so that we can use
	 * the flag and the value as different strings. */
	if(has_value) node -> string[equals_idx] = 0;

	/* Split the strings and store NULLs if there's no data there. */
	char *lflag = &node -> string[2];
	char *value = has_value? &node -> string[equals_idx + 1]: NULL;

	/* See if we can find the flag this corresponds to. */
	LCa_flag_t *flag = find_flag(lflag, 0);

	if(!flag) {
		fprintf(stderr, "%s: error: unknown flag '%s'.\n",
			LCa_prog_name, lflag
		);

		return LCA_BAD_FLAG;
	}

	/* Make sure that the flag isn't being set for the second time or has
	 * been marked as readonly. */
	if(flag -> readonly) {
		fprintf(stderr, "%s: error: the flag '--%s' has been set "
			"multiple times.\n", LCa_prog_name, lflag
		);

		return LCA_VAR_RESET;
	}

	/* Go ahead and mark that we've started processing this flag. */
	flag -> readonly = true;

	/* Execute the supplied function if there is one. */
	if(flag -> function) {
		int ret = flag -> function();
		
		/* If an error occurred, save the error information and bail
		 * out of flag processing. */
		if(ret != LCA_OK) {
			LCa_err_function = flag -> function;
			LCa_function_errno = ret;
			return LCA_FUNC_ERR;
		}
	}

	/* If there's no variable to be dealt with now, return out. */
	if(!flag -> var_ptr) return LCA_OK;

	/* based on the variable type, pass the flag node and value onto the
	 * appropriate helper function. If it's a boolean though, that's simple
	 * enough for us to deal with ourselves. */
	switch(flag -> var_type) {
	case LCA_STRING_VAR:
		return get_strings(flag, node, value);

	case LCA_BOOL_VAR:
		/* Set the flag variable and exit out. */
		*(bool *) flag -> var_ptr = flag -> value;
		return LCA_OK;

	case LCA_OTHER_VAR:
		return get_others(flag, node, value);

	default:
		return LCA_BAD_VAR_TYPE;
	}
}

static int evaluate_sflags(node_t *node) {
	/* As long as we have characters to process, loop over the flags. We
	 * skip the first index since it's the '-' in `-abc'. */
	for(size_t i = 1; node -> string[i]; i++) {
		int ret = evaluate_sflag(node, node -> string[i],
			&node -> string[i + 1]
		);

		if(ret == LCA_OK_VALUE_USED) return LCA_OK;
		else if(ret != LCA_OK) return ret;
	}

	return LCA_OK;
}

static int evaluate_sflag(node_t *node, char sflag, char *value) {
	/* Check if we actually have a value. */
	bool has_value = strlen(value);
	if(!has_value) value = NULL;

	/* See if we can find the flag this corresponds to. */
	LCa_flag_t *flag = find_flag(NULL, sflag);

	if(!flag) {
		fprintf(stderr, "%s: error: unknown flag '-%c'.\n",
			LCa_prog_name, sflag
		);

		return LCA_BAD_FLAG;
	}

	/* Make sure that the flag isn't being set for the second time or has
	 * been marked as readonly. */
	if(flag -> readonly) {
		fprintf(stderr, "%s: error: the flag '-%c' has been set "
			"multiple times.\n", LCa_prog_name, sflag
		);

		return LCA_VAR_RESET;
	}

	/* Go ahead and mark that we've started processing this flag. */
	flag -> readonly = true;

	/* Execute the supplied function if there is one. */
	if(flag -> function) {
		int ret = flag -> function();
		
		/* If an error occurred, save the error information and bail
		 * out of flag processing. */
		if(ret != LCA_OK) {
			LCa_err_function = flag -> function;
			LCa_function_errno = ret;
			return LCA_FUNC_ERR;
		}
	}

	/* If there's no variable to be dealt with now, return out. */
	if(!flag -> var_ptr) return LCA_OK;

	/* based on the variable type, pass the flag node and value onto the
	 * appropriate helper function. If it's a boolean though, that's simple
	 * enough for us to deal with ourselves. */
	int ret = 0;

	switch(flag -> var_type) {
	case LCA_STRING_VAR:
		ret = get_strings(flag, node, value);
		if(ret == LCA_OK) return has_value? LCA_OK_VALUE_USED: LCA_OK;
		else return ret;

	case LCA_BOOL_VAR:
		/* Set the flag variable and exit out. */
		*(bool *) flag -> var_ptr = flag -> value;
		return LCA_OK;

	case LCA_OTHER_VAR:
		ret = get_others(flag, node, value);
		if(ret == LCA_OK) return has_value? LCA_OK_VALUE_USED: LCA_OK;
		else return ret;

	default:
		return LCA_BAD_VAR_TYPE;
	}
}

static LCa_flag_t *find_flag(const char *lflag, char sflag) {
	/* Loop over the arguments we have in the array, and for each of them
	 * see if the short flags match. Else, spend the time it'll take to
	 * check if the long flag strings match. */

	for(size_t i = 0; i < LC_flags_length; i++) {
		if(sflag == LC_flags[i].short_flag) return &LC_flags[i];

		if(!lflag) continue; // Don't pass any null pointer to strcmp.
		if(!LC_flags[i].long_flag) continue;

		if(!strcmp(lflag, LC_flags[i].long_flag)) return &LC_flags[i];
	}

	/* Return NULL if no argument with either the specified short or long
	 * flag were found. */
	return NULL;
}

static int get_strings(LCa_flag_t *flag, node_t *node, char *value) {
	/* If we're dealing with a single string and that's already provided,
	 * then that makes our lives quite simple. */
	if(!flag -> arr_length && value) {
		*(char **) flag -> var_ptr = value;
		return LCA_OK;
	}

	/* If there's only one value and it's not already been provided, we
	 * need to get it from the next node. */
	else if(!flag -> arr_length) {
		/* Make sure the node exists or else we'll need to issue an
		 * error message. */
		if(!node -> next) {
			fprintf(stderr, "%s: error: the flag ", LCa_prog_name);
			print_flag(flag);
			fprintf(stderr, " needs an additional argument.\n");

			return LCA_NO_VAL;
		}

		/* Get the value and remove the node so that we don't process
		 * it again in the future.*/
		*(char **) flag -> var_ptr = pop_node(node);
		return LCA_OK;
	}

	/* Since we have an array, we're going to need to figure out how many
	 * variables there are and how much space to allocate for them. We
	 * start either with 0 or 1 (counting the string passed in in `value'
	 * and we can cycle forward through the nodes until we hit the string
	 * `--' which marks the end of a list. */
	*(flag -> arr_length) = value? 1: 0;

	for(node_t *i = node; i; i = i -> next) {
		if(strcmp(i -> string, "--")) ++*(flag -> arr_length);
		else {
			/* We want to get rid of the "--" as it isn't a
			 * flagless argument, it's just marks the end of the
			 * array. */
			pop_node(i -> prev);
			break;
		}
	}

	/* If the array has already been allocated, de-allocate it and let's
	 * just hope the user hasn't made it static allocation. */
	if(*(char ***) flag -> var_ptr) free(*(char ***) flag -> var_ptr);

	/* Allocate the memory for the array. Also, yes, the type is cursed. */
	*(char ***) flag -> var_ptr = malloc(
		*(flag -> arr_length) * sizeof(char *)
	);

	if(!*(char ***) flag -> var_ptr) return LCA_MALLOC_ERR;

	/* If we were passed a value directly, then go ahead and copy that over
	 * first. */
	if(value) (*(char ***) flag -> var_ptr)[0] = value;

	/* Loop over and copy the other values. */
	for(size_t i = value? 1: 0; i < *(flag -> arr_length); i++) {
		(*(char ***) flag -> var_ptr)[i] = pop_node(node);
	}

	/* Let's go ahead and verify that the array length is correct. */
	if(*(flag -> arr_length) < flag -> min_arr_length) {
		fprintf(stderr, "%s: error: the flag ", LCa_prog_name);
		print_flag(flag);
		fprintf(stderr, " has too few arguments provided.\n");

		return LCA_LESS_VALS;
	}

	if(*(flag -> arr_length) > flag -> max_arr_length) {
		fprintf(stderr, "%s: error: the flag ", LCa_prog_name);
		print_flag(flag);
		fprintf(stderr, " has too many arguments provided.\n");

		return LCA_MORE_VALS;
	}

	return LCA_OK;
}
static int get_others(LCa_flag_t *flag, node_t *node, char *value) {
	/* If we're dealing with a single string and that's already provided,
	 * then that makes our lives quite simple. */
	if(!flag -> arr_length && value) {
		/* Since we are no longer dealing with just a simple string, we
		 * need to verify that they are the correct format and that
		 * sscanf() didn't choke on the input. */

		if(!flag -> fmt_string) return LCA_NULL_FORMAT_STR;
		int ret = sscanf(value, flag -> fmt_string, flag -> var_ptr);
		
		if(ret != 1) {
			fprintf(stderr, "%s: error: the string `%s' is "
				"invalid for the flag", LCa_prog_name, value
			);
			print_flag(flag);
			fprintf(stderr, ".\n");

			return LCA_BAD_VAL;
		}

		return LCA_OK;
	}

	/* If there's only one value and it's not already been provided, we
	 * need to get it from the next node. */
	else if(!flag -> arr_length) {
		/* Make sure the node exists or else we'll need to issue an
		 * error message. */
		if(!node -> next) {
			fprintf(stderr, "%s: error: the flag ", LCa_prog_name);
			print_flag(flag);
			fprintf(stderr, " needs an additional argument.\n");

			return LCA_NO_VAL;
		}

		/* Get the value and remove the node so that we don't process
		 * it again in the future.*/
		value = pop_node(node);

		if(!flag -> fmt_string) return LCA_NULL_FORMAT_STR;
		int ret = sscanf(value, flag -> fmt_string,
			flag -> var_ptr
		);
		
		if(ret != 1) {
			fprintf(stderr, "%s: error: the string `%s' is "
				"invalid for the flag", LCa_prog_name, value
			);
			print_flag(flag);
			fprintf(stderr, ".\n");

			return LCA_BAD_VAL;
		}

		return LCA_OK;
	}

	/* Since we have an array, we're going to need to figure out how many
	 * variables there are and how much space to allocate for them. We
	 * start either with 0 or 1 (counting the string passed in in `value'
	 * and we can cycle forward through the nodes until we hit the string
	 * `--' which marks the end of a list. Also let's make sure that the
	 * sscanf() doesn't choke on the input to make sure it's valid. */

	/* We need a temporary variable to store the data as sscanf() tries to
	 * read it. */
	char testing_area[flag -> var_length];

	if(!flag -> fmt_string) return LCA_NULL_FORMAT_STR;
	int ret = value? sscanf(value, flag -> fmt_string, testing_area): 1;

	if(ret != 1) {
		fprintf(stderr, "%s: error: the string `%s' is invalid for "
			"the flag", LCa_prog_name, value
		);
		print_flag(flag);
		fprintf(stderr, ".\n");

		return LCA_BAD_VAL;
	}

	*(flag -> arr_length) = value? 1: 0;

	for(node_t *i = node -> next; i; i = i -> next) {
		if(!strcmp(i -> string, "--")) {
			/* We want to get rid of the "--" as it isn't a
			 * flagless argument, it's just marks the end of the
			 * array. */
			pop_node(i -> prev);
			break;
		}

		if(!flag -> fmt_string) return LCA_NULL_FORMAT_STR;
		ret = sscanf(i -> string, flag -> fmt_string, testing_area);
		if(ret != 1) break;

		++*(flag -> arr_length);
	}

	/* If the array has already been allocated, de-allocate it and let's
	 * just hope the user hasn't made it static allocation. */
	if(*(void **) flag -> var_ptr) free(*(void **) flag -> var_ptr);

	/* Allocate the memory for the array. Also, yes, the type is cursed. */
	*(void **) flag -> var_ptr = malloc(
		*(flag -> arr_length) * sizeof(flag -> var_length)
	);

	if(!*(void **) flag -> var_ptr) return LCA_MALLOC_ERR;

	/* If we were passed a value directly, then go ahead and scan that over
	 * first. */
	if(value) {
		if(!flag -> fmt_string) return LCA_NULL_FORMAT_STR;
		sscanf(value, flag -> fmt_string, *(void **) flag -> var_ptr);
	}

	/* Loop over and copy the other values. */
	for(size_t i = value? 1: 0; i < *(flag -> arr_length); i++) {
		/* We need to use char ** here to make sure the compiler
		 * doesn't keep complaining forever about us doing pointer
		 * arithmetic with void *. */
		if(!flag -> fmt_string) return LCA_NULL_FORMAT_STR;
		sscanf(pop_node(node), flag -> fmt_string, 
			*(char **) flag -> var_ptr + i * flag -> var_length
		);
	}

	/* Let's go ahead and verify that the array length is correct. */
	if(*(flag -> arr_length) < flag -> min_arr_length) {
		fprintf(stderr, "%s: error: the flag ", LCa_prog_name);
		print_flag(flag);
		fprintf(stderr, " has too few arguments provided.\n");

		return LCA_LESS_VALS;
	}

	if(*(flag -> arr_length) > flag -> max_arr_length) {
		fprintf(stderr, "%s: error: the flag ", LCa_prog_name);
		print_flag(flag);
		fprintf(stderr, " has too many arguments provided.\n");

		return LCA_MORE_VALS;
	}

	return LCA_OK;
}

static char *pop_node(node_t *node) {
	/* Grab the node after the one we're going to delete and link it back
	 * to the one before the one we're going to delete. */
	node_t *next_to_next = node -> next? node -> next -> next: NULL;
	if(next_to_next) next_to_next -> prev = node;

	/* Get the string out of the node we're going to delete and then delete
	 * it. Then, link the node originally before it to the node originally
	 * after it. */
	char *string = node -> next -> string;
	free(node -> next);
	node -> next = next_to_next;

	LCa_flagless_args_length--;
	return string;
}

static void print_flag(LCa_flag_t *flag) {
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

	/* Notably, we are assuming by this point that either one of them is
	 * set because this function isn't called from a place where that
	 * wouldn't be the case. */
}