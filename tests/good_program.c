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

#include <stdio.h>

#define LC_REQ_VER 1
#define LC_REQ_SUBVER 1

#include <libClame.h>
#include <LC_macros.h>

/* Function that prints when it's called. */
int custom_callback(LC_flag_t *flag) {
	(void) flag;
	printf("custom_callback(); ");
	return LC_OK;
}

/* We're going to stress-test every data type we can think of. */
bool boolean_var;
char *string_var, **string_arr; size_t string_arr_len;

int int_var, *int_arr; size_t int_arr_len;
double double_var, *double_arr; size_t double_arr_len;
size_t size_var, *size_arr; size_t size_arr_len;
int32_t int32_var, *int32_arr; size_t int32_arr_len;

uint8_t oct_var, *oct_arr; size_t oct_arr_len;
uint8_t hex_var, *hex_arr; size_t hex_arr_len;

typedef char fname_t[9]; // 8-char filenames like DOS.
fname_t fname_var, *fname_arr; size_t fname_arr_len;

int *limited_arr; size_t limited_arr_len; // Arr of only two values.

LC_flag_t args[] = {
	LC_MAKE_CALL("callback", 'c', custom_callback),
	LC_MAKE_BOOL("boolean_var", 'b', boolean_var, true),

	LC_MAKE_BOOL_F(
		"boolean_callback", '!', boolean_var, true, custom_callback
	),

	LC_MAKE_STRING("string_var", 's', string_var),
	LC_MAKE_STRING_ARR("string_arr", 'S', string_arr, string_arr_len),

	LC_MAKE_VAR("int_var", 'i', int_var, "%d"),
	LC_MAKE_ARR("int_arr", 'I', int_arr, "%d", int_arr_len),

	LC_MAKE_VAR("double_var", 'd', double_var, "%lf"),
	LC_MAKE_ARR("double_arr", 'D', double_arr, "%lf", double_arr_len),

	LC_MAKE_VAR("size_var", 'z', size_var, "%zu"),
	LC_MAKE_ARR("size_arr", 'Z', size_arr, "%zu", size_arr_len),

	LC_MAKE_VAR("int32_var", 'l', int32_var, "%" SCNd32), // l for long
	LC_MAKE_ARR("int32_arr", 'L', int32_arr, "%" SCNd32, int32_arr_len),

	LC_MAKE_VAR("oct_var", 'o', oct_var, "%" SCNo8),
	LC_MAKE_ARR("oct_arr", 'O', oct_arr, "%" SCNo8, oct_arr_len),

	LC_MAKE_VAR("hex_var", 'x', hex_var, "%" SCNx8),
	LC_MAKE_ARR("hex_arr", 'X', hex_arr, "%" SCNx8, hex_arr_len),

	LC_MAKE_VAR("filename_var", 'f', fname_var, "%8s"),
	LC_MAKE_ARR("filename_arr", 'F', fname_arr, "%8s", fname_arr_len),

	LC_MAKE_ARR_BOUNDED("limited_arr", '2', limited_arr, "%d",
		limited_arr_len, 2, 2),

	LC_MAKE_ARR_BOUNDED_F("limited_callback", '@', limited_arr, "%d",
		limited_arr_len, 2, 2, custom_callback)
};

int main(int argc, char **argv) {
	/* Set the arguments and call LC_read(). */
	LC_flags_length = LC_ARRAY_LENGTH(args);
	LC_flags = args;

	int ret = LC_read(argc, argv);

	switch(ret) {
	case LC_OK: break;

	case LC_MALLOC_ERR:
		printf("%s: error allocating memory.", LC_prog_name);
		perror(LC_prog_name); // More precise report from cstdlib.
		return ret;

	default:
		return ret;
	}

	/* Helpers for printing values. */
	#define print_string(header, fmt, test, var, arr, arr_len) \
	if(test) printf(header "_var = \"" fmt "\"; ", var); \
	\
	if(arr_len) { \
		printf(header "_arr = {"); \
		for(size_t i = 0; i < arr_len; i++) { \
			printf("\"" fmt "\", ", arr[i]); \
		} \
		printf("...}; "); \
	}

	#define print_numeric(header, fmt, var, arr, arr_len) \
	if(var) printf(header "_var = " fmt "; ", var); \
	\
	if(arr_len) { \
		printf(header "_arr = {"); \
		for(size_t i = 0; i < arr_len; i++) { \
			printf(fmt ", ", arr[i]); \
		} \
		printf("...}; "); \
	}

	/* Print all the values that were set. */
	if(boolean_var) printf("boolean_var = true; ");

	print_string("string", "%s", string_var, string_var, string_arr,
		string_arr_len);

	print_numeric("int", "%d", int_var, int_arr, int_arr_len);

	print_numeric("double", "%2.lf", double_var, double_arr,
		double_arr_len);


	print_numeric("size", "%zu", size_var, size_arr, size_arr_len);
	print_numeric("int32", "%" PRId32, int32_var, int32_arr,
		int32_arr_len);

	print_numeric("oct", "%" PRIo8, oct_var, oct_arr, oct_arr_len);
	print_numeric("hex", "%" PRIx8, hex_var, hex_arr, hex_arr_len);
	print_string("filename", "%s", fname_var[0], fname_var, fname_arr,
		fname_arr_len);

	if(limited_arr_len) {
		printf("limited_arr = {");
		for(size_t i = 0; i < limited_arr_len; i++) {
			printf("%d, ", limited_arr[i]);
		}
		printf("...}; ");
	}

	/* Print out the flagless arguments. */
	if(LC_flagless_args_length) {
		printf("flagless_args = {");
		for(size_t i = 0; i < LC_flagless_args_length; i++) {
			printf("\"%s\", ", LC_flagless_args[i]);
		}
		printf("...}; ");
	}

	/* Return successfully. */
	printf("...");
	return 0;
}
