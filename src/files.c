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
#include <string.h>

#include <LC_files.h>

/* Create the symbols for the variables that are externed in the header; set
 * counts to zero and pointers to NULL. */

const char *LCf_program_name = NULL;
uint32_t LCf_program_id = 0;

const char *LCf_program_description = NULL;
uint8_t LCf_program_ver = 0;
uint8_t LCf_program_subver = 0;

/* We need our headerdata to be in the from of variables so that we can copy
 * it to file using fwrite(). */

static const uint32_t magic = LCF_MAGIC;
static const uint8_t version = LCF_VERSION;
static const uint8_t subversion = LCF_SUBVERSION;

/* When writing, it doesn't matter if the data is statically or dynamically
 * allocated within the program memory, so we have a helper function for
 * compressing that down to a flat mapping. */

typedef struct {
	int ret; // Did the function succeed or did it error out?
	bool is_array; // Are we writing an array or just a single variable?

	void *data_ptr; // Pointer to the start of the data.
	size_t type_length; // Length of the data type.
	size_t array_length; // Length of the array.

} flat_var_t;

static flat_var_t flatten(LCf_var_t *var);

/* We'll want to offload the writing of variable data to different functions
 * based on its type. The functions are also responsible for returning errors
 * if the variable struture has errors. */

static int write_string(FILE *file, flat_var_t *var);
static int write_integer(FILE *file, flat_var_t *var, bool signed_int);
static int write_other(FILE *file, flat_var_t *var);

/* In order to read or write the boolean data, we need to store the booleans as
 * we encounter them in a linked list, which will all be turned into bits in
 * the final file. */

typedef struct _node_s {
	struct _node_s *next;
	bool value;

} node_t;

static node_t *starting_node = NULL;
static node_t *current_node = NULL;
static size_t nodes_length = 0;

static void bool_clear();

static int write_bool_init(FILE *file, flat_var_t *var);
static int write_bool_fini(FILE *file);

/* Notably, floating point values are folded into the category of other values
 * when being written. */

/* Let's start defining our helper functions for writing data to disk. */

/* This function is responsible for flattening the dynamic/static distinction,
 * which doesn't matter much for writing to disk. */
static flat_var_t flatten(LCf_var_t *var) {
	/* This is the structure that we'll be retruning. */
	flat_var_t flat_var = {LCF_OK, 0, 0, 0, 0};

	switch(var -> allocation_type) {
	case LCF_DYNAMIC_VAR:
		flat_var.is_array = false; // Not an array.

		/* If it's a dynamic variable, it's a pointer to a pointer that
		 * was passed to us, so we need to make sure it's not NULL
		 * before we dereference it in the next step. */
		if(!var -> data_ptr) {
			flat_var.ret = LCF_NULL_DATA_PTR;
			break;
		}

		/* We need to make sure the array that the pointer to the
		 * actual data is not NULL either. */
		if(!*(void **) var -> data_ptr) {
			flat_var.ret = LCF_NULL_DATA_PTR;
			break;
		}

		flat_var.data_ptr = *(void **) var -> data_ptr;
		flat_var.type_length = var -> type_length;

		/* We want to store a variable as a single-length array. */
		flat_var.array_length = 1;
		break;

	case LCF_STATIC_VAR:
		flat_var.is_array = false; // Not an array.

		/* If it's a dynamic variable, it's a pointer that we got, so
		 * we just need to make sure it's not NULL. */
		if(!var -> data_ptr) {
			flat_var.ret = LCF_NULL_DATA_PTR;
			break;
		}

		flat_var.data_ptr = var -> data_ptr;
		flat_var.type_length = var -> type_length;

		/* We want to store a variable as a single-length array. */
		flat_var.array_length = 1;
		break;

	case LCF_DYNAMIC_ARR:
		flat_var.is_array = true; // It is an array.

		/* If it's a dynamic variable, it's a pointer to a pointer that
		 * was passed to us, so we need to make sure it's not NULL
		 * before we dereference it in the next step. */
		if(!var -> data_ptr) {
			flat_var.ret = LCF_NULL_DATA_PTR;
			break;
		}

		/* We need to make sure the array that's the pointer to the
		 * actual data is not NULL either. */
		if(!*(void **) var -> data_ptr) {
			flat_var.ret = LCF_NULL_DATA_PTR;
			break;
		}

		flat_var.data_ptr = *(void **) var -> data_ptr;
		flat_var.type_length = var -> type_length;

		/* We need to check that the length that is passed along as a
		 * pointer to a size_t isn't a NULL pointer before we go ahead
		 * and dereference it. */
		if(!var -> dynamic_arr_length) {
			flat_var.ret = LCF_NULL_ARR_LEN_PTR;
			break;
		}

		flat_var.array_length = *(var -> dynamic_arr_length);
		break;

	case LCF_STATIC_ARR:
		flat_var.is_array = true; // It is an array.

		/* If it's a dynamic variable, it's a pointer that we got, so
		 * we just need to make sure it's not NULL. */
		if(!var -> data_ptr) {
			flat_var.ret = LCF_NULL_DATA_PTR;
			break;
		}

		flat_var.data_ptr = var -> data_ptr;
		flat_var.type_length = var -> type_length;
		flat_var.array_length = var -> static_arr_length;
		break;

	default:
		/* Return an error that we didn't have a valid type of data
		 * allocation. */
		flat_var.ret = LCF_BAD_ALLOC_SPEC;
	}

	return flat_var;
}

