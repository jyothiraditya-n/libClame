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

# Helper function to report errors
assert_eq() {
	if [ "$1" -eq "$2" ]; then
		echo "  -> Return value correct ($1)."
	else
		echo "  -> Incorrect return value ($1 returned, expected $2)."
		exit 1
	fi

}

# Call the args_demo program with various incorrect args to make sure the error
# reporting works correctly.

#define LC_BAD_FLAG 3 // A malformed flag was supplied to the program.
(set -x; build/args_demo --incorrect-flag > "/dev/null" 2>&1)

#define LC_VAR_RESET 4 // A variable was set twice on the command line.
(set -x; build/args_demo -m"Once" -m"Twice" > "/dev/null" 2>&1)
assert_eq "$?" "4"

#define LC_NO_VAL 5 // No value was supplied to a flag that sets a variable.
(set -x; build/args_demo --message > "/dev/null" 2>&1)
assert_eq "$?" "5"

(set -x; build/args_demo -m > "/dev/null" 2>&1)
assert_eq "$?" "5"

#define LC_BAD_VAL 6 // A malformed value was supplied.
(set -x; build/args_demo -s"Five" > "/dev/null" 2>&1)
assert_eq "$?" "6"

#define LC_LESS_VALS 7 // Fewer values were supplied than the flag accepts.
(set -x; build/args_demo -c"1.0" > "/dev/null" 2>&1)
assert_eq "$?" "7"

#define LC_MORE_VALS 8 // More values were supplied than the flag accepts.
(set -x; build/args_demo -c"1.0" "2.0" "3.0" "4.0" > "/dev/null" 2>&1)
assert_eq "$?" "8"

# Call the bad program test and make sure it runs correctly.
if (set -x; build/bad_program_test -t"5"); then
	echo "  -> All return values were correct."
else
	exit 1;
fi