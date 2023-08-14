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

#include <iomanip>
#include <iostream>

#define LC_REQ_VER 1
#define LC_REQ_SUBVER 2

#include <libClame.hpp>
#include <libClame/templates.hpp>

using namespace libClame;

/* Function that prints when it's called. */
void custom_callback() {
	std::cout << "custom_callback(); ";
}

/* We're going to stress-test every data type we can think of. */
bool boolean_var;
std::string string_var; std::list<std::string> string_arr;

int int_var; std::list<int> int_arr;
double double_var; std::list<double> double_arr;
size_t size_var; std::list<size_t> size_arr;
int32_t int32_var; std::list<int32_t> int32_arr;

uint8_t oct_var; std::list<uint8_t> oct_arr;
uint8_t hex_var; std::list<uint8_t> hex_arr;

typedef struct {char str[9];} filename_t; // 8-char filenames like DOS.
filename_t filename_var; std::list<filename_t> filename_arr;

std::list<int> limited_arr; // Arr of only two values.

/* Arguments list. */
std::vector<LC_flag_t> flags;

int main(int argc, char **argv) {
	/* Set up our flags. */
	flags.push_back(make_call("callback", 'c', custom_callback));
	flags.push_back(make_bool("boolean_var", 'b', boolean_var, true, {}));

	flags.push_back(make_string("string_var", 's', string_var, {}));
	flags.push_back(make_str_arr("string_arr", 'S', string_arr, {}, {}));

	flags.push_back(make_var("int_var", 'i', int_var, {}, {}));
	flags.push_back(make_arr("int_arr", 'I', int_arr, {}, {}, {}));

	flags.push_back(make_var("double_var", 'd', double_var, {}, {}));
	flags.push_back(make_arr("double_arr", 'D', double_arr, {}, {}, {}));

	flags.push_back(make_var("size_var", 'z', size_var, {}, {}));
	flags.push_back(make_arr("size_arr", 'Z', size_arr, {}, {}, {}));

	flags.push_back(make_var("int32_var", 'l', int32_var, {}, {}));
	flags.push_back(make_arr("int32_arr", 'L', int32_arr, {}, {}, {}));

	flags.push_back(make_var("oct_var", 'o', oct_var, {}, "%" SCNo8));
	flags.push_back(make_arr("oct_arr", 'O', oct_arr, {}, {}, "%" SCNo8));

	flags.push_back(make_var("hex_var", 'x', hex_var, {}, "%" SCNx8));
	flags.push_back(make_arr("hex_arr", 'X', hex_arr, {}, {}, "%" SCNx8));

	flags.push_back(
		make_var("filename_var", 'f', filename_var, {}, "%8s")
	);

	flags.push_back(
		make_arr("filename_arr", 'F', filename_arr, {}, {}, "%8s")
	);

	flags.push_back(make_arr(
		"limited_arr", '2', limited_arr, limits_t{2, 2}, {}, {}
	));

	try {
		libClame::read(argc, argv, flags);
	}

	catch(libClame::exception& e) {
		switch(e.error) {
		case LC_OK: break;

		case LC_MALLOC_ERR:
			std::cout << libClame::prog_name
				<< ": error allocating memory.\n";

			std::perror(LC_prog_name); // More precise report.
			break;

		default:
			return e.error;
		}
	}

	/* Print all the values that were set. */
	if(boolean_var) std::cout << "boolean_var = true; ";

	if(string_var != "") {
		std::cout << "string_var = \"" << string_var << "\"; ";
	}

	if(string_arr.size()) {
		std::cout << "string_arr = {";
		for(const auto &i: string_arr) {
			std::cout << "\"" << i << "\", ";
		}
		std::cout << "...}; ";
	}

	if(int_var) std::cout << "int_var = " << int_var << "; ";

	if(int_arr.size()) {
		std::cout << "int_arr = {";
		for(const auto &i: int_arr) std::cout << i << ", ";
		std::cout << "...}; ";
	}

	std::cout << std::setprecision(3);

	if(double_var) std::cout << "double_var = " << double_var << "; ";

	if(double_arr.size()) {
		std::cout << "double_arr = {";
		for(const auto &i: double_arr) std::cout << i << ", ";
		std::cout << "...}; ";
	}

	if(size_var) std::cout << "size_var = " << size_var << "; ";

	if(size_arr.size()) {
		std::cout << "size_arr = {";
		for(const auto &i: size_arr) std::cout << i << ", ";
		std::cout << "...}; ";
	}

	if(int32_var) std::cout << "int32_var = " << int32_var << "; ";

	if(int32_arr.size()) {
		std::cout << "int32_arr = {";
		for(const auto &i: int32_arr) std::cout << i << ", ";
		std::cout << "...}; ";
	}

	std::cout << std::oct;

	if(oct_var) std::cout << "oct_var = " << int{oct_var} << "; ";

	if(oct_arr.size()) {
		std::cout << "oct_arr = {";
		for(const auto &i: oct_arr) std::cout << int{i} << ", ";
		std::cout << "...}; ";
	}

	std::cout << std::hex;

	if(hex_var) std::cout << "hex_var = " << int{hex_var} << "; ";

	if(hex_arr.size()) {
		std::cout << "hex_arr = {";
		for(const auto &i: hex_arr) std::cout << int{i} << ", ";
		std::cout << "...}; ";
	}

	if(filename_var.str[0]) {
		std::cout << "filename_var = \"" << filename_var.str << "\"; ";
	}

	if(filename_arr.size()) {
		std::cout << "filename_arr = {";
		for(const auto &i: filename_arr) {
			std::cout << "\"" << i.str << "\", ";
		}
		std::cout << "...}; ";
	}

	std::cout << std::dec;

	if(limited_arr.size()) {
		std::cout << "limited_arr = {";
		for(const auto &i: limited_arr) std::cout << i << ", ";
		std::cout << "...}; ";
	}

	/* Print out the flagless arguments. */
	if(flagless_args.size()) {
		std::cout << "libClame::flagless_args = {";
		for(const auto &i: flagless_args) {
			std::cout << "\"" << i << "\", ";
		}
		std::cout << "...}; ";
	}

	/* Return successfully. */
	std::cout << "...";
	return 0;
}
