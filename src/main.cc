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

/* Header Files */
#include <cstdlib>

/* Main Header File. */
#include <libClame.hh>

/* Flag to call a function */
LC_flag_t libClame::make_call(
	std::string lflag, char sflag, libClame::callback_t function
){
	/* Copy the flag since lflag is invalid after function scope. */
	libClame::__string_list.push_back(std::move(lflag));
	const auto c_lflag = (*libClame::__string_list.rbegin()).c_str();

	/* Add the function to our call table. */
	libClame::__call_table[c_lflag] = function;

	/* Use our standard C macro to make the struct. */
	return LC_flag_t LC_MAKE_CALL(
		c_lflag, sflag, libClame::__interceptor
	);
}

/* Flag to set a boolean to a given value. */

/* We'll use a helper function that takes all possible arguments, and call it
 * through each of the overloaded interface functions we need to make. */
static LC_flag_t __make_bool(
	std::string& lflag, char sflag, bool& var, bool val,
	libClame::callback_t function
){
	/* Make a copy of the flag that won't get mutated. */
	libClame::__string_list.push_back(std::move(lflag));
	const auto c_lflag = (*libClame::__string_list.rbegin()).c_str();

	/* Add the function to our call table. */
	libClame::__call_table[c_lflag] = function;

	/* Make the structure. */
	return LC_MAKE_BOOL_F(
		c_lflag, sflag, var, val, libClame::__interceptor
	);
}

/* Overloaded interface functions. */
LC_flag_t libClame::make_bool(
	std::string lflag, char sflag, bool& var, bool val
){
	/* Pass in a dummy lambda that does nothing. */
	return __make_bool(lflag, sflag, var, val, [](){});
}

LC_flag_t libClame::make_bool(
	std::string lflag, char sflag, bool& var, bool val,
	libClame::callback_t function
){
	return __make_bool(lflag, sflag, var, val, function);
}

/* Flags to get config strings */

/* Same trick as with the bools. */
LC_flag_t __make_string(
	std::string& lflag, char sflag, std::string* string_ptr,
	libClame::callback_t function
){
	/* Make a copy of the flag that won't get mutated. */
	libClame::__string_list.push_back(std::move(lflag));
	const auto c_lflag = (*libClame::__string_list.rbegin()).c_str();

	/* Pointer to the string copied from argv[] that we'll share with C.
	 * It has to be declared in the table since it lives beyond the scope
	 * of these helper functions. */
	auto& c_string = libClame::__c_string_table[c_lflag] = NULL;

	/* Add the function to our shadow table. */
	libClame::__shadow_table[c_lflag] = function;

	/* Add the assignment wrapper function to our call table. */
	libClame::__call_table[c_lflag] = [c_lflag, string_ptr]() {
		/* Dereference the pointer to get a C++ reference. */
		auto& string = *string_ptr;

		/* Get a reference to the original items we instantialised. */
		const auto& c_string = libClame::__c_string_table[c_lflag];
		const auto& function = libClame::__shadow_table[c_lflag];

		/* Move the value to a C++ string. */
		string = c_string;

		/* Run the callback code. */
		function();
	};

	/* Make the structure. */
	return LC_MAKE_STRING_F(
		c_lflag, sflag, c_string, libClame::__interceptor
	);
}

/* Overloaded interface functions. */
LC_flag_t libClame::make_string(
	std::string lflag, char sflag, std::string& string,
	libClame::callback_t function
){
	return __make_string(lflag, sflag, &string, function);
}

/* Overloaded interface functions. */
LC_flag_t libClame::make_string(
	std::string lflag, char sflag, std::string& string
){
	/* Pass in a dummy lambda that does nothing. */
	return __make_string(lflag, sflag, &string, [](){});
}

/* Code for handling string arrays. */
template<template<typename> typename C>
requires libClame::ok_container<C, std::string>
static LC_flag_t __make_str_arr(
	std::string& lflag, char sflag, C<std::string>* strings_ptr,
	libClame::limits_t limits,
	libClame::callback_t function
){
	/* Make a copy of the flag that won't get mutated. */
	libClame::__string_list.push_back(std::move(lflag));
	const auto c_lflag = (*libClame::__string_list.rbegin()).c_str();

	/* Pointer to C string arr copied from argv[] that we'll manage. */
	auto& c_strarr_entry = libClame::__c_strarr_table[c_lflag] = {NULL, 0};
	auto& c_strarr = std::get<char**>(c_strarr_entry);
	auto& c_strarr_len = std::get<size_t>(c_strarr_entry);

	/* Add the function to our shadow table. Empty lambda if nothing was
	 * provided in the optional. */
	libClame::__shadow_table[c_lflag] = function;

	/* Add the wrapper function to our call table. */
	libClame::__call_table[c_lflag] = [c_lflag, strings_ptr](){
		/* Dereference the pointer to get a C++ reference. */
		auto& strings = *strings_ptr;

		/* Get the reference to the array we were sharing with C */
		auto& c_strarr_entry = libClame::__c_strarr_table[c_lflag];
		auto& c_strarr = std::get<char**>(c_strarr_entry);
		auto& c_strarr_len = std::get<size_t>(c_strarr_entry);

		/* Get the reference to the function we were provided. */
		const auto& function = libClame::__shadow_table[c_lflag];

		/* Clear the strings. */
		strings.clear();
		strings.resize(c_strarr_len);

		/* Move the values to a C++ string list. */
		size_t i1 = 0;
		for(auto& i2: strings) i2 = c_strarr[i1++];

		/* Free the array (but not the elements it points to.) */
		std::free(c_strarr);
		c_strarr = NULL;

		/* Run the callback code. */
		function();
	};

	/* Get the limits for the array if they are defined. */
	const auto set_limits = limits;

	const auto& min = std::get<0>(set_limits);
	const auto& max = std::get<1>(set_limits);

	/* Make the structure. */
	return LC_MAKE_STRING_ARR_BOUNDED_F(
		c_lflag, sflag, c_strarr, c_strarr_len, min, max,
		libClame::__interceptor
	);
}

