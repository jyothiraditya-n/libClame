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
#ifndef LC_TEMPLATES_HPP
#define LC_TEMPLATES_HPP 1

/* Main Header File. */
#include <libClame.hpp>

/* Header File Namespace. */
namespace libClame {
#include <libClame/__templates.hpp> // Helper functions.

	/* Flags to get a variable or an array of other types. */
	template<typename T>
	extern LC_flag_t make_var(
		std::string lflag, char sflag, T& var,
		std::optional<callback_t> function,
		std::optional<std::string> sscanf_fmt
	){
		/* Make a copy of the flag that won't get mutated. */
		libClame::__string_table.push_back(std::move(lflag));
		const auto& c_lflag = (
			*libClame::__string_table.rbegin()
		).c_str();

		/* Run the callback code or an empty lambda. */
		libClame::__call_table[c_lflag] = function.value_or([](){});

		/* Get the format string and store it away. */
		libClame::__string_table.push_back(
			sscanf_fmt.value_or(libClame::__get_fmt<T>())
		);

		const auto fmt = (*libClame::__string_table.rbegin()).c_str();

		/* The variables are: long_flag, short_flag, function, var_ptr,
		* var_type, value, fmt_string, arr_length, var_length,
		* min_arr_length, max_arr_length, readonly. */

		return LC_flag_t{
			c_lflag, sflag, libClame::__interceptor, &var,
			LC_OTHER_VAR, 0, fmt, NULL, sizeof(T), 0, 0, false
		};
	}

	template<typename T>
	extern LC_flag_t make_arr(
		std::string lflag, char sflag, std::list<T>& arr,
		std::optional<libClame::limits_t> limits,
		std::optional<callback_t> function,
		std::optional<std::string> sscanf_fmt
	){
		return libClame::__make_arr(
			lflag, sflag, arr, limits, function, sscanf_fmt
		);
	}

	template<typename T>
	extern LC_flag_t make_arr(
		std::string lflag, char sflag, std::vector<T>& arr,
		std::optional<libClame::limits_t> limits,
		std::optional<callback_t> function,
		std::optional<std::string> sscanf_fmt
	){
		return libClame::__make_arr(
			lflag, sflag, arr, limits, function, sscanf_fmt
		);
	}
}
/* End Header Guard */
#endif
