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

/* Included by <libClame/generics.hpp> in the libClame namespace. */
#ifndef LC_GENERICS_HPP
#include <libClame/generics.hpp>
#else

/* Helper for getting scanf codes. */
template<typename T>
std::string __get_fmt() {
	if constexpr(std::is_same_v<T, char>) return "%c";
	if constexpr(std::is_same_v<T, float>) return "%f";
	if constexpr(std::is_same_v<T, double>) return "%lf";

	if constexpr(std::is_same_v<T, int8_t>) return "%" SCNd8;
	if constexpr(std::is_same_v<T, int16_t>) return "%" SCNd16;
	if constexpr(std::is_same_v<T, int32_t>) return "%" SCNd32;
	if constexpr(std::is_same_v<T, int64_t>) return "%" SCNd64;

	if constexpr(std::is_same_v<T, uint8_t>) return "%" SCNu8;
	if constexpr(std::is_same_v<T, uint16_t>) return "%" SCNu16;
	if constexpr(std::is_same_v<T, uint32_t>) return "%" SCNu32;
	if constexpr(std::is_same_v<T, uint64_t>) return "%" SCNu64;

	return ""; // The user must specify a custom format.
}

/* Tables for C/C++ interop. */
template<typename T>
std::unordered_map<std::string, std::tuple<T*, size_t>> __c_arr_table;

template<typename T>
LC_flag_t __make_var(
	std::string lflag, char sflag, T& var, std::string sscanf_fmt,
	callback_t function
){
	/* Make a copy of the flag that won't get mutated. */
	__string_list.push_back(std::move(lflag));
	const auto& c_lflag = (
		*__string_list.rbegin()
	).c_str();

	/* Run the callback code or an empty lambda. */
	__call_table[c_lflag] = function;

	/* Get the format string and store it away. */
	__string_list.push_back(sscanf_fmt);

	const auto fmt = (*__string_list.rbegin()).c_str();

	/* Make the structure. */
	return LC_MAKE_VAR_F(
		c_lflag, sflag, var, fmt, __interceptor
	);
}

template<typename T>
LC_flag_t make_var(std::string lflag, char sflag, T& var) {
	return __make_var(lflag, sflag, var, __get_fmt<T>(), [](){});
}

template<typename T>
LC_flag_t make_var(
	std::string lflag, char sflag, T& var, callback_t function
){
	return __make_var(lflag, sflag, var, __get_fmt<T>(), function);
}

template<typename T>
LC_flag_t make_var(
	std::string lflag, char sflag, T& var, std::string sscanf_fmt
){
	return __make_var(lflag, sflag, var, sscanf_fmt, [](){});
}

template<typename T>
LC_flag_t make_var(
	std::string lflag, char sflag, T& var, std::string sscanf_fmt,
	callback_t function
){
	return __make_var(lflag, sflag, var, sscanf_fmt, function);
}

template<template<typename> typename C, typename T>
requires ok_container<C, T>
LC_flag_t __make_arr(
	std::string& lflag, char sflag, C<T>* arr_ptr, limits_t limits,
	std::string sscanf_fmt, callback_t function
){
	/* Make a copy of the flag that won't get mutated. */
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

		/* Move the values to a C++ string list. */
		for(size_t i = 0; i < c_arr_len; i++) {
			arr.push_back(c_arr[i]);
		}

		/* Free the array. */
		std::free(c_arr);
		c_arr = NULL;

		/* Run the callback code if it was specified. */
		function();
	};

	/* Get the format string and store it away. */
	__string_list.push_back(sscanf_fmt);

	const auto fmt = (*__string_list.rbegin()).c_str();

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
	return __make_arr(
		lflag, sflag, &arr, {0, SIZE_MAX}, __get_fmt<T>(), [](){}
	);
}

template<template<typename> typename C, typename T>
requires ok_container<C, T>
LC_flag_t make_arr(
	std::string lflag, char sflag, C<T>& arr, callback_t function
){
	return __make_arr(
		lflag, sflag, &arr, {0, SIZE_MAX}, __get_fmt<T>(), function
	);
}

