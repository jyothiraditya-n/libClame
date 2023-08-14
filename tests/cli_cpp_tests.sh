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
fmt="   \033[0;1m"

# Helper function to report errors
assert_eq() {
	if [ "$?" -ne "0" ]; then
		echo -e "$fmt-> Program crashed.\033[0;0m ✘\n"
		exit 1
	fi

	if [ "$1" = "$2" ]; then
		echo -e "$fmt-> Output correct.\033[0;0m ✓\n"
	else
		echo -e "$fmt->  Incorrect output (\"$1\" returned," \
			"expected \"$2\").\033[0;0m ✘\n"
		exit 1
	fi
}

# Check callback function.

output=$(set -x; build/good_program_cpp_test -c)
assert_eq "$output" "custom_callback(); ..."

# Try setting all the variables individually.

output=$(set -x; build/good_program_cpp_test -b)
assert_eq "$output" "boolean_var = true; ..."

output=$(set -x; build/good_program_cpp_test --boolean_var)
assert_eq "$output" "boolean_var = true; ..."

output=$(set -x; build/good_program_cpp_test -s"testing")
assert_eq "$output" "string_var = \"testing\"; ..."

output=$(set -x; build/good_program_cpp_test -s "testing")
assert_eq "$output" "string_var = \"testing\"; ..."

output=$(set -x; build/good_program_cpp_test --string_var "testing")
assert_eq "$output" "string_var = \"testing\"; ..."

output=$(set -x; build/good_program_cpp_test -S"hello" "world")
assert_eq "$output" "string_arr = {\"hello\", \"world\", ...}; ..."

output=$(set -x; build/good_program_cpp_test -S "hello" "world")
assert_eq "$output" "string_arr = {\"hello\", \"world\", ...}; ..."

output=$(set -x; build/good_program_cpp_test --string_arr "hello" "world")
assert_eq "$output" "string_arr = {\"hello\", \"world\", ...}; ..."

output=$(set -x; build/good_program_cpp_test -i"12345")
assert_eq "$output" "int_var = 12345; ..."

output=$(set -x; build/good_program_cpp_test -i "12345")
assert_eq "$output" "int_var = 12345; ..."

output=$(set -x; build/good_program_cpp_test --int_var "12345")
assert_eq "$output" "int_var = 12345; ..."

output=$(set -x; build/good_program_cpp_test -I"123" "456")
assert_eq "$output" "int_arr = {123, 456, ...}; ..."

output=$(set -x; build/good_program_cpp_test -I "123" "456")
assert_eq "$output" "int_arr = {123, 456, ...}; ..."

output=$(set -x; build/good_program_cpp_test --int_arr "123" "456")
assert_eq "$output" "int_arr = {123, 456, ...}; ..."

output=$(set -x; build/good_program_cpp_test -d"3.14")
assert_eq "$output" "double_var = 3.14; ..."

output=$(set -x; build/good_program_cpp_test -d "3.14")
assert_eq "$output" "double_var = 3.14; ..."

output=$(set -x; build/good_program_cpp_test --double_var "3.14")
assert_eq "$output" "double_var = 3.14; ..."

output=$(set -x; build/good_program_cpp_test -D"3.14" "2.72")
assert_eq "$output" "double_arr = {3.14, 2.72, ...}; ..."

output=$(set -x; build/good_program_cpp_test -D "3.14" "2.72")
assert_eq "$output" "double_arr = {3.14, 2.72, ...}; ..."

output=$(set -x; build/good_program_cpp_test --double_arr "3.14" "2.72")
assert_eq "$output" "double_arr = {3.14, 2.72, ...}; ..."

output=$(set -x; build/good_program_cpp_test -z"12345")
assert_eq "$output" "size_var = 12345; ..."

output=$(set -x; build/good_program_cpp_test -z "12345")
assert_eq "$output" "size_var = 12345; ..."

output=$(set -x; build/good_program_cpp_test --size_var "12345")
assert_eq "$output" "size_var = 12345; ..."

output=$(set -x; build/good_program_cpp_test -Z"123" "456")
assert_eq "$output" "size_arr = {123, 456, ...}; ..."

output=$(set -x; build/good_program_cpp_test -Z "123" "456")
assert_eq "$output" "size_arr = {123, 456, ...}; ..."

output=$(set -x; build/good_program_cpp_test --size_arr "123" "456")
assert_eq "$output" "size_arr = {123, 456, ...}; ..."

output=$(set -x; build/good_program_cpp_test -o"12")
assert_eq "$output" "oct_var = 12; ..."

output=$(set -x; build/good_program_cpp_test -o "12")
assert_eq "$output" "oct_var = 12; ..."

output=$(set -x; build/good_program_cpp_test --oct_var "12")
assert_eq "$output" "oct_var = 12; ..."

output=$(set -x; build/good_program_cpp_test -O"12" "45")
assert_eq "$output" "oct_arr = {12, 45, ...}; ..."

output=$(set -x; build/good_program_cpp_test -O "12" "45")
assert_eq "$output" "oct_arr = {12, 45, ...}; ..."

output=$(set -x; build/good_program_cpp_test --oct_arr "12" "45")
assert_eq "$output" "oct_arr = {12, 45, ...}; ..."

output=$(set -x; build/good_program_cpp_test -x"de")
assert_eq "$output" "hex_var = de; ..."