template<template<typename> typename C>
requires libClame::ok_container<C, std::string>
LC_flag_t libClame::make_str_arr(
	std::string lflag, char sflag, C<std::string>& strings
){
	return __make_str_arr(lflag, sflag, &strings, {0, SIZE_MAX}, [](){});
}

template<template<typename> typename C>
requires libClame::ok_container<C, std::string>
LC_flag_t libClame::make_str_arr(
	std::string lflag, char sflag, C<std::string>& strings,
	libClame::callback_t function
){
	return __make_str_arr(lflag, sflag, &strings, {0, SIZE_MAX}, function);
}

template<template<typename> typename C>
requires libClame::ok_container<C, std::string>
LC_flag_t libClame::make_str_arr(
	std::string lflag, char sflag, C<std::string>& strings,
	libClame::limits_t limits
){
	return __make_str_arr(lflag, sflag, &strings, limits, [](){});
}

template<template<typename> typename C>
requires libClame::ok_container<C, std::string>
LC_flag_t libClame::make_str_arr(
	std::string lflag, char sflag, C<std::string>& strings,
	libClame::limits_t limits, libClame::callback_t function
){
	return __make_str_arr(lflag, sflag, &strings, limits, function);
}

/* Force templates to instantialise */
#define instantialise(c) \
template LC_flag_t libClame::make_str_arr<c>( \
	std::string, char, c<std::string>& \
); \
\
template LC_flag_t libClame::make_str_arr<c>( \
	std::string, char, c<std::string>&, libClame::callback_t \
); \
\
template LC_flag_t libClame::make_str_arr<c>( \
	std::string, char, c<std::string>&, libClame::limits_t \
); \
\
template LC_flag_t libClame::make_str_arr<c>( \
	std::string, char, c<std::string>&, libClame::limits_t, \
	libClame::callback_t \
); \

instantialise(std::list)
instantialise(std::vector)

/* Command to begin command-line argument processing. */
void libClame::read(int argc, char** argv, std::vector<LC_flag_t>& flags) {
	/* Set up the C flags structure. */
	LC_flags = flags.data();
	LC_flags_length = flags.size();

	/* Call the C parsing function. */
	int ret = LC_read(argc, argv);

	/* Throw an exception if the value wasn't LC_OK. */
	if(ret != LC_OK) throw libClame::exception(ret);

	/* Copy out the flagless arguments. */
	libClame::flagless_args.reserve(LC_flagless_args_length);

	for(size_t i = 0; i < LC_flagless_args_length; i++) {
		libClame::flagless_args.push_back(LC_flagless_args[i]);
	}

	/* Copy out the program name. */
	libClame::prog_name = LC_prog_name;
}

/* Non-flag variables encountered during processing. */
std::vector<std::string> libClame::flagless_args;

/* Program name set via argv[0]. */
std::string libClame::prog_name;

/* Exception logic code for if read() throws. */
libClame::exception::exception(int error): error{error} {}
libClame::exception::~exception() noexcept {}

const char *libClame::exception::what() const noexcept {
	return LC_strerror(this -> error);
}

/* Tables for C/C++ interop. */
std::list<std::string> libClame::__string_list;
std::unordered_map<std::string, libClame::callback_t> libClame::__call_table;
std::unordered_map<std::string, libClame::callback_t> libClame::__shadow_table;

std::unordered_map<std::string, char*> libClame::__c_string_table;

std::unordered_map<std::string, std::tuple<char**, size_t>>
	libClame::__c_strarr_table;

/* Function call __interceptor. */
int libClame::__interceptor(LC_flag_t* c_flag) {
	/* Synthesize our flastringgs into a C++ type. */
	const auto lflag = std::string{c_flag -> long_flag};

	/* Look the correct C++ function up in the calltable. */
	try {
		const auto function = libClame::__call_table.at(lflag);
		function();
	}

	catch(...) {
		/* Exceptions must not propagate to C code. */
		return LC_FUNCTION_ERR;
	}

	/* Nothing caught fire! */
	return LC_OK;
}
