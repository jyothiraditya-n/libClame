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

# Call the cpp_test program with various incorrect args to make sure the error
# reporting works correctly.

#define LC_BAD_FLAG 3 // A malformed flag was supplied to the program.
(set -x; build/good_program_cpp_test -? > "/dev/null" 2>&1)
assert_eq "$?" "3"

(set -x; build/good_program_cpp_test --unknown_flag > "/dev/null" 2>&1)
assert_eq "$?" "3"

#define LC_VAR_RESET 4 // A variable was set twice on the command line.
(set -x; build/good_program_cpp_test -bb > "/dev/null" 2>&1)
assert_eq "$?" "4"

(set -x; build/good_program_cpp_test --boolean_var --boolean_var > \
	"/dev/null" 2>&1)

	assert_eq "$?" "4"

(set -x; build/good_program_cpp_test -f"Once" -f"Twice" > "/dev/null" 2>&1)
assert_eq "$?" "4"

(set -x; build/good_program_cpp_test -f "Once" -f "Twice" > "/dev/null" 2>&1)
assert_eq "$?" "4"

(set -x; build/good_program_cpp_test --filename_var "Once" --filename_var \
	"Twice" > "/dev/null" 2>&1)

assert_eq "$?" "4"

#define LC_NO_VAL 5 // No value was supplied to a flag that sets a variable.
(set -x; build/good_program_cpp_test --filename_var > "/dev/null" 2>&1)
assert_eq "$?" "5"

(set -x; build/good_program_cpp_test -f > "/dev/null" 2>&1)
assert_eq "$?" "5"

(set -x; build/good_program_cpp_test --filename_var -- > "/dev/null" 2>&1)
assert_eq "$?" "0"

(set -x; build/good_program_cpp_test --filename_arr > "/dev/null" 2>&1)
assert_eq "$?" "0"

(set -x; build/good_program_cpp_test -f --> "/dev/null" 2>&1)
assert_eq "$?" "0"

(set -x; build/good_program_cpp_test -F --> "/dev/null" 2>&1)
assert_eq "$?" "0"

(set -x; build/good_program_cpp_test -f--> "/dev/null" 2>&1)
assert_eq "$?" "0"

(set -x; build/good_program_cpp_test -F--> "/dev/null" 2>&1)
assert_eq "$?" "0"

#define LC_BAD_VAL 6 // A malformed value was supplied.
(set -x; build/good_program_cpp_test -z"Five" > "/dev/null" 2>&1)
assert_eq "$?" "6"

(set -x; build/good_program_cpp_test -z"5Five" > "/dev/null" 2>&1)
assert_eq "$?" "6"

(set -x; build/good_program_cpp_test -z "Five" > "/dev/null" 2>&1)
assert_eq "$?" "6"

(set -x; build/good_program_cpp_test -z "5Five" > "/dev/null" 2>&1)
assert_eq "$?" "6"

(set -x; build/good_program_cpp_test --size_var "Five" > "/dev/null" 2>&1)
assert_eq "$?" "6"

(set -x; build/good_program_cpp_test --size_arr "Five" > "/dev/null" 2>&1)
assert_eq "$?" "0"

(set -x; build/good_program_cpp_test --size_arr "5Five" > "/dev/null" 2>&1)
assert_eq "$?" "0"

(set -x; build/good_program_cpp_test -f"filename123" > "/dev/null" 2>&1)
assert_eq "$?" "6"

(set -x; build/good_program_cpp_test -f "filename123" > "/dev/null" 2>&1)
assert_eq "$?" "6"

(set -x; build/good_program_cpp_test --filename_var "filename123" > \
	"/dev/null" 2>&1)

assert_eq "$?" "6"

(set -x; build/good_program_cpp_test --filename_arr "filename123" > \
	"/dev/null" 2>&1)

assert_eq "$?" "0"

#define LC_LESS_VALS 7 // Fewer values were supplied than the flag accepts.
(set -x; build/good_program_cpp_test -2"1" > "/dev/null" 2>&1)
assert_eq "$?" "7"

(set -x; build/good_program_cpp_test -2 "1" > "/dev/null" 2>&1)
assert_eq "$?" "7"

(set -x; build/good_program_cpp_test --limited_arr "1" > "/dev/null" 2>&1)
assert_eq "$?" "7"

#define LC_MORE_VALS 8 // More values were supplied than the flag accepts.
(set -x; build/good_program_cpp_test -2"1" "2" "3" > "/dev/null" 2>&1)
assert_eq "$?" "8"

(set -x; build/good_program_cpp_test -2 "1" "2" "3" > "/dev/null" 2>&1)
assert_eq "$?" "8"

(set -x; build/good_program_cpp_test --limited_arr "1" "2" "3" > \
	"/dev/null" 2>&1)

assert_eq "$?" "8"

(set -x; build/good_program_cpp_test -2"1" "2" "Three" > "/dev/null" 2>&1)
assert_eq "$?" "0"

(set -x; build/good_program_cpp_test -2 "1" "2" "Three" > "/dev/null" 2>&1)
assert_eq "$?" "0"

(set -x; build/good_program_cpp_test --limited_arr "1" "2" "Three" > \
	"/dev/null" 2>&1)

assert_eq "$?" "0"
