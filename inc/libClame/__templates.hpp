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

/* Included by <libClame/templates.hpp> in the libClame namespace. */
#ifndef LC_TEMPLATES_HPP
#include <libClame/templates.hpp>
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

/* Don't write the same code twice. */
template<template<typename> typename T1, typename T2>
static LC_flag_t __make_arr(
	std::string& lflag, char sflag, T1<T2>* arr_ptr,
	std::optional<libClame::limits_t>& limits,
	std::optional<libClame::callback_t>& function,
	std::optional<std::string>& sscanf_fmt
){
	/* Make a copy of the flag that won't get mutated. */
	libClame::__string_list.push_back(std::move(lflag));
	const auto c_lflag = (*libClame::__string_list.rbegin()).c_str();

	/* Pointer to malloc()'d C arr we'll manage. */
	auto& c_arr_entry = libClame::__c_arr_table<T2>[c_lflag] = {NULL, 0};
	auto& c_arr = std::get<0>(c_arr_entry);
	auto& c_arr_len = std::get<1>(c_arr_entry);

	/* Add the function to our shadow table. Empty lambda if nothing was
	 * provided in the optional. */
	libClame::__shadow_table[c_lflag] = function.value_or([](){});;

	/* Add our lambda function  */
	libClame::__call_table[c_lflag] = [c_lflag, arr_ptr](){
		/* Dereference the pointer to get a C++ reference. */
		auto& arr = *arr_ptr;

		/* Pointer to malloc()'d C arr we'll manage. */
		auto& c_arr_entry = libClame::__c_arr_table<T2>[c_lflag];
		auto& c_arr = std::get<0>(c_arr_entry);
		auto& c_arr_len = std::get<1>(c_arr_entry);

		/* Get the reference to the function we were provided. */
		const auto& function = libClame::__shadow_table[c_lflag];

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
	libClame::__string_list.push_back(
		sscanf_fmt.value_or(libClame::__get_fmt<T2>())
	);

	const auto fmt = (*libClame::__string_list.rbegin()).c_str();

	/* Get the limits for the array if they are defined. */
	const auto set_limits = limits.value_or(
		std::tuple<size_t,size_t>{0, SIZE_MAX}
	);

	const auto& min = std::get<0>(set_limits);
	const auto& max = std::get<1>(set_limits);

	/* Make the structure. */
	return LC_MAKE_ARR_BOUNDED_F(
		c_lflag, sflag, c_arr, fmt, c_arr_len, min, max,
		libClame::__interceptor
	);
}

/* End Header Guard. */
#endif