output=$(set -x; build/good_program_cpp_test -x "de")
assert_eq "$output" "hex_var = de; ..."

output=$(set -x; build/good_program_cpp_test --hex_var "de")
assert_eq "$output" "hex_var = de; ..."

output=$(set -x; build/good_program_cpp_test -X"de" "ad" "be" "ef")
assert_eq "$output" "hex_arr = {de, ad, be, ef, ...}; ..."

output=$(set -x; build/good_program_cpp_test -X "de" "ad" "be" "ef")
assert_eq "$output" "hex_arr = {de, ad, be, ef, ...}; ..."

output=$(set -x; build/good_program_cpp_test --hex_arr "de" "ad" "be" "ef")
assert_eq "$output" "hex_arr = {de, ad, be, ef, ...}; ..."

output=$(set -x; build/good_program_cpp_test -f"filename")
assert_eq "$output" "filename_var = \"filename\"; ..."

output=$(set -x; build/good_program_cpp_test -f "filename")
assert_eq "$output" "filename_var = \"filename\"; ..."

output=$(set -x; build/good_program_cpp_test --filename_var "filename")
assert_eq "$output" "filename_var = \"filename\"; ..."

output=$(set -x; build/good_program_cpp_test -F"hello" "world")
assert_eq "$output" "filename_arr = {\"hello\", \"world\", ...}; ..."

output=$(set -x; build/good_program_cpp_test -F "hello" "world")
assert_eq "$output" "filename_arr = {\"hello\", \"world\", ...}; ..."

output=$(set -x; build/good_program_cpp_test --filename_arr "hello" "world")
assert_eq "$output" "filename_arr = {\"hello\", \"world\", ...}; ..."


output=$(set -x; build/good_program_cpp_test -F"hello" "world")
assert_eq "$output" "filename_arr = {\"hello\", \"world\", ...}; ..."

output=$(set -x; build/good_program_cpp_test -F "hello" "world")
assert_eq "$output" "filename_arr = {\"hello\", \"world\", ...}; ..."

output=$(set -x; build/good_program_cpp_test -2"123" "45")
assert_eq "$output" "limited_arr = {123, 45, ...}; ..."

output=$(set -x; build/good_program_cpp_test -2 "123" "45")
assert_eq "$output" "limited_arr = {123, 45, ...}; ..."

output=$(set -x; build/good_program_cpp_test --limited_arr "123" "45")
assert_eq "$output" "limited_arr = {123, 45, ...}; ..."

output=$(set -x; build/good_program_cpp_test "hello" "world")
assert_eq "$output"\
	"libClame::flagless_args = {\"hello\", \"world\", ...}; ..."

# Checking the combination of flags.

output=$(set -x; build/good_program_cpp_test -cb)
assert_eq "$output" "custom_callback(); boolean_var = true; ..."

output=$(set -x; build/good_program_cpp_test -bs"testing")
assert_eq "$output" "boolean_var = true; string_var = \"testing\"; ..."

output=$(set -x; build/good_program_cpp_test -bs "testing")
assert_eq "$output" "boolean_var = true; string_var = \"testing\"; ..."

output=$(set -x; build/good_program_cpp_test -s "testing" -b)
assert_eq "$output" "boolean_var = true; string_var = \"testing\"; ..."

output=$(set -x; build/good_program_cpp_test -S "testing" -b)
assert_eq "$output" "string_arr = {\"testing\", \"-b\", ...}; ..."

output=$(set -x; build/good_program_cpp_test -S "testing" -- -b)
assert_eq "$output" "boolean_var = true; string_arr = {\"testing\", ...}; ..."

output=$(set -x; build/good_program_cpp_test -S "testing" -- -- -b)

assert_eq "$output" \
"string_arr = {\"testing\", ...}; libClame::flagless_args = {\"-b\", ...}; ..."

output=$(set -x; build/good_program_cpp_test -i "123" -b)
assert_eq "$output" "boolean_var = true; int_var = 123; ..."

output=$(set -x; build/good_program_cpp_test -I "123" -b)
assert_eq "$output" "boolean_var = true; int_arr = {123, ...}; ..."

output=$(set -x; build/good_program_cpp_test -I "123" -- -b)
assert_eq "$output" "boolean_var = true; int_arr = {123, ...}; ..."

output=$(set -x; build/good_program_cpp_test -I "123" -- -- -b)
assert_eq "$output" \
	"int_arr = {123, ...}; libClame::flagless_args = {\"-b\", ...}; ..."

output=$(set -x; build/good_program_cpp_test -f "filename" --boolean_var)
assert_eq "$output" "boolean_var = true; filename_var = \"filename\"; ..."

output=$(set -x; build/good_program_cpp_test -F "filename" --boolean_var)
assert_eq "$output" \
	"boolean_var = true; filename_arr = {\"filename\", ...}; ..."

output=$(set -x; build/good_program_cpp_test  -F "filename" -- --boolean_var)
assert_eq "$output" \
	"boolean_var = true; filename_arr = {\"filename\", ...}; ..."

output=$(set -x; build/good_program_cpp_test -F "filename" -- -- --boolean_var)

assert_eq "$output" \
"filename_arr = {\"filename\", ...}; \
libClame::flagless_args = {\"--boolean_var\", ...}; ..."