static int write_string(FILE *file, flat_var_t *var) {
	int ret; // Store a return value from various functions.

	/* Write the length out for the array, if applicable. */
	if(var -> is_array) {
		ret = fwrite(&(var -> array_length), sizeof(size_t), 1, file);
		if(ret == EOF) return LCF_FILEIO_ERR;
	}

	/* Loop over the strings and write them out. */
	for(size_t i = 0; i < var -> array_length; i++) {
		/* Make sure we don't dereference a NULL pointer first. */
		char *string = ((char **) var -> data_ptr)[i];
		if(!string) return LCF_NULL_DATA_PTR;

		ret = fwrite(string, sizeof(char), strlen(string) + 1, file);
		if(ret == EOF) return LCF_FILEIO_ERR;
	}

	ret = fwrite(
		var -> data_ptr, var -> type_length,
		var -> array_length, file
	);
	
	if(ret == EOF) return LCF_FILEIO_ERR;
	else return LCF_OK;
}

static int write_integer(FILE *file, flat_var_t *var, bool signed_int) {
	/* We need to find the minimum number of bytes needed to store the
	 * values. We do this by looping through all of the values given to us
	 * as arrays of bytes, starting with the most significant byte and
	 * exiting out when we encounter the most significant byte that has
	 * data for any of the values passed to us. */

	/* We can't do arithmetic on a void pointer without C yelling at us,
	 * so let's convert the array base to a byte pointer instead. */
	uint8_t *base_ptr = (uint8_t *) var -> data_ptr;

	/* The bounds check on the outer for loop is weird because of unsigned
	 * int overflow. We'll keep the looping variable in function scope
	 * since we'll need it later. */

	/* This variable is needed by the inner loop and must persist between
	 * loop iterations. */
	uint8_t *last_bytes = malloc(sizeof(uint8_t) * var -> array_length);
	if(!last_bytes) return LCF_MALLOC_ERR;

	size_t bytes; // The number of bytes we'll need to store.
	for(bytes = var -> type_length; bytes <= var -> type_length; bytes--) {
		bool bits_set_in_byte = false;
		bool increase_bytes = false;

		/* Notably, variables are stored as single-length arrays due
		 * to the way that the array length is set when flattening. */
		for(size_t i = 0; i < var -> array_length; i++) {
			/* (base_ptr + (i * var -> type_length)) is the ith
			 * element of the array. */
			uint8_t *ptr = base_ptr + (i * var -> type_length);

		#if (LC_ENDIANNESS == LC_BIG_ENDIAN)
			ptr += var -> type_length - bytes;

			/* Adding (type_length - bytes) to it has the effect of 
			 * looking at the first byte of the variable and then 
			 * the second and then the third as the value of bytes 
			 * increases. This corresponds to less significant 
			 * bytes on a big endian system. */
		#else
			ptr += bytes - 1;

			/* Adding (bytes - 1) to it has the effect of looking 
			 * at the last byte, which is the most significant byte
			 * in a little-endian system for a type of length 
			 * bytes. */
		#endif

			/* Now that we have the pointer to the right byte, if
			 * any bits are set in it and we're looking at unsigned
			 * numbers, we know to indicate it. */
			if(*ptr && !signed_int) {
				bits_set_in_byte = true;
				break;
			}

			else if(!signed_int) continue;

			/* When we're looking at signed integers, we need to
			 * know what the last byte we looked at was set to.
			 * If we just ruled out a byte of all zeroes, we can
			 * rule out another byte of all zeroes. If we just
			 * ruled out a byte of all ones, we can rule out
			 * another byte of all ones. However, if we encounter
			 * something different, then we either needed that
			 * last byte, if the first bit of the current byte is
			 * different from the bits of the last byte, or we
			 * didn't need it, and we need to change the value of
			 * the `bytes` variable accordingly. */

			/* Setting the last byte, if we are in the first
			 * iteration of the outer for loop, then we can either
			 * set the byte to 0xFF or 0x00. If the current byte
			 * isn't one of those we must break out. */

			if(bytes == var -> type_length) {
				if(*ptr == 0xFF || !*ptr) {
					last_bytes[i] = *ptr;
					continue;
				}

				else {
					bits_set_in_byte = true;
					break;
				}
			}

			/* Then, if we need an extra byte to make sure that we
			 * faithfully encode the 2's complement going on, then
			 * we need to make sure that we mark that. */

			if(*ptr != last_bytes[i]) {
				if((*ptr & 0x80) ^ (last_bytes[i] & 0x80)) {
					increase_bytes = true;
				}

				bits_set_in_byte = true;
			}
		}

		/* If any bits were set, let's break out. */
		if(increase_bytes) bytes++;
		if(bits_set_in_byte) break;
	}

	/* This is a catch for if we roll past the end of the loop. */
	if(bytes > var -> type_length) bytes = var -> type_length;

	/* Free the memory we needed for our temporary array. */
	free(last_bytes);

	/* Let's go ahead and write the number of bytes per integer and the
	 * number of integers in the array to the file, if it is an array. */

	/* Write the length out for the integers. */
	int ret = fwrite(&bytes, sizeof(size_t), 1, file);
	if(ret == EOF) return LCF_FILEIO_ERR;

	/* Write the length out for the array, if applicable. */
	if(var -> is_array) {
		ret = fwrite(&(var -> array_length), sizeof(size_t), 1, file);
		if(ret == EOF) return LCF_FILEIO_ERR;
	}

	/* Now that we know how many bytes we actually need, we can write them
	 * out, one variable at a time for the entire array. */
	for(size_t i = 0; i < var -> array_length; i++) {
		/* Let's get the start of the integer. */
		uint8_t *start = base_ptr + (i * var -> type_length);

	#if (LC_ENDIANNESS == LC_BIG_ENDIAN)
		start += var -> type_length - bytes;

		/* We need to skip some number of bytes from the start of the 
		 * integer if it's a big endian system. */
	#endif

		/* Write out the bytes. */
		ret = fwrite(start, bytes, 1, file);
		if(ret == EOF) return LCF_FILEIO_ERR;
	}

	/* We're done; yay! */
	return LCF_OK;
}

