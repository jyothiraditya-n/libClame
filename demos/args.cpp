#define LICENCE_TEXT "\n" \
"  libClame: Command-line Arguments Made Easy\n" \
"  Copyright (C) 2021-2023 Jyothiraditya Nellakra\n" \
"  Demonstration Program for <LC_args.h>\n\n" \
\
"  This program is free software: you can redistribute it and/or modify\n" \
"  it under the terms of the GNU General Public License as published by\n" \
"  the Free Software Foundation, either version 3 of the License, or\n" \
"  (at your option) any later version.\n\n" \
\
"  This program is distributed in the hope that it will be useful,\n" \
"  but WITHOUT ANY WARRANTY; without even the implied warranty of\n" \
"  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the\n" \
"  GNU General Public License for more details.\n\n" \
\
"  You should have received a copy of the GNU General Public License\n" \
"  along with this program. If not, see <https://www.gnu.org/licenses/>.\n\n"

#define HELP_TEXT(prog_name) "\n" \
"  Usage: " << prog_name << " [OPTIONS] [--] [FILES]\n\n" \
\
"  Valid options are:\n" \
"    -a, --about  print the about dialogue\n" \
"    -h, --help   print this help dialogue\n\n" \
\
"    -f, --flag                  set the flag\n" \
"    -m, --message MESSAGE       set the message to MESSAGE\n" \
"    -s, --secret INT            set the secret to INT\n" \
"    -i, --ints INTS... [--]     set the ints to INTS\n" \
"    -c, --coords COORDS... [--] set the coords to COORDS\n\n" \
\
"  Note: A '--' before [FILES] signifies the end of the options. Any\n" \
"        options found after it will be treated as filenames.\n\n" \
\
"  Note: After INTS, you will need two '--'s, as the optional '--'\n" \
"        directly after INTS only signals the end of the INTS and\n" \
"        not the end of the options. The same goes for COORDS.\n\n" \
\
"  Note: You can have either 2 coords or 3 coords, (Let's pretend you\n" \
"        can't have 1D or 4+D coordinates for simplicity, lol.\n\n" \
\
"  Happy coding! :)\n\n"

#include <cerrno>
#include <cstdlib>
#include <iomanip>
#include <iostream>

#define LC_REQ_VER 1
#define LC_REQ_SUBVER 2

#include <libClame.hpp>
#include <libClame/generics.hpp>

/* We have some different variables to demonstrate the things that the library
 * can handle, as well as the way that someone might go about implementing
 * the library to handle them into their codebase. */

/* The flag and the message are cool because they're datatypes that we handle
 * directly in the core code: booleans and strings. */
bool flag = false;
std::string message;

/* Who doesn't like variables and arrays? */
int secret = 0;
std::list<int> ints;

/* It's the same with floats, but on top of demonstrating how arrays work, we
 * can also specify a minimum and maximum array length. Let's say you can't
 * have less than 2 dimensions or more than 3 dimensions. */
std::list<double> coords;

/* And this is to get all the args on the command line that were specified
 * without preceding flags. */
std::vector<std::string> files;

/* We want a vector of the arguments.  */
std::vector<LC_flag_t> flags;

/* We'll use a helper function for printing arrays. */
template<template<typename> typename C, typename T>
void print_arr(const std::string header, const C<T>& arr) {
	/* Check if iterator is last in a series. */
	const auto is_last = [&arr](const auto& i) {
		return &i == &*--arr.end();
	};

	std::cout << "\n  " << header << ": ";

	for(const auto& i: arr) {
		/* Don't print comma after last element in array. */
		std::cout << i << (is_last(i)? "": ", ");
	}
}

int main(int argc, char **argv) {
	/* --about, -a: prints the about dialogue. */
	flags.push_back(libClame::make_call("about", 'a', [](){
		std::cout << LICENCE_TEXT;
		std::exit(0);
	}));

	/* --help, -h: prints the help dialogue. */
	flags.push_back(libClame::make_call("help", 'h', [](){
		std::cout << HELP_TEXT(libClame::prog_name);
		std::exit(0);
	}));

	/* --flag, -f: sets the flag to true. */
	flags.push_back(libClame::make_bool("flag", 'f', flag, true));

	/* --message, -m MESSAGE: sets the message. */
	flags.push_back(libClame::make_string("message",'m', message));

	/* --secret, -s INT: set the secret number. */
	flags.push_back(libClame::make_var("secret", 's', secret));

	/* --ints, -i INTS: set the ints. */
	flags.push_back(libClame::make_arr("ints", 'i', ints));

	/* --coords, -c COORDS: set the coords. (2 or 3 values.) */
	flags.push_back(libClame::make_arr(
		"coords", 'c', coords, libClame::limits_t{2, 3})
	);

	/* Get the library to process our args and print out the help dialogue
	 * on a non-system (usage) error. If there is a memory error, then go
	 * ahead and print out an appropriate error message. */
	try {
		libClame::read(argc, argv, flags);
	}
	
	catch(libClame::exception& e) {
		switch(e.error) {
		case LC_OK: break;

		case LC_MALLOC_ERR:
			std::cout << libClame::prog_name
				<< ": error allocating memory.\n";

			std::perror(LC_prog_name); // Cstdlib error report.
			std::exit(e.error);

		default:
			std::cout << HELP_TEXT(libClame::prog_name);
			std::exit(e.error);
		}
	}

	/* Fetch the flagless arguments that the library has collected. */
	files = libClame::flagless_args;

	/* The flag is set to either true or false, so set or unset. */
	std::cout << "\n  Flag: " << (flag? "set": "unset") << "\n";

	/* Check that the message is set and print it out if it is. */
	if(!message.empty()) std::cout << "  Message: " << message << '\n';

	/* Check our secret and print it if it was set. */
	if(secret) std::cout << "  Secret: " << secret << "\n";
	
	/* Print out our arrays. */
	std::cout << std::fixed << std::setprecision(2); // Set fp precision.
	print_arr("Ints", ints);
	print_arr("Coords", coords);
	print_arr("Files", files);

	std::cout << "\n\n"; // Two extra newlines for padding.
	std::exit(0);
}
