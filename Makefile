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

cheaders = $(wildcard inc/*.h)
cobjs = $(patsubst %.c,%.o,$(wildcard src/*.c))
ccobjs = $(patsubst %.cc,%.o,$(wildcard src/*.cc))

ccheaders = $(wildcard inc/libClame/*.h)
cdemos = $(patsubst demo/%.c,%,$(wildcard demo/*.c))
cdemo_objs = $(patsubst %.c,%.o,$(wildcard demo/*.c))

ccdemos = $(patsubst demo/%.cc,%,$(wildcard demo/*.cc))
ccdemo_objs = $(patsubst %.cc,%.o,$(wildcard demo/*.cc))

files = $(foreach file,$(cobjs) $(cdemo_objs),$(wildcard $(file)))
files += $(foreach file,$(ccobjs) $(ccdemo_objs),$(wildcard $(file)))
files += $(foreach file,$(cdemos),$(wildcard $(file)))
files += $(foreach file,$(ccdemos),$(wildcard $(file)))
files += $(wildcard *.a)

CLEAN = $(foreach file,$(files),rm $(file);)

ifneq ($(DEBUG),)
	CPPFLAGS = -std=c99 -Wall -Wextra -Wpedantic -I inc/ -O0 -g
	CFLAGS = -std=c99 -O0 -g

	CXXPPFLAGS = -std=c++17 -Wall -Wextra -Wpedantic -I inc/ -O0 -g
	CXXFLAGS = -std=c++17 -O0 -g
else
	CPPFLAGS += -std=c99 -Wall -Wextra -Wpedantic -I inc/ -O3
	CFLAGS += -std=c99 -O3 -s

	CXXPPFLAGS += -std=c++17 -Wall -Wextra -Wpedantic -I inc/ -O3
	CXXFLAGS += -std=c++17 -O3
endif

LD_LIBS ?= -L. -lClame

$(cobjs) $(cdemo_objs) : %.o : %.c $(headers)
	$(CC) $(CPPFLAGS) -c $< -o $@

$(ccobjs) $(ccdemo_objs) : %.o : %.cc $(headers)
	$(CXX) $(CXXPPFLAGS) -c $< -o $@

libClame.a : $(cobjs) $(ccobjs)
	$(AR) -r libClame.a $(cobjs) $(ccobjs)

$(cdemos) : % : demo/%.o libClame.a
	$(CC) $(CFLAGS) $< -o $@ $(LD_LIBS)

$(ccdemos) : % : demo/%.o libClame.a
	$(CXX) $(CXXFLAGS) $< -o $@ $(LD_LIBS)

.DEFAULT_GOAL = all
.PHONY : all clean debug

all : libClame.a $(cdemos) $(ccdemos)

clean :
	$(CLEAN)

debug :
	+DEBUG="true" make all