static int write_other(FILE *file, flat_var_t *var) {
	/* Write the length out for the data type. */
	int ret = fwrite(&(var -> type_length), sizeof(size_t), 1, file);
	if(ret == EOF) return LCF_FILEIO_ERR;

	/* Write the length out for the array, if applicable. */
	if(var -> is_array) {
		ret = fwrite(&(var -> array_length), sizeof(size_t), 1, file);
		if(ret == EOF) return LCF_FILEIO_ERR;
	}

	/* Write the data out as a variable or a flat array. Notably, variables
	 * are stored as single-length arrays due to the way that the array
	 * length is set when flattening. */
	ret = fwrite(
		var -> data_ptr, var -> type_length,
		var -> array_length, file
	);
	
	if(ret == EOF) return LCF_FILEIO_ERR;
	else return LCF_OK;
}

static void bool_clear() {
	/* Clear out all the nodes and reset our counts. */
	for(current_node = starting_node; current_node;) {
		node_t *to_delete = current_node;
		current_node = current_node -> next;
		free(to_delete);
	}

	nodes_length = 0;
}

static int write_bool_init(FILE *file, flat_var_t *var) {
	/* Write the length out for the array, if applicable. */
	if(var -> is_array) {
		int ret = fwrite(&(var -> array_length), sizeof(size_t), 1, file);
		if(ret == EOF) return LCF_FILEIO_ERR;
	}

	/* Loop over the array and get the boolean values. */
	uint8_t *start_ptr = (uint8_t *) var -> data_ptr;

	for(size_t i = 0; i < var -> array_length; i++) {
		/* For each boolean, we know it's true if there's bits set
		 * in any of its bites. */
		bool bool_value = false;

		for(size_t j = 0; j < var -> type_length; j++) {
			if(*(start_ptr + j)) bool_value = true;
		}

		/* Go ahead and store the boolean, but if the current node is
		 * pointing to NULL, go ahead and create a starting node. */

		if(!current_node) {
			starting_node = malloc(sizeof(node_t));
			if(!starting_node) return LCF_MALLOC_ERR;

			current_node = starting_node;
		}

		else {
			current_node -> next = malloc(sizeof(node_t));
			if(!current_node -> next) return LCF_MALLOC_ERR;

			current_node = current_node -> next;
		}

		/* Go ahead and do the necessary bookkeeping. */
		current_node -> next = NULL;
		current_node -> value = bool_value;
		nodes_length++;

		/* Go to the next boolean. */
		start_ptr += var -> type_length;
	}

	return LCF_OK;
}

static int write_bool_fini(FILE *file) {
	/* We save the booleans in a bitset, so we need to calculate the
	 * number of bytes necessary, with one bit per boolean. */
	size_t length_in_bytes = ((nodes_length % 8)? 1: 0)
		+ (nodes_length / 8);

	uint8_t *bitfield = calloc(length_in_bytes, 1);
	if(!bitfield) return LCF_MALLOC_ERR;

	/* Iterate through the linked list and fill up the bitfield. We go from
	 * least to most significant bit in each byte before going to the next
	 * byte. */
	size_t byte_number = 0, bit_number = 0;
	for(node_t *i = starting_node; i; i = i -> next) {
		if(i -> value) bitfield[byte_number] |= 1 << bit_number++;

		if(bit_number >= 8) {
			bit_number = 0;
			byte_number++;
		}
	}

	/* Write out the bitfield. */
	int ret = fwrite(bitfield, 1, length_in_bytes, file);
	if(ret == EOF) return LCF_FILEIO_ERR;
	
	/* Reset the internal state and return out. */
	bool_clear();
	return LCF_OK;
}