template<template<typename> typename C, typename T>
requires ok_container<C, T>
LC_flag_t make_arr(
	std::string lflag, char sflag, C<T>& arr, limits_t limits
){
	return __make_arr(lflag, sflag, &arr, limits, __get_fmt<T>(), [](){});
}

template<template<typename> typename C, typename T>
requires ok_container<C, T>
LC_flag_t make_arr(
	std::string lflag, char sflag, C<T>& arr, limits_t limits,
	callback_t function
){
	return __make_arr(
		lflag, sflag, &arr, limits, __get_fmt<T>(), function
	);
}

template<template<typename> typename C, typename T>
requires ok_container<C, T>
LC_flag_t make_arr(
	std::string lflag, char sflag, C<T>& arr, std::string sscanf_fmt
){
	return __make_arr(lflag, sflag, &arr, {0, SIZE_MAX}, sscanf_fmt, [](){});
}

template<template<typename> typename C, typename T>
requires ok_container<C, T>
LC_flag_t make_arr(
	std::string lflag, char sflag, C<T>& arr, std::string sscanf_fmt,
	callback_t function
){
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

#define LC_GENERICS_MARK_BY_CONTAINER(property, c, t) \
property LC_flag_t libClame::make_arr<c, t>(std::string, char, c<t>&); \
\
property LC_flag_t libClame::make_arr<c, t>( \
	std::string, char, c<t>&, libClame::limits_t \
); \
\
property LC_flag_t libClame::make_arr<c, t>( \
	std::string, char, c<t>&, libClame::callback_t \
); \
\
property LC_flag_t libClame::make_arr<c, t>( \
	std::string, char, c<t>&, limits_t, libClame::callback_t \
); \
\
property LC_flag_t libClame::make_arr<c, t>( \
	std::string, char, c<t>&, std::string \
); \
\
property LC_flag_t libClame::make_arr<c, t>( \
	std::string, char, c<t>&, std::string, libClame::callback_t \
); \
\
property LC_flag_t libClame::make_arr<c, t>( \
	std::string, char, c<t>&, libClame::limits_t, std::string \
); \
\
property LC_flag_t libClame::make_arr<c, t>( \
	std::string, char, c<t>&, libClame::limits_t, std::string, \
	libClame::callback_t \
);

#define LC_GENERICS_MARK_BY_TYPE(property, t) \
property LC_flag_t libClame::make_var<t>(std::string, char, t&); \
\
property LC_flag_t libClame::make_var<t>( \
	std::string, char, t&, libClame::callback_t \
); \
\
property LC_flag_t libClame::make_var<t>(std::string, char, t&, std::string); \
\
property LC_flag_t libClame::make_var<t>( \
	std::string, char, t&, std::string, libClame::callback_t \
); \
\
LC_GENERICS_MARK_BY_CONTAINER(property, std::list, t) \
LC_GENERICS_MARK_BY_CONTAINER(property, std::vector, t)

LC_GENERICS_MARK_BY_TYPE(extern template, char)
LC_GENERICS_MARK_BY_TYPE(extern template, float)
LC_GENERICS_MARK_BY_TYPE(extern template, double)

LC_GENERICS_MARK_BY_TYPE(extern template, int8_t)
LC_GENERICS_MARK_BY_TYPE(extern template, int16_t)
LC_GENERICS_MARK_BY_TYPE(extern template, int32_t)
LC_GENERICS_MARK_BY_TYPE(extern template, int64_t)

LC_GENERICS_MARK_BY_TYPE(extern template, uint8_t)
LC_GENERICS_MARK_BY_TYPE(extern template, uint16_t)
LC_GENERICS_MARK_BY_TYPE(extern template, uint32_t)
LC_GENERICS_MARK_BY_TYPE(extern template, uint64_t)

/* End Header Guard. */
#endif
