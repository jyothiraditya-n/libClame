# libClame: Command-line Arguments Made Easy
# Copyright (C) 2021-2022 Jyothiraditya Nellakra
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

headers = $(wildcard inc/*.h)
objs = $(patsubst %.c,%.o,$(wildcard src/*.c))

demos = $(patsubst demo/%.c,%,$(wildcard demo/*.c))
demo_objs = $(patsubst %.c,%.o,$(wildcard demo/*.c))

files = $(foreach file,$(objs) $(demo_objs),$(wildcard $(file)))
files += $(foreach file,$(demos),$(wildcard $(file)))
files += $(wildcard *.a)

CLEAN = $(foreach file,$(files),rm $(file);)

CPPFLAGS += -std=c99 -Wall -Wextra -Wpedantic -I inc/ -O3
CFLAGS += -std=c99 -O3 -s
LD_LIBS ?= -L. -lClame

$(objs) : %.o : %.c $(headers)
	$(CC) $(CPPFLAGS) -c $< -o $@

libClame.a : $(objs)
	$(AR) -r libClame.a $(objs)

$(demo_objs) : %.o : %.c $(headers)
	$(CC) $(CPPFLAGS) -c $< -o $@

$(demos) : % : demo/%.o libClame.a
	$(CC) $(CFLAGS) $< -o $@ $(LD_LIBS)

.DEFAULT_GOAL = all
.PHONY : all clean

all : libClame.a $(demos)

clean :
	$(CLEAN)