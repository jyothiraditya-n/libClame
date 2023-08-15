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

# Produce a Latex output file, which quotes the input file in a minted code
# block. Then, add that output file to a listings file.

# Get information on oppsies in your code, pass the files you want to check via
# $@.

# Check that the configuration folder exists.
if [ ! -d ".config/" ]; then
	echo "### missing configuration. you need to run 'make config'."
	exit 1
fi

# Get some configuration data.
cflags="$(cat ".config/cflags_debug.conf")"
ccflags="$(cat ".config/ccflags_debug.conf")"

# We use clang for this even though I generally compile with gcc because that
# increases the change that nasty bugs will get caught by one set of warnings
# or another.

for i in "$@"; do
	case "$i" in
	*.c) (set -x; clang-tidy "$i" -- $cflags);;
	*.cpp) (set -x; clang-tidy "$i" -- $ccflags);;

	*.h) (set -x; clang-tidy "$i" -- $cflags);;
	*.hpp) (set -x; clang-tidy "$i" -- $ccflags);;
	esac
done