int LCf_save(LCf_file_t *file_data) {
	/* Check if the file has been specified correctly. */
	if(file_data -> file && file_data -> filename)
		return LCF_FILE_CONFLICT;

	if(!file_data -> file && !file_data -> filename) return LCF_NO_FILE;

	/* We can copy over the handle given to us, else we need to open the
	 * file ourselves. */
	FILE *file = file_data -> file? file_data -> file: fopen(
		file_data -> filename, "wb"
	);

	if(!file) return LCF_FILEIO_ERR;

	/* Copy the file pointer over to the file data so that our caller can
	 * still close it if we quit out early. */
	if(file_data -> filename) file_data -> file = file;

	/* The metadata includes the number of bytes per size_t, whether we're
	 * using numeric or string IDs, and whether we're sstoring string
	 * descriptions of the program and the variables. */

	uint8_t metadata = LCF_BITS;
	metadata |= file_data -> use_names? LCF_USING_NAMES: LCF_USING_IDS;
	metadata |= file_data -> save_descriptions? LCF_USING_DESCRIPTIONS :
			LCF_IGNORING_DESCRIPTIONS;

	/* Write the libClame header data: magic number, metadata, version and
	 * subversion data. */
	int ret = fwrite(&magic, sizeof(uint32_t), 1, file);
	if(ret == EOF) return LCF_FILEIO_ERR;

	ret = fwrite(&metadata, sizeof(uint8_t), 1, file);
	if(ret == EOF) return LCF_FILEIO_ERR;

	ret = fwrite(&version, sizeof(uint8_t), 1, file);
	if(ret == EOF) return LCF_FILEIO_ERR;

	ret = fwrite(&subversion, sizeof(uint8_t), 1, file);
	if(ret == EOF) return LCF_FILEIO_ERR;

	/* Write the program header: program name or ID, description, version
	 * and subversion information. */

	/* Program name / ID. */
	if(file_data -> use_names) {
		if(!LCf_program_name) return LCF_NO_NAME;
		ret = fwrite(
			LCf_program_name, sizeof(char),
			strlen(LCf_program_name) + 1, file
		);
	}

	else {
		ret = fwrite(&LCf_program_id, sizeof(uint32_t), 1, file);
	}

	if(ret == EOF) return LCF_FILEIO_ERR;

	/* Optional program description */
	if(file_data -> save_descriptions) {
		if(!LCf_program_description) return LCF_NO_DESCRIPTION;
		ret = fwrite(
			LCf_program_description, sizeof(char), 
			strlen(LCf_program_description) + 1, file
		);

		if(ret == EOF) return LCF_FILEIO_ERR;
	}

	/* Program version and subversion. */
	ret = fwrite(&LCf_program_ver, sizeof(uint8_t), 1, file);
	if(ret == EOF) return LCF_FILEIO_ERR;

	ret = fwrite(&LCf_program_subver, sizeof(uint8_t), 1, file);
	if(ret == EOF) return LCF_FILEIO_ERR;

	/* Loop over the variables and arrays and write them to disk. */
	for(size_t i = 0; i < file_data -> vars_length; i++) {
		LCf_var_t *var = &file_data -> vars[i];

		/* We will come back for the booleans at the end of the file,
		 * so let's skip them for now. */
		if(var -> data_type == LCF_BOOL_VAR) continue;

		/* Write the name/ID, description, data type and type length
		 * for the variable, error out if we get invalid values. */

		/* Variable name / ID. */
		if(file_data -> use_names) {
			if(!var -> name) return LCF_NO_VAR_NAME;
			ret = fwrite(
				var -> name, sizeof(char),
				strlen(var -> name) + 1, file
			);
		}

		else {
			ret = fwrite(
				&(var -> id), sizeof(uint8_t), 1, file
			);
		}

		if(ret == EOF) return LCF_FILEIO_ERR;

		/* Variable description */
		if(file_data -> save_descriptions) {
			if(!var -> description) return LCF_NO_VAR_DESCRIPTION;
			ret = fwrite(
				var -> description, sizeof(char), 
				strlen(var -> description) + 1, file
			);

			if(ret == EOF) return LCF_FILEIO_ERR;
		}

		/* Let's destructure this variable in preparation for calling
		 * the appropriate helper function. If it passes us an error,
		 * we need to pass it on to our caller function. */

		flat_var_t flat_var = flatten(var);
		if(flat_var.ret != LCF_OK) return flat_var.ret;

		/* Variable data type. We need to get this based on the data
		 * type specified in the structure as well as the return value
		 * of the flatten function. */
		uint8_t data_type = LCF_VAR_TYPE(
			var -> data_type, flat_var.is_array
		);

		ret = fwrite(&data_type, sizeof(uint8_t), 1, file);
		if(ret == EOF) return LCF_FILEIO_ERR;

		switch(var -> data_type) {
		case LCF_STRING_VAR:
			ret = write_string(file, &flat_var);
			if(ret != LCF_OK) return ret;
			else break;

		/* We skip over bools so that we can collect them together at
		 * the end of the file. */

		case LCF_SIGNED_VAR:
			ret = write_integer(file, &flat_var, true);
			if(ret != LCF_OK) return ret;
			else break;

		case LCF_UNSIGNED_VAR:
			ret = write_integer(file, &flat_var, false);
			if(ret != LCF_OK) return ret;
			else break;

		/* We treat approx floats as just plain immutable values while
		 * writing them out. */

		case LCF_APPROX_FLOAT:
		case LCF_OTHER_VAR:
			ret = write_other(file, &flat_var);
			if(ret != LCF_OK) return ret;
			else break;

		default:
			/* Error out because of the unknown type. */
			return LCF_BAD_DATA_SPEC;

		}
	}

	/* We need to now go back for the booleans. First, though, let's make
	 * sure that our cache is clear. */
	bool_clear();

	for(size_t i = 0; i < file_data -> vars_length; i++) {
		LCf_var_t *var = &file_data -> vars[i];

		/* Skip all the ones that aren't booleans. */
		if(var -> data_type != LCF_BOOL_VAR) continue;

		/* Write the name/ID, description, data type and type length
		 * for the variable, error out if we get invalid values. */

		/* Variable name / ID. */
		if(file_data -> use_names) {
			if(!var -> name) return LCF_NO_VAR_NAME;
			ret = fwrite(
				var -> name, sizeof(char),
				strlen(var -> name) + 1, file
			);
		}

		else {
			ret = fwrite(
				&(var -> id), sizeof(uint8_t), 1, file
			);
		}

		if(ret == EOF) return LCF_FILEIO_ERR;

		/* Variable description */
		if(file_data -> save_descriptions) {
			if(!var -> description) return LCF_NO_VAR_DESCRIPTION;
			ret = fwrite(
				var -> description, sizeof(char), 
				strlen(var -> description) + 1, file
			);

			if(ret == EOF) return LCF_FILEIO_ERR;
		}

		/* Let's destructure this variable in preparation for calling
		 * the appropriate helper function. If it passes us an error,
		 * we need to pass it on to our caller function. */

		flat_var_t flat_var = flatten(var);
		if(flat_var.ret != LCF_OK) return flat_var.ret;

		/* Variable data type. We need to get this based on the data
		 * type specified in the structure as well as the return value
		 * of the flatten function. */
		uint8_t data_type = LCF_VAR_TYPE(
			var -> data_type, flat_var.is_array
		);

		ret = fwrite(&data_type, sizeof(uint8_t), 1, file);
		if(ret == EOF) return LCF_FILEIO_ERR;

		/* Let's go ahead and write the initial data for the booleans
		 * as we loop over them. */
		ret = write_bool_init(file, &flat_var);
		if(ret != LCF_OK) return ret;
	}

	/* Before we write the final data for the booleans, we need to write a
	 * single NULL byte, which would be read by LCf_read() as either a 0
	 * ID variable or a variable with the name "" and thus serve to trigger
	 * the end of processing for the booleans. */
	uint8_t null_byte = 0;
	ret = fwrite(&null_byte, sizeof(uint8_t), 1, file);
	if(ret == EOF) return LCF_FILEIO_ERR;

	/* We need to now write the final data for the booleans. */
	ret = write_bool_fini(file);
	if(ret != LCF_OK) return ret;

	/* Close the file, if we opened it, and return out. */
	if(file_data -> filename) {
		file_data -> file = NULL;

		ret = fclose(file);
		if(ret == EOF) return LCF_FILEIO_ERR;
	}
	
	return LCF_OK;
}

