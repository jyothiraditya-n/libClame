# libClame Version 1.2: Command-Line Arguments Made Easy
by Jyothiraditya Nellakra

![workflow status](https://github.com/jyothiraditya-n/libClame/actions/workflows/c-cpp.yml/badge.svg)

# About

This library is a C/C++ language implementation of a parser for command-line arguments passed in a UNIX-like syntax. It should be flexible enough to meet most of your needs, along with extensibility through function callbacks to user-specied code.

## Contributing

Please report bugs and submit your ideas for improvements either to my email ID <jyothiraditya.n@gmail.com> or at the GitHub repository <https://github.com/jyothiraditya-n/libClame>. 

## Licence Disclaimer

libClame: Command-line Arguments Made Easy Copyright (C) 2021-2023 Jyothiraditya Nellakra

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later  version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.

# Compiling, Building & Linking

This library is linked statically into your program binary, with the `.o` object files being held in the `libClame.a` archive file. This file can be passed to the linker `ld` (or your C compiler `cc`, which then sends it to `ld`) by use of the `-L<dir> -lClame` flags, where `dir` is the directory of the compiled binary. As for the header files, they are stored within the `inc/` folder.

## Prerequisite Build Tools

In order to compile the library, you will need a working copy of a C compiler that supports the C99 standard and GNU Make. This will probably get you both a linker and an archiver on most any version of Unix. Thereafter, to compile the latex documentation, you will need Latex, a working copy of the `minted` Latex package and its requisite copy of Python `pygments`, as well as the following Latex libraries:

- `geometry`, used for setting the page layout.
- `hyperref`, used for URLs.
- `tabulary` and `booktabs`, used for dynamic tables.

## Automatic Configuration & Compilation With Make

The compilation of this library and its associated documentation and example programs is handled through GNU Make. After downloading the library, running `make config` will allow you to configure the build options. The prompts will be auto-populated with what should hopefully be sane defaults, so you can continue through just by pressing enter if you don't wish to change anything.

Once configured, you can create a release binary for the archive by running `make` or `make release`. A development binary with debugging symbols can be made with `make debug`. In either case, the output file is located in the `build/` folder.

Similarly, you can compile the documentation for the project with `make release docs`, which will produce a PDF file at `build/libClame.pdf`. Running `make demos` will also produce the demo program binaries in the `build/` folder.

Testing is done by running `make test`. And you can clean up all the build and configuration files by running `make clean` and `make deep-clean`, respectively.
