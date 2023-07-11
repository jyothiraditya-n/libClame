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

# Get information for compiling C code.
read -p "CC / C Compiler> " -e -i "cc" cc
read -p "CFLAGS / C Compiler Flags (Release Mode)> " -e \
	-i "-std=c99 -Wall -Wextra -Wpedantic -s -O3 -Iinc/" cflags_release

read -p "CFLAGS / C Compiler Flags (Debug Mode)> " -e \
	-i "-std=c99 -Wall -Wextra -Werror -Wpedantic -g -Og -Iinc/" \
	cflags_debug

# Get information for linking.
read -p "LD / Linker> " -e -i "ld" ld
read -p "AR / Archiver> " -e -i "ar" ar

read -p "LD_LIBS / Linker Flags> " -e -i "-Lbuild/ -lClame" ld_libs

# Get information for latex.
latex_pre="pdflatex -interaction"
latex_post="-shell-escape main.tex"

read -p "LATEX / Latex Compilation Command (Release Mode)> " -e \
	-i "for i in 1, 2; do $latex_pre=batchmode $latex_post; done" \
	latex_release

read -p "LATEX / Latex Compilation Command (Debug Mode)> " -e \
	-i "$latex_pre=nonstopmode $latex_post" latex_debug

# Make the config folder and write the information to files.
(set -x
	mkdir -p config/

	echo "$cc" > "config/cc.conf"
	echo "$cflags_release" > "config/cflags_release.conf"
	echo "$cflags_debug" > "config/cflags_debug.conf"

	echo "$ld" > "config/ld.conf"
	echo "$ar" > "config/ar.conf"
	echo "$ld_libs" > "config/ld_libs.conf"
	echo "$latex_release" > "config/latex_release.conf"
	echo "$latex_debug" > "config/latex_debug.conf"
)