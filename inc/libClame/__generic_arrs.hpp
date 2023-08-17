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

/* Ultimately included by <libClame/generics.hpp> in the libClame namespace. */
#ifndef LC_GENERICS_HPP
#include <libClame/generics.hpp>
#else

/* Flags to get arrays of various types. */

/* We'll use a helper function that takes all possible arguments, and call it
 * through each of the overloaded interface functions we need to make. */
template<template<typename> typename C, typename T>
requires ok_container<C, T>
LC_flag_t __make_arr(
	std::string& lflag, char sflag, C<T>* arr_ptr, limits_t limits,
	std::string sscanf_fmt, callback_t function
){
	/* Copy the flag since lflag is invalid after function scope. */
	__string_list.push_back(std::move(lflag));
	const auto c_lflag = (*__string_list.rbegin()).c_str();

	/* Pointer to malloc()'d C arr we'll manage. */
	auto& c_arr_entry = __c_arr_table<T>[c_lflag] = {NULL, 0};
	auto& c_arr = std::get<0>(c_arr_entry);
	auto& c_arr_len = std::get<1>(c_arr_entry);

	/* Add the function to our shadow table. Empty lambda if nothing was
	 * provided in the optional. */
	__shadow_table[c_lflag] = function;

	/* Add our lambda function  */
	__call_table[c_lflag] = [c_lflag, arr_ptr](){
		/* Dereference the pointer to get a C++ reference. */
		auto& arr = *arr_ptr;

		/* Pointer to malloc()'d C arr we'll manage. */
		auto& c_arr_entry = __c_arr_table<T>[c_lflag];
		auto& c_arr = std::get<0>(c_arr_entry);
		auto& c_arr_len = std::get<1>(c_arr_entry);

		/* Get the reference to the function we were provided. */
		const auto& function = __shadow_table[c_lflag];

		/* Clear the C++ array. */
		arr.clear();
		arr.resize(c_arr_len);

		/* Copy the values over. */
		size_t i1 = 0;
		for(auto& i2: arr) i2 = c_arr[i1++];

		/* Free the array. */
		std::free(c_arr);
		c_arr = NULL;

		/* Run the callback code if it was specified. */
		function();
	};

	/* Get the format string and store it away. */
	__string_list.push_back(sscanf_fmt);

	auto fmt = (*__string_list.rbegin()).c_str();

	/* Get the limits for the array as they are defined. */
	const auto& min = std::get<0>(limits);
	const auto& max = std::get<1>(limits);

	/* Make the structure. */
	return LC_MAKE_ARR_BOUNDED_F(
		c_lflag, sflag, c_arr, fmt, c_arr_len, min, max,
		__interceptor
	);
}

template<template<typename> typename C, typename T>
requires ok_container<C, T>
LC_flag_t make_arr(
	std::string lflag, char sflag, C<T>& arr
){
	/* Try to auto deduce the type. */
	constexpr auto fmt = __get_fmt<T>();
	static_assert(!fmt.empty(), "Type requires explicit format string.");

	/* Pass in a dummy lambda that does nothing. Set the size limits to
	 * accept any length of array. */
	return __make_arr(
		lflag, sflag, &arr, {0, SIZE_MAX}, std::string{fmt}, [](){}
	);
}

template<template<typename> typename C, typename T>
requires ok_container<C, T>
LC_flag_t make_arr(
	std::string lflag, char sflag, C<T>& arr, callback_t function
){
	/* Try to auto deduce the type. */
	constexpr auto fmt = __get_fmt<T>();
	static_assert(!fmt.empty(), "Type requires explicit format string.");

	/* Set the size limits to accept any length of array. */
	return __make_arr(
		lflag, sflag, &arr, {0, SIZE_MAX}, std::string{fmt}, function
	);
}

template<template<typename> typename C, typename T>
requires ok_container<C, T>
LC_flag_t make_arr(
	std::string lflag, char sflag, C<T>& arr, limits_t limits
){
	/* Try to auto deduce the type. */
	constexpr auto fmt = __get_fmt<T>();
	static_assert(!fmt.empty(), "Type requires explicit format string.");

	/* Pass in a dummy lambda that does nothing. */
	return __make_arr(lflag, sflag, &arr, limits, std::string{fmt}, [](){});
}

template<template<typename> typename C, typename T>
requires ok_container<C, T>
LC_flag_t make_arr(
	std::string lflag, char sflag, C<T>& arr, limits_t limits,
	callback_t function
){
	/* Try to auto deduce the type. */
	constexpr auto fmt = __get_fmt<T>();
	static_assert(!fmt.empty(), "Type requires explicit format string.");

	return __make_arr(
		lflag, sflag, &arr, limits, std::string{fmt}, function
	);
}

template<template<typename> typename C, typename T>
requires ok_container<C, T>
LC_flag_t make_arr(
	std::string lflag, char sflag, C<T>& arr, std::string sscanf_fmt
){
	/* Pass in a dummy lambda that does nothing. Set the size limits to
	 * accept any length of array. */
	return __make_arr(
		lflag, sflag, &arr, {0, SIZE_MAX}, sscanf_fmt, [](){}
	);
}

template<template<typename> typename C, typename T>
requires ok_container<C, T>
LC_flag_t make_arr(
	std::string lflag, char sflag, C<T>& arr, std::string sscanf_fmt,
	callback_t function
){
	/* Set the size limits to accept any length of array. */
	return __make_arr(
		lflag, sflag, &arr, {0, SIZE_MAX}, sscanf_fmt, function
	);
}

template<template<typename> typename C, typename T>
requires ok_container<C, T>
LC_flag_t make_arr(
	std::string lflag, char sflag, C<T>& arr, limits_t limits,
	std::string sscanf_fmt
){
	/* Pass in a dummy lambda that does nothing. */
	return __make_arr(lflag, sflag, &arr, limits, sscanf_fmt, [](){});
}

template<template<typename> typename C, typename T>
requires ok_container<C, T>
LC_flag_t make_arr(
	std::string lflag, char sflag, C<T>& arr, limits_t limits,
	std::string sscanf_fmt, callback_t function
){
	return __make_arr(lflag, sflag, &arr, limits, sscanf_fmt, function);
}

/* End Header Guard. */
#endif