/* When reading data, it's useful to get a dynamically allocated string out of
 * the file in order to compare it to stuff. Since we're going to be returning
 * an integer on error as well as a pointer to the string, here's a structure
 * for both of those. */
typedef struct {
	int ret;
	char *value;

} string_t;

static string_t read_string_file(FILE *file);

/* The same for a size_t; since the file may have any number of bits for its
 * size_t and we'll need to both convert it to our native arch's understanding
 * of a size type to process the data, as well as make sure that we actually
 * can fit the memory it represents within our system's memory. */

typedef struct {
	int ret;
	size_t value;

} size_file_t;

static size_file_t read_size_file(LCf_file_t *file_data);

/* We need a helper function to find the variable based on the name or the ID.
 * We'll have it return an appropriate error if it encounters an issue. */
typedef struct {
	int ret;
	LCf_var_t *ptr;

} var_t;

static var_t find_variable(LCf_file_t *file_data, char *name, uint8_t id);

/* We need a helper function to read a variable, since we don't want everything
 * indented inside a while loop. */
static int read_variable(LCf_file_t *file_data, FILE *file);

/* We'll want helper functions for each subtype of data, similar to when saving
 * data to disk. */

static int read_string(LCf_file_t *file_data, LCf_var_t *var);
static int read_int(LCf_file_t *file_data, LCf_var_t *var, bool signed_int);
static int read_float(LCf_file_t *file_data, LCf_var_t *var);
static int read_other(LCf_file_t *file_data, LCf_var_t *var);

/* We'll use the same cache structures and practices for reading as while
 * writing to disk. */

static int read_bool_init(LCf_file_t *file_data, LCf_var_t *var);
static int read_bool_fini(LCf_file_t *file_data);

static string_t read_string_file(FILE *file) {
	string_t string = {LCF_OK, NULL}; // The structure we're returning.

	/* Let's get the current position in the file and forward through until
	 * we encounter the null byte at the end of the string. */
	long start = ftell(file);
	if(start == -1) {
		string.ret = LCF_FILEIO_ERR;
		return string;
	}

	while(true) {
		int character = fgetc(file);
		if(character == EOF) {
			string.ret = LCF_FILEIO_ERR;
			return string;
		}

		if(character == 0) break;
	}

	long end = ftell(file);
	if(end == -1) {
		string.ret = LCF_FILEIO_ERR;
		return string;
	}

	/* We have the length, but we need one extra byte for the NULL. */
	size_t length = (unsigned) end - (unsigned) start + 1;

	/* Get the memory we need for the string. */
	string.value = malloc(length);
	if(!string.value) {
		string.ret = LCF_MALLOC_ERR;
		return string;
	}

	/* Go back to the start of the string and copy it out. */
	int ret = fseek(file, start, SEEK_SET);
	if(ret) {
		/* Make sure that we don't forgot to free memory lest we cause
		 * a memory leak. */
		free(string.value);
		string.value = NULL;

		string.ret = LCF_FILEIO_ERR;
		return string;
	}

	ret = fread(string.value, sizeof(char), length, file);
	if(ret == EOF) {
		/* Make sure that we don't forgot to free memory lest we cause
		 * a memory leak. */
		free(string.value);
		string.value = NULL;

		string.ret = LCF_FILEIO_ERR;
		return string;
	}

	/* We can go ahead and return now. */
	return string;
}

/* In case we're reading the size variables as stored by a system with the same
 * number of bits and the same endianness as us, we can significantly speed up
 * computation by having a dedicated read_size_native() function. */

