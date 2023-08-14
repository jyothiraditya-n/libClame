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

# Default values.
cc="cc"
cflags_release="-std=c99 -Wall -Wextra -Wpedantic -s -O3 -Iinc/"
cflags_debug="-std=c99 -Wall -Wextra -Werror -Wpedantic -g -Og -Iinc/"

cpp="c++"
ccflags_release="-std=c++20 -Wall -Wextra -Wpedantic -s -O2 -Iinc/"
ccflags_debug="-std=c++20 -Wall -Wextra -Werror -Wpedantic -g -Og -Iinc/"

ld="ld"
ar="ar"
ld_libs="-Lbuild/ -lClame"

latex_pre="pdflatex -interaction"
latex_post="-shell-escape main.tex"
latex_release="for i in 1, 2; do $latex_pre=batchmode $latex_post; done"
latex_debug="$latex_pre=nonstopmode $latex_post"

# Only read user input if we aren't running in auto mode.
if [ "$1" != "-auto" ]; then
	# Get information for compiling C code.
	read -p "CC / C Compiler> " -e -i "$cc" cc
	read -p "CFLAGS / C Compiler Flags (Release Mode)> " -e \
		-i "$cflags_release" cflags_release

	read -p "CFLAGS / C Compiler Flags (Debug Mode)> " -e \
		-i "$cflags_debug" cflags_debug

	read -p "CPP / C++ Compiler> " -e -i "$cpp" cpp
	read -p "CCFLAGS / C++ Compiler Flags (Release Mode)> " -e \
		-i "$cppflags_release" cppflags_release

	read -p "CPPFLAGS / C++ Compiler Flags (Debug Mode)> " -e \
		-i "$cppflags_debug" cppflags_debug

	# Get information for linking.
	read -p "LD / Linker> " -e -i "$ld" ld
	read -p "AR / Archiver> " -e -i "$ar" ar

	read -p "LD_LIBS / Linker Flags> " -e -i "$ld_libs" ld_libs

	# Get information for latex.

	read -p "LATEX / Latex Compilation Command (Release Mode)> " -e \
		-i "$latex_release" latex_release

	read -p "LATEX / Latex Compilation Command (Debug Mode)> " -e \
		-i "$latex_debug" latex_debug
fi

# Make the config folder and write the information to files.
mkdir -p .config/

echo "$cc" > ".config/cc.conf"
echo "$cflags_release" > ".config/cflags_release.conf"
echo "$cflags_debug" > ".config/cflags_debug.conf"

echo "$cpp" > ".config/cpp.conf"
echo "$ccflags_release" > ".config/ccflags_release.conf"
echo "$ccflags_debug" > ".config/ccflags_debug.conf"

echo "$ld" > ".config/ld.conf"
echo "$ar" > ".config/ar.conf"
echo "$ld_libs" > ".config/ld_libs.conf"

echo "$latex_release" > ".config/latex_release.conf"
echo "$latex_debug" > ".config/latex_debug.conf"
