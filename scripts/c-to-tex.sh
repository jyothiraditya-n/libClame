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

# To use the script, pass the input file as $1, the output file as $2, and the
# listings file as $3.

# Produce the Latex file.
{	printf "%s" "\section{Demo Program Source Listing: "
	printf "%s\n" "\mintinline{bash}{"$1"}}"
	printf "%s\n" "\begin{minted}{c}"
	cat "$1"
	printf "\n%s\n" "\end{minted}"
}	> "$2" || exit $?

# Add the produced output as an include to a listings file.
printf "%s\n" "\input{$(basename "$2")}" >> "$3" || exit $?