#! /bin/bash

# libClame: Command-line Arguments Made Easy
# Copyright (C) 2021-2023 Jyothiraditya Nellakra
#
# This program is free software: you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free Software
# Foundation, either version 3 of the License, or (at your option) any later
# version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE. See the GNU General Public License for more 
# details.
#
# You should have received a copy of the GNU General Public License along with
# this program. If not, see <https://www.gnu.org/licenses/>.

# Format to make some lines more visible than others.
fmt="  \033[0;1m"

# Helper function to report errors
assert_eq() {
	if [ "$1" -eq "$2" ]; then
		echo -e "$fmt-> Return value correct.\033[0;0m ✓\n"
	else
		echo -e "$fmt->  Incorrect return value ($1 returned," \
			"expected $2).\033[0;0m ✘\n"
		exit 1
	fi

}

# Helper function to test both C and C++ executables
test() {

#define LC_BAD_FLAG 3 // A malformed flag was supplied to the program.

(set -x; $program -? > "/dev/null" 2>&1)
assert_eq "$?" "3"

(set -x; $program --unknown_flag > "/dev/null" 2>&1)
assert_eq "$?" "3"

#define LC_VAR_RESET 4 // A variable was set twice on the command line.

(set -x; $program -bb > "/dev/null" 2>&1)
assert_eq "$?" "4"

(set -x; $program --boolean_var --boolean_var > "/dev/null" 2>&1)
assert_eq "$?" "4"

(set -x; $program -f"Once" -f"Twice" > "/dev/null" 2>&1)
assert_eq "$?" "4"

(set -x; $program -f "Once" -f "Twice" > "/dev/null" 2>&1)
assert_eq "$?" "4"

(set -x; $program --filename_var "Once" --filename_var "Twice" \
	> "/dev/null" 2>&1)

assert_eq "$?" "4"

#define LC_NO_VAL 5 // No value was supplied to a flag that sets a variable.

(set -x; $program --filename_var > "/dev/null" 2>&1)
assert_eq "$?" "5"

(set -x; $program -f > "/dev/null" 2>&1)
assert_eq "$?" "5"

(set -x; $program --filename_var -- > "/dev/null" 2>&1)
assert_eq "$?" "0"

(set -x; $program --filename_arr > "/dev/null" 2>&1)
assert_eq "$?" "0"

(set -x; $program -f --> "/dev/null" 2>&1)
assert_eq "$?" "0"

(set -x; $program -F --> "/dev/null" 2>&1)
assert_eq "$?" "0"

(set -x; $program -f--> "/dev/null" 2>&1)
assert_eq "$?" "0"

(set -x; $program -F--> "/dev/null" 2>&1)
assert_eq "$?" "0"

#define LC_BAD_VAL 6 // A malformed value was supplied.

(set -x; $program -z"Five" > "/dev/null" 2>&1)
assert_eq "$?" "6"

(set -x; $program -z"5Five" > "/dev/null" 2>&1)
assert_eq "$?" "6"

(set -x; $program -z "Five" > "/dev/null" 2>&1)
assert_eq "$?" "6"

(set -x; $program -z "5Five" > "/dev/null" 2>&1)
assert_eq "$?" "6"

(set -x; $program --size_var "Five" > "/dev/null" 2>&1)
assert_eq "$?" "6"

(set -x; $program --size_arr "Five" > "/dev/null" 2>&1)
assert_eq "$?" "0"

(set -x; $program --size_arr "5Five" > "/dev/null" 2>&1)
assert_eq "$?" "0"

(set -x; $program -f"filename123" > "/dev/null" 2>&1)
assert_eq "$?" "6"

(set -x; $program -f "filename123" > "/dev/null" 2>&1)
assert_eq "$?" "6"

(set -x; $program --filename_var "filename123" > "/dev/null" 2>&1)
assert_eq "$?" "6"

(set -x; $program --filename_arr "filename123" > "/dev/null" 2>&1)
assert_eq "$?" "0"

#define LC_LESS_VALS 7 // Fewer values were supplied than the flag accepts.

(set -x; $program -2"1" > "/dev/null" 2>&1)
assert_eq "$?" "7"

(set -x; $program -2 "1" > "/dev/null" 2>&1)
assert_eq "$?" "7"

(set -x; $program --limited_arr "1" > "/dev/null" 2>&1)
assert_eq "$?" "7"

#define LC_MORE_VALS 8 // More values were supplied than the flag accepts.

(set -x; $program -2"1" "2" "3" > "/dev/null" 2>&1)
assert_eq "$?" "8"

(set -x; $program -2 "1" "2" "3" > "/dev/null" 2>&1)
assert_eq "$?" "8"

(set -x; $program --limited_arr "1" "2" "3" > "/dev/null" 2>&1)
assert_eq "$?" "8"

(set -x; $program -2"1" "2" "Three" > "/dev/null" 2>&1)
assert_eq "$?" "0"

(set -x; $program -2 "1" "2" "Three" > "/dev/null" 2>&1)
assert_eq "$?" "0"

(set -x; $program --limited_arr "1" "2" "Three" > "/dev/null" 2>&1)
assert_eq "$?" "0"

}

# Call the C and C++ test programs with various incorrect args to make sure the
# error reporting works correctly.
if command -v valgrind; then
	program="valgrind -q build/good_program_test"
	test

	program="valgrind -q build/good_program_cpp_test"
	test
else
	program="build/good_program_test"
	test

	program="build/good_program_cpp_test"
	test
fi

# Call the bad program test and make sure it runs correctly.
if (set -x; build/bad_program_test -t"5"); then
	echo -e "$fmt-> All return values were correct.\033[0;0m ✓\n"
else
	echo -e "$fmt-> Incorrect return values detected.\033[0;0m ✘\n"
	exit 1;
fi
