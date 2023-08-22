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

/* Begin Header Guard. */
#ifndef LC_GENERICS_HH
#define LC_GENERICS_HH 1

/* Main Header File. */
#include <libClame.hh>

/* Header File Namespace. */
namespace libClame {
	/* Flags to get variables of various types. */
	template<typename T>
	LC_flag_t make_var(std::string lflag, char sflag, T& var);

	template<typename T>
	LC_flag_t make_var(
		std::string lflag, char sflag, T& var, callback_t function
	);

	template<typename T>
	LC_flag_t make_var(
		std::string lflag, char sflag, T& var, std::string sscanf_fmt
	);

	template<typename T>
	LC_flag_t make_var(
		std::string lflag, char sflag, T& var, std::string sscanf_fmt,
		callback_t function
	);

	/* Flags to get arrays of various types. */
	template<template<typename> typename C, typename T>
	requires ok_container<C, T>
	LC_flag_t make_arr(
		std::string lflag, char sflag, C<T>& arr
	);

	template<template<typename> typename C, typename T>
	requires ok_container<C, T>
	LC_flag_t make_arr(
		std::string lflag, char sflag, C<T>& arr, callback_t function
	);

	template<template<typename> typename C, typename T>
	requires ok_container<C, T>
	LC_flag_t make_arr(
		std::string lflag, char sflag, C<T>& arr,
		libClame::limits_t limits
	);

	template<template<typename> typename C, typename T>
	requires ok_container<C, T>
	LC_flag_t make_arr(
		std::string lflag, char sflag, C<T>& arr,
		libClame::limits_t limits, callback_t function
	);

	template<template<typename> typename C, typename T>
	requires ok_container<C, T>
	LC_flag_t make_arr(
		std::string lflag, char sflag, C<T>& arr,
		std::string sscanf_fmt
	);

	template<template<typename> typename C, typename T>
	requires ok_container<C, T>
	LC_flag_t make_arr(
		std::string lflag, char sflag, C<T>& arr,
		std::string sscanf_fmt, callback_t function
	);

	template<template<typename> typename C, typename T>
	requires ok_container<C, T>
	LC_flag_t make_arr(
		std::string lflag, char sflag, C<T>& arr,
		libClame::limits_t limits, std::string sscanf_fmt
	);

	template<template<typename> typename C, typename T>
	requires ok_container<C, T>
	LC_flag_t make_arr(
		std::string lflag, char sflag, C<T>& arr,
		libClame::limits_t limits, std::string sscanf_fmt,
		callback_t function
	);

	/* The code for these templated functions. */
	#include <libClame/__generics.hh>
}

/* End Header Guard */
#endif
