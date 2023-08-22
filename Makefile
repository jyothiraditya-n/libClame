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

# Make sure that config runs before making anything.
ifeq ($(wildcard .config.mk),) # The config file is missing.

ifneq ($(filter autoconfig,$(MAKECMDGOALS)),) # Run make autoconfig.
$(info *** Configuration missing; running 'make autoconfig'.)
$(shell chmod +x configure.sh)
$(shell ./configure.sh --auto)

else # Run make config.
$(info *** Configuration missing; running 'make config'.)
$(shell chmod +x configure.sh)
$(shell ./configure.sh)

endif
endif

# Include the config file.
include .config.mk

# We are a library, and our main goal is to construct the archive file.
FINAL = build/$(NAME).a

# We want to build with debugging symbols as a default.
.DEFAULT_GOAL = debug

ifneq ($(filter release,$(MAKECMDGOALS)),)

CFLAGS = $(CFLAGS_RELEASE)
CCFLAGS = $(CCFLAGS_RELEASE)
TEST_FLAGS = $(TEST_FLAGS_RELEASE)

else

CFLAGS = $(CFLAGS_DEBUG)
CCFLAGS = $(CCFLAGS_DEBUG)
TEST_FLAGS = $(TEST_FLAGS_DEBUG)

endif

# Lists of files to be used in our recipe rules.

# Shell scripts.
build_scripts = $(wildcard *.sh)
demo_scripts = $(shell find demos/ -name "*.sh")
test_scripts = $(shell find tests/ -name "*.sh")
scripts = $(build_scripts) $(demo_scripts) $(test_scripts)

# Core C/C++ source files.
c_srcs = $(shell find src/ -name "*.c")
cc_srcs = $(shell find src/ -name "*.cc")

# Core C/C++ header files.
c_headers = $(shell find inc/ -name "*.h")
cc_headers = $(shell find inc/ -name "*.hh")
headers = $(c_headers) $(cc_headers)

# Objects that need to be made from the C/C++ source files.
c_objs = $(patsubst %.c,build/%.o,$(c_srcs))
cc_objs = $(patsubst %.cc,build/%_cc.o,$(cc_srcs))
objs = $(c_objs) $(cc_objs)

# C/C++ demo program source files.
c_demo_srcs = $(shell find demos/ -name "*.c")
cc_demo_srcs = $(shell find demos/ -name "*.cc")

# C/C++ demo program output files.
c_demos = $(patsubst demos/%.c,build/%_demo,$(c_demo_srcs))
cc_demos = $(patsubst demos/%.cc,build/%_cc_demo,$(cc_demo_srcs))
demos = $(c_demos) $(cc_demos) $(demo_scripts)

# C/C++ test program source files.
c_test_srcs = $(shell find tests/ -name "*.c")
cc_test_srcs = $(shell find tests/ -name "*.cc")

# C/C++ test program output files.
c_tests = $(patsubst tests/%.c,build/%_test,$(c_test_srcs))
cc_tests = $(patsubst tests/%.cc,build/%_cc_test,$(cc_test_srcs))
tests = $(c_tests) $(cc_tests) $(test_scripts)

# Automatic rules for creating those files.
.PHONY: $(scripts)

# Command to print out how many targets are done and how many remain.
DONE_TALLY = X # Counting starts at 1.
DONE_COUNT = $(words $(DONE_TALLY)$(eval DONE_TALLY := X $(DONE_TALLY)))
LEFT_COUNT = $$(PROGRESS="" $(MAKE) -kq $(MAKECMDGOALS) | wc -l)
TOTAL_COUNT = $$(echo "$(DONE_COUNT) + $(LEFT_COUNT)" | bc -l)

PERCENTAGE = $$(echo "$(DONE_COUNT) * 100 / $(TOTAL_COUNT)" | bc -l)
PROGRESS ?= printf "  \033[0;1m[%.0f%%]\r\033[0;0m" $(PERCENTAGE)


$(scripts) : % :
	@+printf "  CHMOD ${@}\n"; $(PROGRESS)
	@chmod +x $@

$(c_objs) : build/%.o : %.c $(headers)
	@+printf "  CC    ${<}\n"; $(PROGRESS)
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) -c $< -o $@

$(cc_objs) : build/%_cc.o : %.cc $(headers)
	@+printf "  CXX   ${<}\n"; $(PROGRESS)
	@mkdir -p $(@D)
	@$(CXX) $(CCFLAGS) -c $< -o $@

$(c_demos) : build/%_demo : demos/%.c $(headers) $(FINAL) $(LIBS)
	@+printf "  CC    ${<}\n"; $(PROGRESS)
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) $< -o $@ $(LD_LIBS)

$(cc_demos) : build/%_cc_demo : demos/%.cc $(headers) $(FINAL) $(LIBS)
	@+printf "  CXX   ${<}\n"; $(PROGRESS)
	@mkdir -p $(@D)
	@$(CXX) $(CCFLAGS) $< -o $@ $(LD_LIBS)

$(c_tests) : build/%_test : tests/%.c $(headers) $(FINAL) $(LIBS)
	@+printf "  CC    ${<}\n"; $(PROGRESS)
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) $< -o $@ $(LD_LIBS)

$(cc_tests) : build/%_cc_test : tests/%.cc $(headers) $(FINAL) $(LIBS)
	@+printf "  CXX   ${<}\n"; $(PROGRESS)
	@mkdir -p $(@D)
	@$(CXX) $(CCFLAGS) $< -o $@ $(LD_LIBS)

# Our main target file.
$(FINAL): $(objs)
	@+printf "  AR    $(FINAL)\n"; $(PROGRESS)
	@mkdir -p $(@D)
	@$(AR) rcs $(FINAL) $(objs)

# Commands
.PHONY : release debug demos clean deep-clean
.PHONY : config autoconfig tidy format

release: $(FINAL)

debug: $(FINAL)

demos: $(demos)

clean :
	@+printf "  RM    build/\n"; $(PROGRESS)
	@-rm -r build/

deep-clean : clean
	@+printf "  RM    .config.mk\n"; $(PROGRESS)
	@-rm -r .config.mk

config: $(build_scripts)
	@+printf "  BASH  configure.sh\n"; $(PROGRESS)
	@./configure.sh

autoconfig: $(build_scripts)
	@+printf "  BASH  configure.sh --auto\n"; $(PROGRESS)
	@./configure.sh --auto

tidy :
	@+printf "  BASH  clang-tidy ...\n"; $(PROGRESS)
	@clang-tidy $(c_srcs) $(headers) -- $(CFLAGS)
	@clang-tidy $(c_demo_srcs) -- $(CFLAGS)
	@clang-tidy $(c_test_srcs) -- $(CFLAGS)
	@cland-tidy $(cc_srcs) $(headers) -- $(CCFLAGS)
	@cland-tidy $(cc_demo_srcs) -- $(CCFLAGS)
	@cland-tidy $(cc_test_srcs) -- $(CCFLAGS)

# Testing requires a little bit of hacky coding for the make configuration.
runnable_tests = $(patsubst %,run-%,$(test_scripts))

.PHONY: $(runnable_tests) test
$(runnable_tests) : run-% : % $(tests)
	@+printf "  BASH  ${<}\n"; $(PROGRESS)
	@$< $(TEST_FLAGS)

test: $(runnable_tests)
