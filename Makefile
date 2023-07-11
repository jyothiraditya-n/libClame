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

# Make sure that config runs before making anything.
ifeq ($(shell [ -d "config/" ] && echo "config"),)
ifeq ($(filter config,$(MAKECMDGOALS)),)
$(info *** Configuration missing; running `make config'.)
$(shell make config)
endif
endif

# Compilation Flags for Make
.DEFAULT_GOAL = debug

# Compilation Flags for C
CC = $(shell cat config/cc.conf)

ifneq ($(filter release,$(MAKECMDGOALS)),)
CFLAGS = $(shell cat config/cflags_release.conf)
else
CFLAGS = $(shell cat config/cflags_debug.conf)
endif

# Compilation Flags for Linking
LD = $(shell cat config/ld.conf)
AR = $(shell cat config/ar.conf)

LD_LIBS = $(shell cat config/ld_libs.conf)

# Command to make the LaTeX docs.
ifneq ($(filter release,$(MAKECMDGOALS)),)
LATEX = $(shell cat config/latex_release.conf)
else
LATEX = $(shell cat config/latex_debug.conf)
endif

# Files that need to be created.
folders = build/ build/docs/ build/src/

build_scripts = $(wildcard scripts/*.sh)
test_scripts = $(wildcard tests/*.sh)
scripts = $(build_scripts) $(test_scripts)

texs = $(patsubst demos/%.c,build/docs/%.c.tex,$(wildcard demos/*.c))
headers = $(wildcard inc/*.h)

objs = $(patsubst src/%.c,build/src/%.o,$(wildcard src/*.c))
demos = $(patsubst demos/%.c,build/%_demo,$(wildcard demos/*.c))
tests = $(patsubst tests/%.c,build/%_test,$(wildcard tests/*.c))

# Automatic Rules for creating those files.
$(folders) : % :
	mkdir -p $@

.PHONY : $(scripts)

$(scripts) : % :
	chmod +x $@

$(texs) : build/docs/%.c.tex : demos/%.c $(build_scripts) | build/docs/texs.tex
	scripts/c-to-tex.sh $< $@ build/docs/texs.tex

$(objs) : build/src/%.o : src/%.c $(headers) | build/src/
	$(CC) $(CFLAGS) -c $< -o $@

$(demos) : build/%_demo : demos/%.c $(headers) build/libClame.a
	$(CC) $(CFLAGS) $< -o $@ $(LD_LIBS)

$(tests) : build/%_test : tests/%.c $(headers) build/libClame.a
	$(CC) $(CFLAGS) $< -o $@ $(LD_LIBS)

# Other, manually defined recipes.
.PHONY : build/docs/texs.tex

build/docs/texs.tex : | build/docs/
	-rm $@

build/libClame.a : $(objs)
	$(AR) -r build/libClame.a $(objs)

build/libClame.pdf : $(texs)
	mkdir -p build/docs/
	cp docs/* build/docs/
	cd build/docs/; $(LATEX)
	mv build/docs/main.pdf build/libClame.pdf

# Commands
.PHONY : config release debug demos docs test clean deep-clean

config : $(build_scripts)
	scripts/configure.sh

release : build/libClame.a

debug : build/libClame.a

demos : $(demos)

docs : build/libClame.pdf

clean :
	-rm -r build/

deep-clean : clean
	-rm -r config/

# Testing requires a little bit of hacky coding.
runnable_tests = $(patsubst %,run-%,$(test_scripts))

.PHONY : $(runnable_tests)
$(runnable_tests) : run-% : % $(demos) $(tests)
	$<

test : $(runnable_tests)