static size_file_t read_size_native(LCf_file_t *file_data) {
	/* Our return value. */
	size_file_t size = {LCF_OK, 0};

	/* Read from the file. */
	int ret = fread(&size.value, sizeof(size_t), 1, file_data -> file);
	if(ret == EOF) {
		size.ret = LCF_FILEIO_ERR;
		return size;
	}

	/* This is the safe return path. */
	return size;
}

static size_file_t read_size_file(LCf_file_t *file_data) {
	/* If we have the same number of bytes in a size_t and the same
	 * endianness as the file we read from, then call our faster native
	 * function. */
	if(file_data -> same_bits && file_data -> same_endianness) {
		return read_size_native(file_data);
	}

	/* Our return value. */
	size_file_t size = {LCF_OK, 0};

	/* Let us allocate the amount of memory needed and read out the bytes
	 * from the file. We'll use stack allocation here although it's not
	 * necessarily the best for performance sake, simply because it'll save
	 * us the need to possibly deallocate heap memory later. */
	uint8_t bytes[file_data -> bits];

	/* Read from the file. */
	int ret = fread(bytes, 1, file_data -> bits, file_data -> file);
	if(ret == EOF) {
		size.ret = LCF_FILEIO_ERR;
		return size;
	}

	/* Check that no bits are set higher than the ones that we use. This
	 * means different things depending on if the source is little or big
	 * endian. We can skip doing this if the size_t on our system is larger
	 * than or equal to the one used by the file. */
	if(file_data -> bits > LCF_BITS) {
		uint8_t difference = file_data -> bits - LCF_BITS;
		bool problems = false;

		switch(file_data -> endianness) {
		case LC_BIG_ENDIAN:
			for(uint8_t i = 0; i < difference; i++) {
				problems = problems || bytes[i];
			}

			break;

		case LC_LITTLE_ENDIAN:
			for(uint8_t i = LCF_BITS; i < file_data -> bits; i++) {
				problems = problems || bytes[i];
			}

			break;
		}

		/* Error out accordingly. */
		if(problems) {
			size.ret = LCF_BAD_ARCH;
			return size;
		}
	}

	/* We can copy out the bytes that matter safely, adjusting of course
	 * based on the architecture of the system that wrote the data. This
	 * also looks different based on whether we need to skip bytes. */

	switch(file_data -> endianness) {
	case LC_BIG_ENDIAN:
		if(file_data -> bits <= LCF_BITS) {
			/* Most significant bit first. */
			for(uint8_t i = 0; i < file_data -> bits; i++) {
				size.value <<= 8;
				size.value |= (size_t) bytes[i];
			}
		}

		else {
			uint8_t skip = file_data -> bits - LCF_BITS;

			/* Start at the skip offset. */
			for(uint8_t i = skip; i < file_data -> bits; i++) {
				size.value <<= 8;
				size.value |= (size_t) bytes[i];
			}
		}

		break;

	case LC_LITTLE_ENDIAN:
		if(file_data -> bits <= LCF_BITS) {
			/* Most significant bit last. Expect wraparound. */
			for(uint8_t i = file_data -> bits - 1; i < 255; i++) {
				size.value <<= 8;
				size.value |= (size_t) bytes[i];
			}
		}

		else {
			/* Start at the correct offset. */
			for(uint8_t i = LCF_BITS - 1; i < 255; i++) {
				size.value <<= 8;
				size.value |= (size_t) bytes[i];
			}
		}
	}

	/* Return our value successfully. */
	return size;
}

static var_t find_variable(LCf_file_t *file_data, char *name, uint8_t id) {
	var_t var = {LCF_OK, NULL}; // The structure we're returning.

	/* Loop over all the variables we have until we find a match. */
	for(size_t i = 0; i < file_data -> vars_length; i++) {
		/* If we are using names to find variables, look for a matching
		 * strings, but check that we don't have a NULL string. */
		if(file_data -> use_names && name) {
			if(!file_data -> vars[i].name) {
				var.ret = LCF_NO_VAR_NAME;
				return var;
			}

			if(!strcmp(name, file_data -> vars[i].name)) {
				var.ptr = &(file_data -> vars[i]);
				return var;
			}
		}

		/* Otherwise we can always check with the ID. */
		else if(id == file_data -> vars[i].id) {
			var.ptr = &(file_data -> vars[i]);
			return var;
		}
	}

	/* If we couldn't find anything, set an appropriate error. */
	var.ret = LCF_BAD_VARIABLE;
	return var;
}

