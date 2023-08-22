#! /bin/bash

# Amocao C/C++ Build System Copyright (C) 2021-2023 Jyothiraditya Nellakra
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

# Helper function for printing errors.
printerr() { printf "$0: Error: %s\n" "$*" >&2; }

# Load the default variable configurations stored in defaults.conf. Error out
# if the file does not exist but automatic configuration is specified.
if [ "$1" = "--auto" ] && [ ! -f "defaults.conf" ]; then
	printerr "Can't run autoconfig without a valid defaults.conf file."
	exec "$0" # Without autoconfigure option.

elif [ -f "defaults.conf" ]; then
	source <(grep "=" "defaults.conf")
fi

# Only read user input if we aren't running in auto mode.
if [ "$1" != "--auto" ]; then
	# Get basic program information.
	read -p "NAME / Project name> " -e -i "$name" name

	# Get information for compiling C code.
	read -p "CC / C Compiler> " -e -i "$cc" cc
	read -p "CFLAGS / C Compiler Flags (Release Mode)> " -e \
		-i "$cflags_release" cflags_release

	read -p "CFLAGS / C Compiler Flags (Debug Mode)> " -e \
		-i "$cflags_debug" cflags_debug

	# Get information for compiling C++ code.
	read -p "CXX / C++ Compiler> " -e -i "$cxx" cxx
	read -p "CCFLAGS / C++ Compiler Flags (Release Mode)> " -e \
		-i "$ccflags_release" ccflags_release

	read -p "CCFLAGS / C++ Compiler Flags (Debug Mode)> " -e \
		-i "$ccflags_debug" ccflags_debug

	# Get information for linking.
	read -p "LD / Linker> " -e -i "$ld" ld
	read -p "AR / Archiver> " -e -i "$ar" ar
	read -p "LD_LIBS / Linker Flags> " -e -i "$ld_libs" ld_libs

	# Get a list of addditional libraries to include in our build.
	read -p "LIBS / Additional Libs to Build> " -e -i "${libs[@]}" libs;
	libs=($libs) # Turn the string into an array.

	# Get information for the tests.
	read -p "TEST_FLAGS_RELEASE / Testing Flags (Release Mode)> " -e \
		-i "$test_flags_release" test_flags_release

	read -p "TEST_FLAGS_DEBUG / Testing Flags (Debug Mode)> " -e \
		-i "$test_flags_debug" test_flags_debug
fi

{ # Write the config to the makefile header file, overwriting it.
	echo "NAME = $name"
	echo ""
	echo "CC = $cc"
	echo "CFLAGS_RELEASE = $cflags_release"
	echo "CFLAGS_DEBUG = $cflags_debug"
	echo ""
	echo "CXX = $cxx"
	echo "CCFLAGS_RELEASE = $ccflags_release"
	echo "CCFLAGS_DEBUG = $ccflags_debug"
	echo ""
	echo "LD = $ld"
	echo "AR = $ar"
	echo "LD_LIBS = $ld_libs"
	echo ""
	echo "LIBS = ${libs[@]}"
	echo ""
	echo "TEST_FLAGS_RELEASE = $test_flags_release"
	echo "TEST_FLAGS_DEBUG = $test_flags_debug"
	echo ""

} > ".config.mk"

# Generate the associative arrays for the libraries we need to build.

declare -A make_release # Commands to make release builds of libraries.
declare -A make_debug # Commands to make debug builds of libraries.
declare -A make_clean # Commands to clean build files from libraries.

declare -A make_config # Commands to configure libraries' build systems.
declare -A make_autoconfig # Commands to automatically configure the builds.
declare -A make_deep_clean # Commands to clear out the config files.

# Get the configuration for the libraries.
for i in "${libs[@]}"; do
	# If the library defines an amocao.conf file for defaults to build it,
	# then get the info from that file.
	[ -f "$i/amocao.conf" ] && source <(grep "=" "$i/amocao.conf")

	# If we have a .conf file for the defaults to build using the library,
	# load defaults from it. (This might overwrite some of the library's
	# own configuration details.)
	[ -f "$i.conf" ] && source <(grep "=" "$i.conf")

	# If we need to autoconfigure but neither of those files existed, then
	# issue an error.
	if [ "$1" == "--auto" ] && [ ! -f "$i/amocao.conf" ] \
		&& [ ! -f "$i.conf" ]
	then
		printerr "Can't autoconfig without valid library .conf files."
		echo "Please set up either $i/amocao.conf or $i.conf"
		exec "$0" # Without autoconfigure option.
	fi

	# Get our final manual configuration details.
	if [ "$1" != "--auto" ]; then
		read -p "MAKE_RELEASE@$i / Make Release Command> " -e \
			-i "${make_release[$i]}" make_release[$i]

		read -p "MAKE_DEBUG@$i / Make Debug Command> " -e \
			-i "${make_debug[$i]}" make_debug[$i]

		read -p "MAKE_CLEAN@$i / Make Clean Command> " -e \
			-i "${make_clean[$i]}" make_clean[$i]

		read -p "MAKE_CONFIG@$i / Make Config Command> " -e \
			-i "${make_config[$i]}" make_config[$i]

		read -p "MAKE_AUTOCONFIG@$i / Make Autoconfig Command> " -e \
			-i "${make_autoconfig[$i]}" make_autoconfig[$i]

		read -p "MAKE_DEEP_CLEAN@$i / Make Deep Clean Command> " -e \
			-i "${make_deep_clean[$i]}" make_deep_clean[$i]
	fi

	{ # Append these details to the config makefile.
		echo ".PHONY: $i-release $i-debug $i-clean"
		echo ".PHONY: $i-config $i--autoconfig $i-deep-clean"
		echo ""
		echo -e "$i-release :"
		echo -e "\t@+printf \"  MAKE  $i-release\"; \$(PROGRESS)"
		echo -e "\t+@cd $i; ${make_release[$i]}"
		echo ""
		echo -e "$i-debug :"
		echo -e "\t@+printf \"  MAKE  $i-debug\"; \$(PROGRESS)"
		echo -e "\t+@cd $i; ${make_debug[$i]}"
		echo ""
		echo -e "$i-clean :"
		echo -e "\t@+printf \"  MAKE  $i-clean\"; \$(PROGRESS)"
		echo -e "\t+@cd $i; ${make_clean[$i]}"
		echo ""
		echo -e "$i-config :"
		echo -e "\t@+printf \"  MAKE  $i-config\"; \$(PROGRESS)"
		echo -e "\t+@cd $i; ${make_config[$i]}"
		echo ""
		echo -e "$i-autoconfig :"
		echo -e "\t@+printf \"  MAKE  $i-autoconfig\"; \$(PROGRESS)"
		echo -e "\t+@cd $i; ${make_autoconfig[$i]}"
		echo ""
		echo -e "$i-deep-clean :"
		echo -e "\t@+printf \"  MAKE  $i-deep-clean\"; \$(PROGRESS)"
		echo -e "\t+@cd $i; ${make_deep_clean[$i]}"
		echo ""

	} >> ".config.mk"
done
