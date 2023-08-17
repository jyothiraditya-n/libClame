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
#include <libClame.hpp>

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
