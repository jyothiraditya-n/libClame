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

# Helper function to test both C and C++ executables
test() {

# Check callback function.
output=$(set -x; $program -c)
assert_eq "$output" "custom_callback(); ..."

# Try setting all the variables individually.
output=$(set -x; $program -b)
assert_eq "$output" "boolean_var = true; ..."

output=$(set -x; $program --boolean_var)
assert_eq "$output" "boolean_var = true; ..."

output=$(set -x; $program -!)
assert_eq "$output" "custom_callback(); boolean_var = true; ..."

output=$(set -x; $program --boolean_callback)
assert_eq "$output" "custom_callback(); boolean_var = true; ..."

output=$(set -x; $program -s"testing")
assert_eq "$output" "string_var = \"testing\"; ..."

output=$(set -x; $program -s "testing")
assert_eq "$output" "string_var = \"testing\"; ..."

output=$(set -x; $program --string_var "testing")
assert_eq "$output" "string_var = \"testing\"; ..."

output=$(set -x; $program -S"hello" "world")
assert_eq "$output" "string_arr = {\"hello\", \"world\", ...}; ..."

output=$(set -x; $program -S "hello" "world")
assert_eq "$output" "string_arr = {\"hello\", \"world\", ...}; ..."

output=$(set -x; $program --string_arr "hello" "world")
assert_eq "$output" "string_arr = {\"hello\", \"world\", ...}; ..."

output=$(set -x; $program -i"12345")
assert_eq "$output" "int_var = 12345; ..."

output=$(set -x; $program -i "12345")
assert_eq "$output" "int_var = 12345; ..."

output=$(set -x; $program --int_var "12345")
assert_eq "$output" "int_var = 12345; ..."

output=$(set -x; $program -I"123" "456")
assert_eq "$output" "int_arr = {123, 456, ...}; ..."

output=$(set -x; $program -I "123" "456")
assert_eq "$output" "int_arr = {123, 456, ...}; ..."

output=$(set -x; $program --int_arr "123" "456")
assert_eq "$output" "int_arr = {123, 456, ...}; ..."

output=$(set -x; $program -d"3.14")
assert_eq "$output" "double_var = 3.14; ..."

output=$(set -x; $program -d "3.14")
assert_eq "$output" "double_var = 3.14; ..."

output=$(set -x; $program --double_var "3.14")
assert_eq "$output" "double_var = 3.14; ..."

output=$(set -x; $program -D"3.14" "2.72")
assert_eq "$output" "double_arr = {3.14, 2.72, ...}; ..."

output=$(set -x; $program -D "3.14" "2.72")
assert_eq "$output" "double_arr = {3.14, 2.72, ...}; ..."

output=$(set -x; $program --double_arr "3.14" "2.72")
assert_eq "$output" "double_arr = {3.14, 2.72, ...}; ..."

output=$(set -x; $program -z"12345")
assert_eq "$output" "size_var = 12345; ..."

output=$(set -x; $program -z "12345")
assert_eq "$output" "size_var = 12345; ..."

output=$(set -x; $program --size_var "12345")
assert_eq "$output" "size_var = 12345; ..."

output=$(set -x; $program -Z"123" "456")
assert_eq "$output" "size_arr = {123, 456, ...}; ..."

output=$(set -x; $program -Z "123" "456")
assert_eq "$output" "size_arr = {123, 456, ...}; ..."

output=$(set -x; $program --size_arr "123" "456")
assert_eq "$output" "size_arr = {123, 456, ...}; ..."

output=$(set -x; $program -o"12")
assert_eq "$output" "oct_var = 12; ..."

output=$(set -x; $program -o "12")
assert_eq "$output" "oct_var = 12; ..."

output=$(set -x; $program --oct_var "12")
assert_eq "$output" "oct_var = 12; ..."

output=$(set -x; $program -O"12" "45")
assert_eq "$output" "oct_arr = {12, 45, ...}; ..."

output=$(set -x; $program -O "12" "45")
assert_eq "$output" "oct_arr = {12, 45, ...}; ..."

output=$(set -x; $program --oct_arr "12" "45")
assert_eq "$output" "oct_arr = {12, 45, ...}; ..."

output=$(set -x; $program -x"de")
assert_eq "$output" "hex_var = de; ..."

output=$(set -x; $program -x "de")
assert_eq "$output" "hex_var = de; ..."

output=$(set -x; $program --hex_var "de")
assert_eq "$output" "hex_var = de; ..."

output=$(set -x; $program -X"de" "ad" "be" "ef")
assert_eq "$output" "hex_arr = {de, ad, be, ef, ...}; ..."

output=$(set -x; $program -X "de" "ad" "be" "ef")
assert_eq "$output" "hex_arr = {de, ad, be, ef, ...}; ..."

output=$(set -x; $program --hex_arr "de" "ad" "be" "ef")
assert_eq "$output" "hex_arr = {de, ad, be, ef, ...}; ..."

output=$(set -x; $program -f"filename")
assert_eq "$output" "filename_var = \"filename\"; ..."

output=$(set -x; $program -f "filename")
assert_eq "$output" "filename_var = \"filename\"; ..."

output=$(set -x; $program --filename_var "filename")
assert_eq "$output" "filename_var = \"filename\"; ..."

output=$(set -x; $program -F"hello" "world")
assert_eq "$output" "filename_arr = {\"hello\", \"world\", ...}; ..."

output=$(set -x; $program -F "hello" "world")
assert_eq "$output" "filename_arr = {\"hello\", \"world\", ...}; ..."

output=$(set -x; $program --filename_arr "hello" "world")
assert_eq "$output" "filename_arr = {\"hello\", \"world\", ...}; ..."

output=$(set -x; $program -F"hello" "world")
assert_eq "$output" "filename_arr = {\"hello\", \"world\", ...}; ..."

output=$(set -x; $program -F "hello" "world")
assert_eq "$output" "filename_arr = {\"hello\", \"world\", ...}; ..."

output=$(set -x; $program -2"123" "45")
assert_eq "$output" "limited_arr = {123, 45, ...}; ..."

output=$(set -x; $program -2 "123" "45")
assert_eq "$output" "limited_arr = {123, 45, ...}; ..."

output=$(set -x; $program --limited_arr "123" "45")
assert_eq "$output" "limited_arr = {123, 45, ...}; ..."

output=$(set -x; $program -@"123" "45")
assert_eq "$output" "custom_callback(); limited_arr = {123, 45, ...}; ..."

output=$(set -x; $program -@ "123" "45")
assert_eq "$output" "custom_callback(); limited_arr = {123, 45, ...}; ..."

output=$(set -x; $program --limited_callback "123" "45")
assert_eq "$output" "custom_callback(); limited_arr = {123, 45, ...}; ..."

output=$(set -x; $program "hello" "world")
assert_eq "$output" "flagless_args = {\"hello\", \"world\", ...}; ..."

# Checking the combination of flags.

output=$(set -x; $program -cb)
assert_eq "$output" "custom_callback(); boolean_var = true; ..."

output=$(set -x; $program -bs"testing")
assert_eq "$output" "boolean_var = true; string_var = \"testing\"; ..."

output=$(set -x; $program -bs "testing")
assert_eq "$output" "boolean_var = true; string_var = \"testing\"; ..."

output=$(set -x; $program -s "testing" -b)
assert_eq "$output" "boolean_var = true; string_var = \"testing\"; ..."

output=$(set -x; $program -S "testing" -b)
assert_eq "$output" "string_arr = {\"testing\", \"-b\", ...}; ..."

output=$(set -x; $program -S "testing" -- -b)
assert_eq "$output" "boolean_var = true; string_arr = {\"testing\", ...}; ..."

output=$(set -x; $program -S "testing" -- -- -b)

assert_eq "$output" \
"string_arr = {\"testing\", ...}; flagless_args = {\"-b\", ...}; ..."

output=$(set -x; $program -i "123" -b)
assert_eq "$output" "boolean_var = true; int_var = 123; ..."

output=$(set -x; $program -I "123" -b)
assert_eq "$output" "boolean_var = true; int_arr = {123, ...}; ..."

output=$(set -x; $program -I "123" -- -b)
assert_eq "$output" "boolean_var = true; int_arr = {123, ...}; ..."

output=$(set -x; $program -I "123" -- -- -b)
assert_eq "$output" \
	"int_arr = {123, ...}; flagless_args = {\"-b\", ...}; ..."

output=$(set -x; $program -f "filename" --boolean_var)
assert_eq "$output" "boolean_var = true; filename_var = \"filename\"; ..."

output=$(set -x; $program -F "filename" --boolean_var)
assert_eq "$output" \
	"boolean_var = true; filename_arr = {\"filename\", ...}; ..."

output=$(set -x; $program  -F "filename" -- --boolean_var)
assert_eq "$output" \
	"boolean_var = true; filename_arr = {\"filename\", ...}; ..."

output=$(set -x; $program -F "filename" -- -- --boolean_var)

assert_eq "$output" \
"filename_arr = {\"filename\", ...}; \
flagless_args = {\"--boolean_var\", ...}; ..."

# Check multi-callback chaining.

output=$(set -x; $program -c!@"123" "45")
many="custom_callback(); custom_callback(); custom_callback(); boolean_var"
assert_eq "$output" "$many = true; limited_arr = {123, 45, ...}; ..."

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
