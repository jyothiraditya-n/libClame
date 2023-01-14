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

#include <stdlib.h>
#include <string.h>

#include <LC_args.h>

/* Create the symbols for the variables that are externed in the header; set
 * counts to zero and pointers to NULL. */

LCa_t *LC_args = NULL;
size_t LC_args_length = 0;

const char **LCa_flagless_args = NULL;
size_t LCa_flagless_args_length = 0;

int (*LCa_err_function)() = NULL;
int LCa_errno = 0;

/* We'll hold the arguments in a linked list and remove them as we process them
 * so that only the flagless arguments are left in the end. */

typedef struct node_s {
	/* A singly linked list is sufficient for our purposes. */
	struct node_s *next;

	/* The type of a command-line argument is a pointer to a null-
	 * terminated string. */
	const char *string; 

} node_t;

/* We'll need a root node for our linked list, and we'll want a pointer to the
 * node we are currently processing, as that'll need to be referenced and
 * modified across multiple subroutines. */

static node_t root;
static node_t *current;

/* There is only one function of ours that's visible to things outside of this
 * translation unit. We'll want some helper functions to reduce the complexity
 * of the beast, but we'll declare them here and define them later. */

static int evaluate_argument();
static LCa_t *find_flag(const char *lflag, char sflag);

int LCa_read(int argc, char **argv) {
	/* If there's any previously allocated array of flagless arguments,
	 * clear it before we add things to it to prevent a memory leak. */
	if(LCa_flagless_args) {
		free(LCa_flagless_args);
		LCa_flagless_args = NULL; // Prevent a double-free.
	}

	/* Bail if the LC_args array is not properly set up. */
	if(!LC_args) return LCA_NO_ARGS;

	/* Push the arguments onto the root node in from first to last. */
	LCa_flagless_args_length = 0;
	current = &root;

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
		
		current = current -> next;
		current -> next = NULL;
	}

	/* Evaluate every argument, autoforward through the list of arguments
	 * and if we get any errors, pass it back up the call stack. */
	for(current = &root; current; current = current -> next) {
		int ret = evaluate_argument();
		if(ret != LCA_OK) return ret;
	}

	/* At this point all that's left is getting an array for the flagless
	 * arguments and deallocating the list. */

	LCa_flagless_args = malloc(sizeof(const char *)
		* LCa_flagless_args_length);
	if(!LCa_flagless_args) return LCA_MALLOC_ERR;

	/* Loop through the linked list to copy the references and deallocate
	 * the nodes as we go along. */
	node_t *delete = &root;
	current = &root;

	for(size_t i = 0; i < LCa_flagless_args_length; i++) {
		LCa_flagless_args[i] = current -> string;

		/* This process will exit having left current as a NULL
		 * pointer, which is pretty convenient. */
		delete = current;
		current = current -> next;

		/* Don't delete the root node, as that's static allocation. */
		if(delete != &root) free(delete);
	}

	return LCA_OK;
}

static int evaluate_argument() {
	return LCA_OK;
}

static LCa_t *find_flag(const char *lflag, char sflag) {
	/* Loop over the arguments we have in the array, and for each of them
	 * see if the short flags match. Else, spend the time it'll take to
	 * check if the long flag strings match. */

	for(size_t i = 0; i < LC_args_length; i++) {
		if(sflag == LC_args[i].short_flag) return &LC_args[i];

		if(!lflag) continue; // Don't pass any null pointer to strcmp.
		if(!LC_args[i].long_flag) continue;

		if(!strcmp(lflag, LC_args[i].long_flag)) return &LC_args[i];
	}

	/* Return NULL if no argument with either the specified short or long
	 * flag were found. */
	return NULL;
}