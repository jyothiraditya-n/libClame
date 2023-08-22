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
#ifndef LIBCLAME_HH
#define LIBCLAME_HH 1

/* Standard Library Includes. */
#include <string>

#include <tuple>
#include <unordered_map>

#include <list>
#include <vector>

#include <functional>
#include <exception>
#include <concepts>

/* Main Program Header. */
extern "C" {
#include <libClame.h>
#include <LC_macros.h>
}

/* Header File Namespace. */
namespace libClame {
	/* C++ style callback function definition. */
	typedef std::function<void()> callback_t;

	/* We can work with std::list, std::forward_list and std::vector for
	 * arrays of values. */
	template<template<typename> typename C, typename T>
	concept ok_container = (
		std::is_same_v<C<T>, std::list<T>>
		|| std::is_same_v<C<T>, std::vector<T>>
	);

	/* Arrays scanned in can have a minimum and maximum size. */
	typedef std::tuple<size_t, size_t> limits_t;

	/* Flag to call a function */
	extern LC_flag_t make_call(
		std::string lflag, char sflag, callback_t function
	);

	/* Flag to set a boolean to a given value. */
	extern LC_flag_t make_bool(
		std::string lflag, char sflag, bool& var, bool val
	);

	extern LC_flag_t make_bool(
		std::string lflag, char sflag, bool& var, bool val,
		callback_t function
	);

	/* Flags to get config strings */
	extern LC_flag_t make_string(
		std::string lflag, char sflag, std::string& string
	);

	extern LC_flag_t make_string(
		std::string lflag, char sflag, std::string& string,
		callback_t function
	);

	template<template<typename> typename C>
	requires ok_container<C, std::string>
	extern LC_flag_t make_str_arr(
		std::string lflag, char sflag, C<std::string>& strings
	);

	template<template<typename> typename C>
	requires ok_container<C, std::string>
	extern LC_flag_t make_str_arr(
		std::string lflag, char sflag, C<std::string>& strings,
		callback_t function
	);

	template<template<typename> typename C>
	requires ok_container<C, std::string>
	extern LC_flag_t make_str_arr(
		std::string lflag, char sflag, C<std::string>& strings,
		limits_t limits
	);

	template<template<typename> typename C>
	requires ok_container<C, std::string>
	extern LC_flag_t make_str_arr(
		std::string lflag, char sflag, C<std::string>& strings,
		limits_t limits, callback_t function
	);

	/* Command to begin command-line argument processing. */
	extern void read(int argc, char** argv, std::vector<LC_flag_t>& flags);

	/* Exception type for if read() throws. */
	class exception : std::exception {
	public:
		explicit exception(int error);
		virtual ~exception() noexcept;
		virtual const char *what() const noexcept;
		int error;
	};

	/* Non-flag variables encountered during processing. */
	extern std::vector<std::string> flagless_args;

	/* Program name set via argv[0]. */
	extern std::string prog_name;

	/* Tables for C/C++ interop and C++ lambda cache tables. */
	extern std::list<std::string> __string_list;
	extern std::unordered_map<std::string, callback_t> __call_table;
	extern std::unordered_map<std::string, callback_t> __shadow_table;
	extern std::unordered_map<std::string, char*> __c_string_table;

	extern std::unordered_map<std::string, std::tuple<char**, size_t>>
		__c_strarr_table;

	/* Function call interceptor. */
	extern int __interceptor(LC_flag_t* __c_flag);
};

/* End Header Guard */
#endif
