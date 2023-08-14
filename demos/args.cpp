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

#include <cerrno>
#include <iostream>

#define LC_REQ_VER 1
#define LC_REQ_SUBVER 2

#include <libClame.hpp>
#include <libClame/templates.hpp>

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

/* It's the same with floats, but on top of demonstrating how arrays work,
 * we'll also specify a minimum and maximum array length. */
std::list<double> coords;

/* And this is to get all the args on the command line that were specified
 * without preceding flags. */
std::vector<std::string> files;

/* These helper functions are specified as part of the arguments structure. */
void about_flag();
void help_flag();

/* Whether we're printing the help and exiting normally or printing the help
 * because we encountered an error, we are basically printing the same thing so
 * it's useful to have the code shared in this function. */
void help_and_return(int ret);

/* We want a vector of the arguments.  */
std::vector<LC_flag_t> flags;

int main(int argc, char **argv) {
	/* --about, -a: prints the about dialogue. */
	flags.push_back(libClame::make_call("about", 'a', about_flag));

	/* --help, -h: prints the help dialogue. */
	flags.push_back(libClame::make_call("help", 'h', help_flag));

	/* --flag, -f: sets the flag to true. */
	flags.push_back(
		libClame::make_bool("flag", 'f', flag, true, {})
	);

	/* --message, -m MESSAGE: sets the message. */
	flags.push_back(libClame::make_string("message",'m', message, {}));

	/* --secret, -s INT: set the secret number. */
	flags.push_back(libClame::make_var("secret", 's', secret, {}, {}));

	/* --ints, -i INTS: set the ints. */
	flags.push_back(libClame::make_arr("ints", 'i', ints, {}, {}, {}));

	/* --coords, -c COORDS: set the coords. (2 or 3 values.) */
	flags.push_back(libClame::make_arr(
		"coords", 'c', coords, libClame::limits_t{2, 3}, {}, {}
	));

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

			std::perror(LC_prog_name); // More precise report.
			break;

		default:
			help_and_return(e.error);
		}
	}

	/* Fetch the flagless arguments that the library has collected for us
	 * along with the count of how many there are. */
	files = libClame::flagless_args;

	/* The flag is set to either true or false, so set or unset. The
	 * message is just a boring old string. */
	std::cout << "\n  Flag: " << (flag? "set": "unset") << "\n";

	/* Check that the message is set and print it out if it is. */
	if(message != "") std::cout << "  Message: " << message << '\n';

	/* Check our secret. */
	if(secret) std::cout << "  Secret: " << secret << "\n";
	
	/* The integers, coords and files all follow the same pattern, where
	 * we just want to print them out as a list following a header and then
	 * put a couple new lines after them to pad it before the next set. */

	std::cout << "\n  Ints: ";
	for(const auto& i: ints) std::cout << i << ", ";

	std::cout << "\n  Coords: ";
	for(const auto& i: coords) std::cout << i << ", ";

	std::cout << "\n  Files: ";
	for(const auto& i: files) std::cout << i << ", ";

	std::cout << "\n\n";
	exit(0);
}

void about_flag() {
	putchar('\n');
	puts("  libClame: Command-line Arguments Made Easy");
	puts("  Copyright (C) 2021-2023 Jyothiraditya Nellakra");
	puts("  Demonstration Program for <LC_args.h>\n");

	puts("  This program is free software: you can redistribute it and/or modify");
	puts("  it under the terms of the GNU General Public License as published by");
	puts("  the Free Software Foundation, either version 3 of the License, or");
	puts("  (at your option) any later version.\n");

	puts("  This program is distributed in the hope that it will be useful,");
	puts("  but WITHOUT ANY WARRANTY; without even the implied warranty of");
	puts("  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the");
	puts("  GNU General Public License for more details.\n");

	puts("  You should have received a copy of the GNU General Public License");
	puts("  along with this program. If not, see <https://www.gnu.org/licenses/>.\n");

	exit(0);
}

void help_flag() {
	help_and_return(0);
}

void help_and_return(int ret) {
	putchar('\n');
	printf("  Usage: %s [OPTIONS] [--] [FILES]\n\n", LC_prog_name);

	puts("  Valid options are:");
	puts("    -a, --about  print the about dialogue");
	puts("    -h, --help   print this help dialogue\n");

	puts("    -f, --flag                  set the flag");
	puts("    -m, --message MESSAGE       set the message to MESSAGE");
	puts("    -s, --secret INT            set the secret to INT");
	puts("    -i, --ints INTS... [--]     set the ints to INTS");
	puts("    -c, --coords COORDS... [--] set the coords to COORDS\n");

	puts("  Note: A '--' before [FILES] signifies the end of the options. Any");
	puts("        options found after it will be treated as filenames.\n");

	puts("  Note: After INTS, you will need two '--'s, as the optional '--'");
	puts("        directly after INTS only signals the end of the INTS and");
	puts("        not the end of the options. The same goes for COORDS.\n");

	puts("  Note: You can have either 2 coords or 3 coords, (Let's pretend you");
	puts("        can't have 1D or 4+D coordinates for simplicity, lol.\n");

	puts("  Happy coding! :)\n");
	exit(ret);
}
