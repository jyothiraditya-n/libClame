/* libClame: Command-line Arguments Made Easy
 * Copyright (C) 2021 Jyothiraditya Nellakra
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>. */

#include <stdio.h>
#include <stdlib.h>
#include <libClame.h>

char *name;
char *message = "default message";

char *two_things[2] = {
	"first default thing",
	"second default thing"
};

char *some_things[5] = {
	"first of three default things",
	"second of three default things",
	"third of three default things"
};

int num_things = 3;

static void about() {
	printf("  libClame: Command-line Arguments Made Easy\n");
	printf("  Copyright (C) 2021 Jyothiraditya Nellakra\n\n");

	printf("  This program is free software: you can redistribute it and/or modify\n");
	printf("  it under the terms of the GNU General Public License as published by\n");
	printf("  the Free Software Foundation, either version 3 of the License, or\n");
	printf("  (at your option) any later version.\n\n");

	printf("  This program is distributed in the hope that it will be useful,\n");
	printf("  but WITHOUT ANY WARRANTY; without even the implied warranty of\n");
	printf("  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the\n");
	printf("  GNU General Public License for more details.\n\n");

	printf("  You should have received a copy of the GNU General Public License\n");
	printf("  along with this program. If not, see <https://www.gnu.org/licenses/>.\n\n");

	exit(0);
}

static void help() {
	printf("Usage: %s [OPTIONS]\n\n", name);

	printf("Valid options are:\n");
	printf("-message MESSAGE: set the message to MESSAGE.\n");
	printf("-two-things THINGA THINGB: set the two things to THINGA and "
		"THINGB.\n");
	printf("-some-things THINGA THINGB ... --: set some things, between "
		"three and five arguments.\n\n");

	printf("Happy coding! :)\n");
}

int main(int argc, char **argv) {
	name = argv[0];

	// Note: For brevity, I'm not checking for null-pointers issued in case
	// of malloc failing. Although, I don't think this will be necessary
	// for the vast majority of people, you may still want to do it.

	LC_entry_t *entry = LC_new_entry();
	entry -> instr = "-about";
	entry -> func = about;

	entry = LC_new_entry();
	entry -> instr = "-help";
	entry -> func = help;

	entry = LC_new_entry();
	entry -> instr = "-message";
	entry -> data = &message;

	entry = LC_new_entry();
	entry -> instr = "-two-things";
	entry -> data = two_things;
	entry -> array_min = 2;
	entry -> array_max = 2;

	entry = LC_new_entry();
	entry -> instr = "-some-things";
	entry -> data = some_things;
	entry -> array_min = 3;
	entry -> array_max = 5;
	entry -> array_len = &num_things;

	int ret = LC_parse(argc, argv);

	if(ret != LC_OK) {
		help();
		exit(1);
	}

	printf("Message: %s\n\n", message);
	printf("Two Things: %s\n%s\n\n", two_things[0], two_things[1]);

	for(int i = 0; i < num_things; i++) {
		printf("Thing %d of %d Things: %s\n",
			i, num_things, some_things[i]);
	}

	exit(0);
}