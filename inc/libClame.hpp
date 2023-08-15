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
#ifndef LIBCLAME_HPP
#define LIBCLAME_HPP 1

/* Standard Library Includes. */
#include <exception>
#include <functional>
#include <list>
#include <optional>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

/* Main Program Header. */
extern "C" {
#include <libClame.h>
#include <LC_macros.h>
}

/* Header File Namespace. */
namespace libClame {
	/* C++ style callback function definition. */
	typedef std::function<void()> callback_t;

	/* Flag to call a function */
	extern LC_flag_t make_call(
		std::string lflag, char sflag, callback_t function
	);

	/* Flag to set a boolean to a given value. */
	extern LC_flag_t make_bool(
		std::string lflag, char sflag, bool& var, bool val,
		std::optional<callback_t> function
	);

	/* Flags to get config strings */
	extern LC_flag_t make_string(
		std::string lflag, char sflag, std::string& string,
		std::optional<callback_t> function
	);

	typedef std::tuple<size_t, size_t> limits_t;

	extern LC_flag_t make_str_arr(
		std::string lflag, char sflag, std::list<std::string>& strings,
		std::optional<limits_t> limits,
		std::optional<callback_t> function
	);

	extern LC_flag_t make_str_arr(
		std::string lflag, char sflag,
		std::vector<std::string>& strings,
		std::optional<limits_t> limits,
		std::optional<callback_t> function
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