static int read_variable(LCf_file_t *file_data, FILE *file) {
	/* Let's start reading the variable data. The first thing we'll do is
	 * to try to find the variable we're reading to. */
	var_t var = {0, NULL};

	if(file_data -> use_names) {
		/* Read the first string out of the file; the variable name. */
		string_t string = read_string_file(file);
		if(string.ret != LCF_OK) return string.ret;

		/* If the string is "", then it tells us to stop processing the
		 * booleans and wrap up. */
		if(!strlen(string.value)) return read_bool_fini(file_data);

		/* Let's use the string to find a matching variable. */
		var = find_variable(file_data, string.value, 0);

		/* Let's free the string, lest we get a memory leak. */
		free(string.value);
	}

	else {
		/* Read the variable id out of the file. */
		uint8_t id;

		int ret = fread(&id, sizeof(uint8_t), 1, file);
		if(ret == EOF) return LCF_FILEIO_ERR;

		/* If the ID is 0, then it tells us to stop processing the
		 * booleans and wrap up. */
		if(!id) return read_bool_fini(file_data);

		/* Let's use the string to find a matching variable. */
		var = find_variable(file_data, NULL, id);
	}

	/* Return the error out if we didn't find a variable. */
	if(var.ret != LCF_OK) return var.ret;

	/* If the variable has a description, we can just ignore the string.
	 * Just keep reading until we encounter a NULL character. */
	while(file_data -> save_descriptions) {
		int character = fgetc(file);
		if(character == EOF) return LCF_FILEIO_ERR;
		else if(character == 0) break;
	}

	/* To check the variable data type, we need to figure out if the data
	 * stored here should be a variable or an array. */
	bool is_array = false;

	switch(var.ptr -> allocation_type) {
	case LCF_DYNAMIC_VAR: // At this point we're not checking for whether
	case LCF_STATIC_VAR:  // the data is stored dynamically or statically.
		is_array = true;
		break;

	case LCF_DYNAMIC_ARR:
	case LCF_STATIC_ARR:
		is_array = false;
		break;

	default:
		/* If the specified allocation type isn't one of the above ones
		 * then we need to return an apporpriate error. */
		return LCF_BAD_ALLOC_SPEC;
	}

	/* We can get the value that should be stored in the file. */
	uint8_t var_data_type = LCF_VAR_TYPE(var.ptr -> data_type, is_array);

	/* Now read the value that's actually stored in the file. */
	uint8_t data_type = 0;
	int ret = fwrite(&data_type, sizeof(uint8_t), 1, file);
	if(ret == EOF) return LCF_FILEIO_ERR;

	/* If they don't match, issue an appropriate error. */
	if(data_type != var_data_type) return LCF_BAD_DATA_TYPE;

	/* Pass the variable off to the appropriate reading function. */
	switch(var.ptr -> data_type) {
	case LCF_STRING_VAR:
		return read_string(file_data, var.ptr);

	case LCF_BOOL_VAR:
		return read_bool_init(file_data, var.ptr);

	case LCF_SIGNED_VAR:
		return read_int(file_data, var.ptr, true);

	case LCF_UNSIGNED_VAR:
		return read_int(file_data, var.ptr, false);

	case LCF_APPROX_FLOAT:
		return read_float(file_data, var.ptr);

	case LCF_OTHER_VAR:
		return read_other(file_data, var.ptr);

	default:
		/* Error out because of the unknown type. */
		return LCF_BAD_DATA_SPEC;

	}
}

static int read_string(LCf_file_t *file_data, LCf_var_t *var) {
	(void) file_data;
	(void) var;
	return LCF_OK;
}

static int read_int(LCf_file_t *file_data, LCf_var_t *var, bool signed_int) {
	(void) file_data;
	(void) var;
	(void) signed_int;
	return LCF_OK;
}

static int read_float(LCf_file_t *file_data, LCf_var_t *var) {
	(void) file_data;
	(void) var;
	return LCF_OK;
}

static int read_other(LCf_file_t *file_data, LCf_var_t *var) {
	(void) file_data;
	(void) var;
	return LCF_OK;
}

static int read_bool_init(LCf_file_t *file_data, LCf_var_t *var) {
	(void) file_data;
	(void) var;
	return LCF_OK;
}

static int read_bool_fini(LCf_file_t *file_data) {
	(void) file_data;
	return LCF_OK;
}

