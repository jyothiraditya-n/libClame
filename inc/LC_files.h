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
#ifndef LC_FILES_H
#define LC_FILES_H 1

#include <stdbool.h>
#include <stddef.h>
#include <inttypes.h>

#include <LC.h>

/* Version Information */
#define LCF_VERSION 1 /* Incremented when backwards compatibility broken. */
#define LCF_SUBVERSION 0 /* Incremented when new features added. */

/* Each data structure that we process has properties specified by an LCf_var_t
 * struct. A run down of what all of the properties do and mean is written in
 * the comments within the structure definition itself. */

typedef struct {
	/* Basic data about the variable. */
	const char *name; // A unique name for the variable in the program.
	uint8_t id; // A unique numeric ID to use instead of the name.

	/* Please note that the ID 0 and the name "" are both reserved for
	 * internal use. */

	/* Otherwise, you only need to specify the name or ID, you don't need
	 * both, and the option to enable one or the other for storing and
	 * reading from files is controlled on a file-by-file basis in the
	 * LCf_file_t structure. */

	const char *description; // A descriptive string for the variable.
	size_t type_length; // Set to the length of the variable data type.

	/* Variable typing. */
	int data_type; // The type of data we're storing, one of the following:

	#define LCF_STRING_VAR 1 // Null-terminated character arrays.
	#define LCF_BOOL_VAR 2 // True-false values.
	#define LCF_SIGNED_VAR 3 // Two's complement signing.
	#define LCF_UNSIGNED_VAR 4 // Unsigned Integers.
	#define LCF_APPROX_FLOAT 5 // Floating point numbers you wanna fudge.
	#define LCF_OTHER_VAR 6 // Immutable data.

	/* We treat strings differently, since if they're NULL-terminated we
	 * can avoid storing their lengths as variables in the file, and know
	 * their extents based on where the NULL byte is. Similarly, it also
	 * means you don't need to supply us a length for the string. */

	/* We treat boolean values differently, since we collect them at the
	 * end of the file and compress arrays to reduce the amount of space
	 * they take up (1 bit per bool). Nevertheless, we need you to still
	 * give us length information since the boolean type used by each
	 * program can take up a different amount of space in RAM. */

	/* We need to know if it's a signed or unsigned integer since we can
	 * actually save on the number of bytes needed to store them and
	 * expand them back out when they're read into program memory. This
	 * also allows you to share data files between versions of your program
	 * running on 32-bit and 64-bit machines where the variable sizes may
	 * differ, as long as the values are within the range of what can be
	 * read into the reading computer's memory. */

	/* We can also lossily convert 64-bit floats to 32-bit floats when
	 * reading them from a file into memory, but we need to know whether to
	 * do this or simply issue an error. */

	/* ALl other values are treated as immutable and we won't change them
	 * when storing or reading them. */

	/* Variable storage. */
	int allocation_type; // What the allocation type is.
	void *data_ptr; // Pointer to the data. (Alternatively void **data_ptr)

	#define LCF_DYNAMIC_VAR 1 // A single variable on the heap.
	#define LCF_STATIC_VAR 2 // A single variable in the data segment.
	#define LCF_DYNAMIC_ARR 3 // An array on the heap.
	#define LCF_STATIC_ARR 4 // An array in the data segment.

	/* If the data is statically allocated, store a pointer to the start of
	 * where it's stored, else store a pointer to a pointer to the start of
	 * the dynamically allocated memory and set the appropriate type. */

	/* Note, if the data is strings, then we assume a 'static allocation'
	 * to mean a static array of pointers where the strings that these
	 * pointers are pointing to are stored dynamically on the heap. If not,
	 * attempting to read strings into this memory will cause a crash when
	 * LCf_read() tries to free() these pointers, as it will do all dynamic
	 * memory pointers to avoid data leaks. */

	/* We need the length of the array if it's statically allocated, else
	 * a pointer to the length in case it's dynamically allocated. Set both
	 * of them to zero and NULL respectively if you've either got a null-
	 * terminated string whose length is stored implicitly in the data or
	 * if you don't have an array at all but just a single variable. */

	size_t static_arr_length; // The length for the static array.
	size_t *dynamic_arr_length; // The length for the dynamic array.

	/* Minimum and maximum lengths, to help verify data. */
	size_t min_arr_length; // Set to 0 to disable checking.
	size_t max_arr_length; // Set to SIZE_MAX to disable checking.

	/* The following are for use within the file and are not part of the
	 * API for communicating with the library. */

	#define LCF_VAR_TYPE(data_type, is_array) \
		((is_array? 0xF0: 0x00) | data_type)

} LCf_var_t;

/* It's worth noting that the setup we have here doesn't lend itself granularly
 * to dealing with data structures. Ideally, to store them, you'd want to
 * destructure them into their individual variables in order to take advantage
 * of our resizing abilities. However, when it comes to dealing with a large
 * number of structures, it's kind of on the program to try to turn them into
 * a bunch of regular arrays of variables. */

/* Alternatively, you could also forgo the sizing abilities and just store the
 * data structures as immutable data types. It's up to you. */

/* Constants that are stored in the file. */

#define LCF_MAGIC 0x11bc1a2e // "libClame" -> "11bc1a2e"
#define LCF_MAGIC_REVERSE 0x2e1abc11 // "libClame" but written little-endian.

#define LCF_BITS sizeof(size_t)

#define LCF_BITS_MASK 0x0F

/* The following are used to store extra information about the data file such
 * as whether the identifying elements are strings or numeric IDs and whether
 * descriptive strings are included. */

#define LCF_USING_NAMES 0x80
#define LCF_USING_IDS 0x00

