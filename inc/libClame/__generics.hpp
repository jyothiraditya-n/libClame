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

/* Additional header files. */
#include <string_view>

/* Helper for getting scanf codes. The returned value must pass a static
 * assertion that the string is not empty. */
template<typename T>
constexpr std::string_view __get_fmt() {
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

/* Flags to get variables of various types. */

/* We'll use a helper function that takes all possible arguments, and call it
 * through each of the overloaded interface functions we need to make. */
template<typename T>
LC_flag_t __make_var(
	std::string lflag, char sflag, T& var, std::string sscanf_fmt,
	callback_t function
){
	/* Copy the flag since lflag is invalid after function scope. */
	__string_list.push_back(std::move(lflag));
	const auto& c_lflag = (
		*__string_list.rbegin()
	).c_str();

	/* Run the callback code. */
	__call_table[c_lflag] = function;

	/* Get the format string and store it away. */
	__string_list.push_back(sscanf_fmt);

	auto fmt = (*__string_list.rbegin()).c_str();

	/* Make the structure. */
	return LC_MAKE_VAR_F(
		c_lflag, sflag, var, fmt, __interceptor
	);
}

/* Overloaded interface functions. */
template<typename T>
LC_flag_t make_var(std::string lflag, char sflag, T& var) {
	/* Try to auto deduce the type. */
	constexpr auto fmt = __get_fmt<T>();
	static_assert(!fmt.empty(), "Type requires explicit format string.");

	/* Pass in a dummy lambda that does nothing. */
	return __make_var(lflag, sflag, var, std::string{fmt}, [](){});
}

template<typename T>
LC_flag_t make_var(
	std::string lflag, char sflag, T& var, callback_t function
){
	/* Try to auto deduce the type. */
	constexpr auto fmt = __get_fmt<T>();
	static_assert(!fmt.empty(), "Type requires explicit format string.");

	return __make_var(lflag, sflag, var, std::string{fmt}, function);
}

template<typename T>
LC_flag_t make_var(
	std::string lflag, char sflag, T& var, std::string sscanf_fmt
){
	/* Pass in a dummy lambda that does nothing. */
	return __make_var(lflag, sflag, var, sscanf_fmt, [](){});
}

template<typename T>
LC_flag_t make_var(
	std::string lflag, char sflag, T& var, std::string sscanf_fmt,
	callback_t function
){
	return __make_var(lflag, sflag, var, sscanf_fmt, function);
}

/* Include code for dealing with arrays. */
#include <libClame/__generic_arrs.hpp>

/* End Header Guard. */
#endif