int LCf_read(LCf_file_t *file_data) {
	/* Check if the file has been specified correctly. */
	if(file_data -> file && file_data -> filename) return LCF_FILE_CONFLICT;
	if(!file_data -> file && !file_data -> filename) return LCF_NO_FILE;

	/* We can copy over the handle given to us, else we need to open the
	 * file ourselves. */
	FILE *file = file_data -> file? file_data -> file: fopen(
		file_data -> filename, "rb"
	);

	if(!file) return LCF_FILEIO_ERR;

	/* Copy the file pointer over to the file data so that our caller can
	 * still close it if we quit out early. */
	if(file_data -> filename) file_data -> file = file;

	/* Let's get the first chunk of data and get to processing it: the
	 * magic number of the file, the metadata, the version and the
	 * subversion of libClame that generated it. */
	int ret = fread(&(file_data -> magic), sizeof(uint32_t), 1, file);
	if(ret == EOF) return LCF_FILEIO_ERR;

	ret = fread(&(file_data -> metadata), sizeof(uint8_t), 1, file);
	if(ret == EOF) return LCF_FILEIO_ERR;

	ret = fread(&(file_data -> version), sizeof(uint8_t), 1, file);
	if(ret == EOF) return LCF_FILEIO_ERR;

	ret = fread(&(file_data -> subversion), sizeof(uint8_t), 1, file);
	if(ret == EOF) return LCF_FILEIO_ERR;

	/* The magic number must match exactly. Else, check for if it's
	 * reversed, in which case the endianness of the machine that saved the
	 * file is different from ours and we need to invert all variables
	 * longer than a byte. */
	file_data -> same_endianness = true;
	file_data -> endianness = LC_ENDIANNESS;

	if(file_data -> magic != LCF_MAGIC) {
		file_data -> same_endianness = false;

		/* Flip the endianness on the variable. */
		switch(LC_ENDIANNESS) {
		case LC_BIG_ENDIAN:
			file_data -> endianness = LC_LITTLE_ENDIAN;
			break;

		case LC_LITTLE_ENDIAN:
			file_data -> endianness = LC_BIG_ENDIAN;
		}

		if(file_data -> magic != LCF_MAGIC_REVERSE) {
			return LCF_BAD_FORMAT;
		}
	}

	/* The version number must match exactly. */
	if(file_data -> version != LCF_VERSION) return LCF_BAD_LCF_VER;

	/* The subversion number must not be greater. */
	if(file_data -> subversion > LCF_SUBVERSION) return LCF_BAD_LCF_VER;

	/* Let's deconstruct the metadata. */
	if(file_data -> metadata & LCF_USING_NAMES) {
		file_data -> use_names = true;
	}

	if(file_data -> metadata & LCF_USING_DESCRIPTIONS) {
		file_data -> save_descriptions = true;
	}

	/* If the file is the same number of bits as us, let's write that
	 * down else we need to be cautious about whether we're copying out
	 * more data than we can store later on down the line. */
	file_data -> bits = file_data -> metadata & LCF_BITS_MASK;
	file_data -> same_bits = file_data -> bits == LCF_BITS;

	/* Let's start reading the program data. */
	if(file_data -> use_names) {
		/* Read the first string out of the file; the program name. */
		string_t string = read_string_file(file);
		if(string.ret != LCF_OK) return string.ret;

		/* Go ahead and save the string for debugging purposes. */
		file_data -> program_name = string.value;

		/* If the value doesn't match the program name we've been given
		 * then we need to err out appropriately. */
		if(strcmp(string.value, LCf_program_name)) {
			return LCF_BAD_PROGRAM;
		}
	}

	else {
		/* Read the program id out of the file. */
		ret = fread(
			&(file_data -> program_id),
			sizeof(uint32_t), 1, file
		);

		if(ret == EOF) return LCF_FILEIO_ERR;

		/* If the value doesn't match, error out appropriately. */
		if(file_data -> program_id != LCf_program_id) {
			return LCF_BAD_PROGRAM;
		}
	}

	/* If the program has a description, we can copy that to the file
	 * descriptor structure but we don't need to do anything with it. */
	if(file_data -> save_descriptions) {
		/* Read the description out of the file. */
		string_t string = read_string_file(file);
		if(string.ret != LCF_OK) return string.ret;

		/* Go ahead and save the string for debugging purposes. */
		file_data -> program_description = string.value;
	}

	/* We need to get the program version and subversion. */
	ret = fread(&(file_data -> program_ver), sizeof(uint8_t), 1, file);
	if(ret == EOF) return LCF_FILEIO_ERR;

	ret = fread(&(file_data -> program_subver), sizeof(uint8_t), 1, file);
	if(ret == EOF) return LCF_FILEIO_ERR;

	/* The version number must match exactly. */
	if(file_data -> program_ver != LCf_program_ver) {
		return LCF_BAD_LCF_VER;
	}

	/* The subversion number must not be greater. */
	if(file_data -> program_subver > LCf_program_subver) {
		return LCF_BAD_LCF_VER;
	}

	/* Until we reach the end of the file, keep reading variables so long
	 * as we're not erroring out. */
	while(!feof(file)) {
		ret = read_variable(file_data, file);
		if(ret != LCF_OK) return ret;
	}

	/* Close the file, if we opened it, and return out. */
	if(file_data -> filename) {
		file_data -> file = NULL;

		ret = fclose(file);
		if(ret == EOF) return LCF_FILEIO_ERR;
	}

	return 0;
}

void LCf_clean() {
	/* Call the code for cleanup of the boolean-related structures. */
	bool_clear();
}

const char *LCf_error_string(int ret) {
	/* Just return the name of the error. */
	switch(ret) {
		case LCF_OK: return "LCF_OK";
		case LCF_FILEIO_ERR: return "LCF_FILEIO_ERR";
		case LCF_MALLOC_ERR: return "LCF_MALLOC_ERR";
		case LCF_BAD_ARCH: return "LCF_BAD_ARCH";

		case LCF_BAD_FORMAT: return "LCF_BAD_FORMAT";
		case LCF_BAD_LCF_VER: return "LCF_BAD_LCF_VER";
		case LCF_BAD_PROG_VER: return "LCF_BAD_PROG_VER";
		case LCF_BAD_PROGRAM: return "LCF_BAD_PROGRAM";

		case LCF_BAD_VARIABLE: return "LCF_BAD_VARIABLE";
		case LCF_BAD_DATA_TYPE: return "LCF_BAD_DATA_TYPE";
		case LCF_BAD_DATA_LEN: return "LCF_BAD_DATA_LEN";
		case LCF_BAD_ARR_LEN: return "LCF_BAD_ARR_LEN";

		case LCF_NO_VAR_NAME: return "LCF_NO_VAR_NAME";
		case LCF_NO_VAR_DESCRIPTION: return "LCF_NO_VAR_DESCRIPTION";
		case LCF_BAD_DATA_SPEC: return "LCF_BAD_DATA_SPEC";
		case LCF_BAD_ALLOC_SPEC: return "LCF_BAD_ALLOC_SPEC";
		case LCF_NULL_DATA_PTR: return "LCF_NULL_DATA_PTR";
		case LCF_NULL_ARR_LEN_PTR: return "LCF_NULL_ARR_LEN_PTR";

		case LCF_NO_FILE: return "LCF_NO_FILE";
		case LCF_FILE_CONFLICT: return "LCF_FILE_CONFLICT";
		case LCF_NO_VARS: return "LCF_NO_VARS";

		case LCF_NO_NAME: return "LCF_NO_NAME";
		case LCF_NO_DESCRIPTION: return "LCF_NO_DESCRIPTION";
	}

	/* If the error code were something else, then return another error. */
	return LCF_UNKNOWN_ERR;
}