#define LCF_USING_DESCRIPTIONS 0x40
#define LCF_IGNORING_DESCRIPTIONS 0x00

/* When we save a file, we want either a string or a 32-bit magic number to
 * serve as a verification that we're reading a file for the correct program.
 * A description of what the program is as a string can also be useful. */

extern const char *LCf_program_name; // Program name (unique)
extern uint32_t LCf_program_id; // Or a program ID (unique)

extern const char *LCf_program_description; // Program description.
extern uint8_t LCf_program_ver; // Invalid if file ver != this variable.
extern uint8_t LCf_program_subver; // Invalid if file subver > this variable.

/* There's a lot of data associated with a file, so we need another sturcture
 * which can keep track of that information going to our functions and being
 * returned by them. */

typedef struct {
	/* File properties. */
	const char *filename; // The filename we should open.
	FILE *file; // A file object that's already open.

	/* Notably, when returning with an error, if the file has not yet been
	 * closed, the pointer to the file object is copied into the `file`
	 * variable so that you can still close it yourself. */

	/* The following variables are set when reading the data and used
	 * when writing the data. */

	bool use_names; // Use string-based names instead of numeric IDs.
	bool save_descriptions; // Write descriptions to file.

	/* Set file to NULL if you're gonna pass a filename to the library,
	 * else set the filename to NULL to pass in a FILE object. */

	/* Notably, if you wanna pass a file like stdin or something to us,
	 * you don't have to specify filename, and if you're not giving us an
	 * open binary file handle, then set filename and make file NULL. */

	LCf_var_t *vars; // The data that we want to process within this file.
	size_t vars_length; // The length of that specifying array.

	/* The following variables are copied out of the file structure and
	 * are set to whatever the file reports, in case it's useful error
	 * reporting information. Note that there's no guarantees all these
	 * fields are going to be set, or that they'll be normalised to NULL
	 * values if you don't set them yourself before passing the structure
	 * to a library function. */

	/* Notably, they are also only set when reading a file, and the strings
	 * are dynamically allocated so you need to free them yourself. */

	uint32_t magic; // The magic number for the file.
	uint8_t metadata; // The file settings.
	uint8_t version; // The libClame version that generated it.
	uint8_t subversion; // The libClame subversion.

	char *program_name;  // The name of the program that generated it.
	uint32_t program_id; // OR the ID of that program.
	char *program_description; // The description, if it saved one.

	uint8_t program_ver; // The version of the program.
	uint8_t program_subver; // The subversion of that program.

	int endianness; // The endianness of the file. 
	uint8_t bits; // The number of bytes for a size_t variable.

	bool same_endianness; // We have the same endianness as the file.
	bool same_bits; // We have the same number of bits in a size_t.

} LCf_file_t;

/* Once you've got the variable structures set up, you can go ahead and pass it
 * to the library by calling the appropriate functions. An example of doing so
 * is as such:
 * LCf_vars_t vars[] = { {...}, {...}, ... };
 * LCf_file_t file = { ..., vars, LC_ARRAY_LENGTH(vars), ... };
 * int ret = LCf_read(file); */

extern int LCf_read(LCf_file_t *file_data);
extern int LCf_save(LCf_file_t *file_data);

/* The following are the menaings of the returned values from the functions. */

#define LCF_OK 0  // No errors occurred.
#define LCF_FILEIO_ERR 1 // We weren't able to read or write to the file.
#define LCF_MALLOC_ERR 2 // We weren't able to allocate memory.
#define LCF_BAD_ARCH 3 // The file has vars larger than our system supports.

#define LCF_BAD_FORMAT 4 // The header's magic number was incorrect.
#define LCF_BAD_LCF_VER 5 // The LCF version on the file is incompatible.
#define LCF_BAD_PROG_VER 6 // The file has an incompatible program version.
#define LCF_BAD_PROGRAM 7 // The program ID or name doesn't match.

#define LCF_BAD_VARIABLE 8 // The file specifies a variable that doesn't exist.
#define LCF_BAD_DATA_TYPE 9 // The data type in the file did not match.
#define LCF_BAD_DATA_LEN 10 // The file specifies a data type of the wrong size.
#define LCF_BAD_ARR_LEN 11 // The array in the file was of an invalid length.

#define LCF_NO_VAR_NAME 12 // A variable name was unspecified.
#define LCF_NO_VAR_DESCRIPTION 13 // A variable description was unspecified.
#define LCF_BAD_DATA_SPEC 14 // The data type was not specified as a valid one.
#define LCF_BAD_ALLOC_SPEC 15 // The allocation type was invalid.
#define LCF_NULL_DATA_PTR 16 // A NULL pointer was given to the variable.
#define LCF_NULL_ARR_LEN_PTR 17 // A NULL pointer was given to the array length.

#define LCF_NO_FILE 18 // Both the filename and file were given as NULL.
#define LCF_FILE_CONFLICT 19 // Both the filename and file were specified.
#define LCF_NO_VARS 20 // The pointer to the LCf_var_t array was given as NULL.

#define LCF_NO_NAME 21 // The program name was unspecified.
#define LCF_NO_DESCRIPTION 22 // The program description was unspecified.

/* The libClame file handlers maintain some internal state that may be left
 * uncleaned in the case of a sudden return with an error code. In order to
 * force the internal state to be cleaned, you can run the following
 * function. */

extern void LCf_clean();

/* This function converts the error code into a string if you want to print it
 * out to the console. */
const char *LCf_error_string(int ret);

#define LCF_UNKNOWN_ERR "LCF_UNKNOWN_ERR"

/* End Header Guard */
#endif