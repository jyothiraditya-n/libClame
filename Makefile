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
ifeq ($(shell [ -d ".config/" ] && echo "config"),)
ifeq ($(filter config,$(MAKECMDGOALS)),)
ifeq ($(filter autoconfig,$(MAKECMDGOALS)),)
$(info *** Configuration missing; running `make autoconfig'.)
$(shell make autoconfig)
endif
endif
endif

# Compilation Flags for Make
.DEFAULT_GOAL = debug

# Compilation Flags for C
CC = $(shell cat .config/cc.conf)

ifneq ($(filter release,$(MAKECMDGOALS)),)
CFLAGS = $(shell cat .config/cflags_release.conf)
else
CFLAGS = $(shell cat .config/cflags_debug.conf)
endif

# Compilation Flags for C++
CPP = $(shell cat .config/cpp.conf)

ifneq ($(filter release,$(MAKECMDGOALS)),)
CCFLAGS = $(shell cat .config/ccflags_release.conf)
else
CCFLAGS = $(shell cat .config/ccflags_debug.conf)
endif

# Compilation Flags for Linking
LD = $(shell cat .config/ld.conf)
AR = $(shell cat .config/ar.conf)

LD_LIBS = $(shell cat .config/ld_libs.conf)

# Command to make the LaTeX docs.
ifneq ($(filter release,$(MAKECMDGOALS)),)
LATEX = $(shell cat .config/latex_release.conf)
else
LATEX = $(shell cat .config/latex_debug.conf)
endif

# Files that need to be created.
folders = build/ build/docs/ build/src/

build_scripts = $(wildcard *.sh)
test_scripts = $(wildcard tests/*.sh)
scripts = $(build_scripts) $(test_scripts)

c_texs = $(patsubst demos/%.c,build/docs/%.c.tex,$(wildcard demos/*.c))
cpp_texs = $(patsubst demos/%.cpp,build/docs/%.cpp.tex,$(wildcard demos/*.cpp))
texs = $(c_texs) $(cpp_texs)

docs = $(texs) $(wildcard docs/*)
headers = $(wildcard inc/*.h) $(wildcard inc/libClame/*.h)
headers = $(wildcard inc/*.hpp) $(wildcard inc/libClame/*.hpp)

c_objs = $(patsubst src/%.c,build/src/%.o,$(wildcard src/*.c))
cpp_objs = $(patsubst src/%.cpp,build/src/%_cpp.o,$(wildcard src/*.cpp))
objs = $(c_objs) $(cpp_objs)

c_demos = $(patsubst demos/%.c,build/%_demo,$(wildcard demos/*.c))
cpp_demos += $(patsubst demos/%.cpp,build/%_cpp_demo,$(wildcard demos/*.cpp))
demos = $(c_demos) $(cpp_demos)

c_tests = $(patsubst tests/%.c,build/%_test,$(wildcard tests/*.c))
cpp_tests += $(patsubst tests/%.cpp,build/%_cpp_test,$(wildcard tests/*.cpp))
tests = $(c_tests) $(cpp_tests)

# Automatic Rules for creating those files.
$(folders) : % :
	mkdir -p $@

.PHONY : $(scripts)

$(scripts) : % :
	chmod +x $@

$(c_texs) : build/docs/%.c.tex : demos/%.c $(build_scripts) | \
	build/docs/texs.tex
	./make_tex.sh "c" $< $@ build/docs/texs.tex

$(cpp_texs) : build/docs/%.cpp.tex : demos/%.cpp $(build_scripts) | \
	build/docs/texs.tex
	./make_tex.sh "cpp" $< $@ build/docs/texs.tex

$(c_objs) : build/src/%.o : src/%.c $(headers) | build/src/
	$(CC) $(CFLAGS) -c $< -o $@

$(cpp_objs) : build/src/%_cpp.o : src/%.cpp $(headers) | build/src/
	$(CPP) $(CCFLAGS) -c $< -o $@

$(c_demos) : build/%_demo : demos/%.c $(headers) build/libClame.a
	$(CC) $(CFLAGS) $< -o $@ $(LD_LIBS)

$(cpp_demos) : build/%_cpp_demo : demos/%.cpp $(headers) build/libClame.a
	$(CPP) $(CCFLAGS) $< -o $@ $(LD_LIBS)

$(c_tests) : build/%_test : tests/%.c $(headers) build/libClame.a
	$(CC) $(CFLAGS) $< -o $@ $(LD_LIBS)

$(cpp_tests) : build/%_cpp_test : tests/%.cpp $(headers) build/libClame.a
	$(CPP) $(CCFLAGS) $< -o $@ $(LD_LIBS)

# Other, manually defined recipes.
.PHONY : build/docs/texs.tex

build/docs/texs.tex : | build/docs/
	-rm $@

build/libClame.a : $(objs)
	$(AR) -r build/libClame.a $(objs)

build/libClame.pdf : $(docs)
	mkdir -p build/docs/
	cp docs/* build/docs/
	cd build/docs/; $(LATEX)
	mv build/docs/main.pdf build/libClame.pdf

# Commands
.PHONY : config autoconfig release debug demos docs test clean deep-clean

config : $(build_scripts)
	./configure.sh

autoconfig : $(build_scripts)
	./configure.sh -auto

release : build/libClame.a

debug : build/libClame.a

demos : $(demos)

docs : build/libClame.pdf

clean :
	-rm -r build/

deep-clean : clean
	-rm -r .config/

# Testing requires a little bit of hacky coding.
runnable_tests = $(patsubst %,run-%,$(test_scripts))

.PHONY : $(runnable_tests)
$(runnable_tests) : run-% : % $(tests)
	$<

test : $(runnable_tests)
