/* libClame: Command-line Arguments Made Easy
 * Copyright (C) 2021-2022 Jyothiraditya Nellakra
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

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <LC_files.h>
#include <LC_vars.h>

char *LCf_program_name = "";
uint8_t LCf_program_ver = 1;
uint8_t LCf_program_subver = 0;

static uint32_t magic = LCF_MAGIC;
static uint8_t bits = LCF_BITS;
static uint8_t version = LCF_VERSION;
static uint8_t subversion = LCF_SUBVERSION;

static int read_native(FILE *file, size_t size) {
	size_t len = 0;
	size_t ret = fread(&len, size, 1, file);
	if(ret != 1) { fclose(file); return LCF_FILEIO_ERR; }

	char *id = malloc(len + 1);
	ret = fread(id, len, 1, file);
	if(ret != 1) { fclose(file); free(id); return LCF_FILEIO_ERR; }
	else id[len] = 0;

	if(strcmp(id, LCf_program_name)) {
		fclose(file); free(id);
		return LCF_BAD_PROG_NAME;
	}

	free(id);

loop:	if(feof(file)) return LCF_OK;

	ret = fread(&len, size, 1, file);
	if(ret != 1) { fclose(file); return LCF_FILEIO_ERR; }

	id = malloc(len + 1);
	ret = fread(id, len, 1, file);
	if(ret != 1) { fclose(file); free(id); return LCF_FILEIO_ERR; }
	else id[len] = 0;

	LCv_t *var = LCv_get(id);
	if(!var) { fclose(file); free(id); return LCF_BAD_VAR; }
	else free(id);

	ret = fread(&len, size, 1, file);
	if(ret != 1) { fclose(file); return LCF_FILEIO_ERR; }

	if(len < var -> min_len || len > var -> max_len) {
		fclose(file);
		return LCF_BAD_LEN;
	}

	ret = fread(var -> data, var -> size, len, file);
	if(ret != len) { fclose(file); return LCF_FILEIO_ERR; }
	else *(var -> len) = len;

	goto loop;
}

static int read_64on32(FILE *file) {
	size_t lens[2] = {0, 0};
	size_t ret = fread(lens, 4, 1, file);
	if(ret != 1) { fclose(file); return LCF_FILEIO_ERR; }
	if(lens[1]) { fclose(file); return LCF_BAD_ARCH; }

	char *id = malloc(lens[0] + 1);
	ret = fread(id, lens[0], 1, file);
	if(ret != 1) { fclose(file); free(id); return LCF_FILEIO_ERR; }
	else id[lens[0]] = 0;

	if(strcmp(id, LCf_program_name)) {
		fclose(file); free(id);
		return LCF_BAD_PROG_NAME;
	}

	free(id);

loop:	if(feof(file)) return LCF_OK;

	ret = fread(lens, 4, 1, file);
	if(ret != 1) { fclose(file); return LCF_FILEIO_ERR; }
	if(lens[1]) { fclose(file); return LCF_BAD_ARCH; }

	id = malloc(lens[0] + 1);
	ret = fread(id, lens[0], 1, file);
	if(ret != 1) { fclose(file); free(id); return LCF_FILEIO_ERR; }
	else id[lens[0]] = 0;

	LCv_t *var = LCv_get(id);
	if(!var) { fclose(file); free(id); return LCF_BAD_VAR; }
	else free(id);

	ret = fread(lens, 4, 1, file);
	if(ret != 1) { fclose(file); return LCF_FILEIO_ERR; }
	if(lens[1]) { fclose(file); return LCF_BAD_ARCH; }

	if(lens[0] < var -> min_len || lens[0] > var -> max_len) {
		fclose(file);
		return LCF_BAD_LEN;
	}

	ret = fread(var -> data, var -> size, lens[0], file);
	if(ret != lens[0]) { fclose(file); return LCF_FILEIO_ERR; }
	else *(var -> len) = lens[0];

	goto loop;
}


int LCf_read(const char *filename) {
	FILE *file = fopen(filename, "rb");
	if(!file) return LCF_FILEIO_ERR;

	uint32_t fmagic = 0;
	uint8_t fbits, fversion = 0, fsubversion = 0;
	uint8_t f_program_ver = 0, f_program_subver = 0;

	size_t ret = fread(&fmagic, 4, 1, file);
	if(ret != 1) { fclose(file); return LCF_FILEIO_ERR; }

	if(magic != fmagic) {
		fclose(file);
		return LCF_BAD_FORMAT;
	}

	ret = fread(&fbits, 1, 1, file);
	if(ret != 1) { fclose(file); return LCF_FILEIO_ERR; }

	ret = fread(&fversion, 1, 1, file);
	if(ret != 1) { fclose(file); return LCF_FILEIO_ERR; }

	ret = fread(&fsubversion, 1, 1, file);
	if(ret != 1) { fclose(file); return LCF_FILEIO_ERR; }

	if(version != fversion) {
		fclose(file);
		return LCF_BAD_LCF_VER;
	}

	if(subversion > fsubversion) {
		fclose(file);
		return LCF_BAD_LCF_VER;
	}

	ret = fread(&f_program_ver, 1, 1, file);
	if(ret != 1) { fclose(file); return LCF_FILEIO_ERR; }

	ret = fread(&f_program_subver, 1, 1, file);
	if(ret != 1) { fclose(file); return LCF_FILEIO_ERR; }

	if(LCf_program_ver != f_program_ver) {
		fclose(file);
		return LCF_BAD_LCF_VER;
	}

	if(LCf_program_subver > f_program_subver) {
		fclose(file);
		return LCF_BAD_PROG_VER;
	}

	if(LCF_BITS == fbits) return read_native(file, sizeof(size_t));
	else if(LCF_BITS == 64 && fbits == 32) return read_native(file, 4);
	else if(LCF_BITS == 32 && fbits == 64) return read_64on32(file);
	
	else {
		fclose(file);
		return LCF_BAD_ARCH;
	}
}

int LCf_save(const char *filename) {
	FILE *file = fopen(filename, "wb");
	if(!file) return LCF_FILEIO_ERR;

	size_t ret = fwrite(&magic, 4, 1, file);
	if(ret != 1) { fclose(file); return LCF_FILEIO_ERR; }

	ret = fwrite(&bits, 1, 1, file);
	if(ret != 1) { fclose(file); return LCF_FILEIO_ERR; }

	ret = fwrite(&version, 1, 1, file);
	if(ret != 1) { fclose(file); return LCF_FILEIO_ERR; }

	ret = fwrite(&subversion, 1, 1, file);
	if(ret != 1) { fclose(file); return LCF_FILEIO_ERR; }

	ret = fwrite(&LCf_program_ver, 1, 1, file);
	if(ret != 1) { fclose(file); return LCF_FILEIO_ERR; }

	ret = fwrite(&LCf_program_subver, 1, 1, file);
	if(ret != 1) { fclose(file); return LCF_FILEIO_ERR; }

	size_t len = strlen(LCf_program_name);

	ret = fwrite(&len, sizeof(size_t), 1, file);
	if(ret != 1) { fclose(file); return LCF_FILEIO_ERR; }

	ret = fwrite(LCf_program_name, len, 1, file);
	if(ret != 1) { fclose(file); return LCF_FILEIO_ERR; }

	for(LCv_t *i = LC_vars; i; i = i -> next) {
		len = strlen(i -> id);

		ret = fwrite(&len, sizeof(size_t), 1, file);
		if(ret != 1) { fclose(file); return LCF_FILEIO_ERR; }

		ret = fwrite(i -> id, len, 1, file);
		if(ret != 1) { fclose(file); return LCF_FILEIO_ERR; }

		if(i -> len) {
			ret = fwrite(i -> len, sizeof(size_t), 1, file);
			if(ret != 1) { fclose(file); return LCF_FILEIO_ERR; }

			ret = fwrite(i -> data, i -> size, *(i -> len), file);

			if(ret != *(i -> len)) {
				fclose(file);
				return LCF_FILEIO_ERR;
			}
		}

		else {
			ret = fwrite(i -> data, i -> size, 1, file);
			if(ret != 1) { fclose(file); return LCF_FILEIO_ERR; }
		}
	}

	int ret2 = fclose(file);
	if(ret2) return LCF_FILEIO_ERR;

	return LCF_OK;
}