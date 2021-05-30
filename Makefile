# libClame: Command-line Arguments Made Easy
# Copyright (C) 2021 Jyothiraditya Nellakra
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

headers = $(wildcard include/*.h)
objs = $(patsubst %.c,%.o,$(wildcard source/*.c))

demos = $(patsubst demo/%.c,%,$(wildcard demo/*.c))
objs += $(patsubst %.c,%.o,$(wildcard demo/*.c))

files = $(foreach file,$(objs) $(demo_objs),$(wildcard $(file)))
files += $(foreach file,$(demos) $(demo_shs),$(wildcard $(file)))
files += $(wildcard *.a)

CLEAN = $(foreach file,$(files),rm $(file);)

CFLAGS ?= -std=gnu17 -Wall -Wextra -Werror -O3 -I include/
LD_LIBS ?= -L. -lClame

$(objs) : %.o : %.c $(headers)
	$(CC) $(CFLAGS) -c $< -o $@

libClame.a : $(objs)
	$(AR) -r libClame.a $(objs)

$(demos) : % : demo/%.o libClame.a
	$(CC) $< -o $@ $(LD_LIBS)

.DEFAULT_GOAL = all
.PHONY : all clean

all : libClame.a $(demos)

clean :
	$(CLEAN)