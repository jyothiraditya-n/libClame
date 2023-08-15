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
int32_t int32_var; std::vector<int32_t> int32_arr; // note the vector.

uint8_t oct_var; std::list<uint8_t> oct_arr;
uint8_t hex_var; std::vector<uint8_t> hex_arr; // note the vector.

typedef struct {char str[9];} filename_t; // 8-char filenames like DOS.
filename_t filename_var; std::list<filename_t> filename_arr;

std::list<int> limited_arr; // Arr of only two values.

/* Arguments list. */
std::vector<LC_flag_t> flags;

/* Helpers to print the variables out. */
void print_string(
	const std::string& var, const std::list<std::string>& arr,
	const std::string& desc
){
	if(var != "") std::cout << desc << "_var = \"" << var << "\"; ";

	if(!arr.size()) return;
	std::cout << desc << "_arr = {";

	for(const auto &i: arr) std::cout << "\"" << i << "\", ";
	std::cout << "...}; ";
}

void print_string(
	const filename_t &var, const std::list<filename_t>& arr,
	const std::string& desc
){
	/* Convert argument types. */
	std::list<std::string> strings;
	for(auto&& i: arr) strings.push_back(i.str);

	/* Call the main helper function. */
	print_string(var.str, strings, desc);
}

/* Don't print uint8_t as a character. */
std::ostream& operator<<(std::ostream& os, const uint8_t& obj){
	return os << (int) obj;
}

template<template<typename> typename C, typename T>
void print_numeric(
	const T& var, const C<T>& arr, const std::string& desc
){
	if(var) std::cout << desc << "_var = " << var << "; ";

	if(!arr.size()) return;
	std::cout << desc << "_arr = {";

	for(const auto &i: arr) std::cout << i << ", ";
	std::cout << "...}; ";
};

int main(int argc, char **argv) {
	/* Set up our flags. */
	flags.push_back(make_call("callback", 'c', custom_callback));
	flags.push_back(make_bool("boolean_var", 'b', boolean_var, true, {}));

	flags.push_back(make_bool(
		"boolean_callback", '!', boolean_var, true, custom_callback)
	);

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

	flags.push_back(make_arr(
		"limited_callback", '@', limited_arr, limits_t{2, 2},
		custom_callback, {}
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
			return e.error;

		default:
			return e.error;
		}
	}

	/* Print all the values that were set. */
	if(boolean_var) std::cout << "boolean_var = true; ";

	print_string(string_var, string_arr, "string");

	std::cout << std::setprecision(3);
	print_numeric(int_var, int_arr, "int");
	print_numeric(double_var, double_arr, "double");
	print_numeric(size_var, size_arr, "size");
	print_numeric(int32_var, int32_arr, "int32");

	std::cout << std::oct;
	print_numeric(oct_var, oct_arr, "oct");

	std::cout << std::hex;
	print_numeric(hex_var, hex_arr, "hex");

	print_string(filename_var, filename_arr, "filename");

	std::cout << std::dec;
	if(limited_arr.size()) {
		std::cout << "limited_arr = {";
		for(const auto &i: limited_arr) std::cout << i << ", ";
		std::cout << "...}; ";
	}

	/* Print out the flagless arguments. */
	if(flagless_args.size()) {
		std::cout << "flagless_args = {";
		for(const auto &i: flagless_args) {
			std::cout << "\"" << i << "\", ";
		}
		std::cout << "...}; ";
	}

	/* Return successfully. */
	std::cout << "...";
	return 0;
}
