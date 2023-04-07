

	/* If the data is an array, then data will be treated as a pointer to
	 * the memory where we should store a pointer to the start of a runtime
	 * allocated array. (data will be transparently treated as a
	 * a variable of type void **) */

	/* We have a boolean to define whether or not the variable is a string
	 * because we will find the length of a string using
	 * strlen(*(char **) data_ptr) instead of needing arr_length to be a
	 * pointer to the size. */
	bool data_is_string; // Set to false if not an array, too.

static int read_native(LCf_var_t *vars, size_t vars_length,
	FILE *file, size_t size
);

static int read_64on32(LCf_var_t *vars, size_t vars_length, FILE *file);

LCf_var_t *find_var(LCf_var_t *vars, size_t var_length, char *name);


int LCf_read(const char *filename, LCf_var_t *vars, size_t vars_length) {
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

	if(LCF_BITS == fbits) return read_native(vars, vars_length, file, sizeof(size_t));
	else if(LCF_BITS == 64 && fbits == 32) return read_native(vars, vars_length, file, 4);
	else if(LCF_BITS == 32 && fbits == 64) return read_64on32(vars, vars_length, file);
	
	else {
		fclose(file);
		return LCF_BAD_ARCH;
	}
}

int LCf_save(const char *filename, LCf_var_t *vars, size_t vars_length) {
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

	for(size_t i = 0; i < vars_length; i++) {
		len = strlen(vars[i].name);

		ret = fwrite(&len, sizeof(size_t), 1, file);
		if(ret != 1) { fclose(file); return LCF_FILEIO_ERR; }

		ret = fwrite(vars[i].name, len, 1, file);
		if(ret != 1) { fclose(file); return LCF_FILEIO_ERR; }


		if(vars[i].arr_length) {
			if(!*(void **)vars[i].data_ptr) {
				
			}

			if(vars[i].data_is_string) {
				*(vars[i].arr_length) = strlen(*(char **)(vars[i].data_ptr)) + 1;
			}

			ret = fwrite(vars[i].arr_length, sizeof(size_t), 1, file);
			if(ret != 1) { fclose(file); return LCF_FILEIO_ERR; }

			ret = fwrite(*(void **)(vars[i].data_ptr), vars[i].var_length, *(vars[i].arr_length), file);

			if(ret != *(vars[i].arr_length)) {
				fclose(file);
				return LCF_FILEIO_ERR;
			}
		}

		else {
			ret = fwrite(vars[i].data_ptr, vars[i].var_length, 1, file);
			if(ret != 1) { fclose(file); return LCF_FILEIO_ERR; }
		}
	}

	int ret2 = fclose(file);
	if(ret2 == EOF) return LCF_FILEIO_ERR;

	return LCF_OK;
}

static int read_native(LCf_var_t *vars, size_t vars_length, FILE *file,
	size_t size
){
	size_t len = 0;
	size_t ret = fread(&len, size, 1, file);
	if(ret != 1) { fclose(file); return LCF_FILEIO_ERR; }

	char *name = malloc(len + 1);
	if(!name) { fclose(file); return LCF_MALLOC_ERR; }

	ret = fread(name, len, 1, file);
	if(ret != 1) { fclose(file); free(name); return LCF_FILEIO_ERR; }
	else name[len] = 0;

	if(strcmp(name, LCf_program_name)) {
		fclose(file); free(name);
		return LCF_BAD_PROG_NAME;
	}

	free(name);

	while(true) {
		ret = fread(&len, size, 1, file);
		if(ret != 1) {
			if(feof(file)) {
				int ret = fclose(file);
				if(ret == EOF) return LCF_FILEIO_ERR;
				else return LCF_OK;
			}

			fclose(file); return LCF_FILEIO_ERR;
		}

		name = malloc(len + 1);
		if(!name) {fclose(file); return LCF_MALLOC_ERR; }

		ret = fread(name, len, 1, file);
		if(ret != 1) { fclose(file); free(name); return LCF_FILEIO_ERR; }
		else name[len] = 0;

		LCf_var_t *var = find_var(vars, vars_length, name);
		if(!var) { fclose(file); free(name); return LCF_BAD_VAR; }
		else free(name);

		if(!var -> arr_length && !var -> data_is_string) {
			ret = fread(var -> data_ptr, var -> var_length, len, file);
			if(ret != len) { fclose(file); return LCF_FILEIO_ERR; }
			continue;
		}

		ret = fread(&len, size, 1, file);
		if(ret != 1) { fclose(file); return LCF_FILEIO_ERR; }

		if(len < var -> min_arr_length || len > var -> max_arr_length) {
			fclose(file);
			return LCF_BAD_LEN;
		}

		if(*(void **)(var -> data_ptr)) free(*(void **)(var -> data_ptr));

		*(void **)(var -> data_ptr) = malloc(var -> var_length * len);
		if(!*(void **)(var -> data_ptr)) {fclose(file); return LCF_MALLOC_ERR;}

		ret = fread(*(void **)(var -> data_ptr), var -> var_length, len, file);
		if(ret != len) { fclose(file); return LCF_FILEIO_ERR; }
		
		if(!var -> data_is_string) *(var -> arr_length) = len;
	}
}

static int read_64on32(LCf_var_t *vars, size_t vars_length, FILE *file) {
	size_t lens[2] = {0, 0};
	size_t ret = fread(lens, 4, 1, file);
	if(ret != 1) { fclose(file); return LCF_FILEIO_ERR; }
	if(lens[1]) { fclose(file); return LCF_BAD_ARCH; }

	char *name = malloc(lens[0] + 1);
	if(!name) {fclose(file); return LCF_MALLOC_ERR; }

	ret = fread(name, lens[0], 1, file);
	if(ret != 1) { fclose(file); free(name); return LCF_FILEIO_ERR; }
	else name[lens[0]] = 0;

	if(strcmp(name, LCf_program_name)) {
		fclose(file); free(name);
		return LCF_BAD_PROG_NAME;
	}

	free(name);

	while(true) {
		ret = fread(lens, 4, 1, file);
		if(ret != 1) {
			if(feof(file)) {
				int ret = fclose(file);
				if(ret == EOF) return LCF_FILEIO_ERR;
				else return LCF_OK;
			}

			fclose(file); return LCF_FILEIO_ERR;
		}
		
		if(lens[1]) { fclose(file); return LCF_BAD_ARCH; }

		name = malloc(lens[0] + 1);
		if(!name) {fclose(file); return LCF_MALLOC_ERR; }

		ret = fread(name, lens[0], 1, file);
		if(ret != 1) { fclose(file); free(name); return LCF_FILEIO_ERR; }
		else name[lens[0]] = 0;

		LCf_var_t *var = find_var(vars, vars_length, name);
		if(!var) { fclose(file); free(name); return LCF_BAD_VAR; }
		else free(name);

		if(!var -> arr_length) {
			ret = fread(var -> data_ptr, var -> var_length, 1, file);
			if(ret != lens[0]) { fclose(file); return LCF_FILEIO_ERR; }
			continue;
		}

		ret = fread(lens, 4, 1, file);
		if(ret != 1) { fclose(file); return LCF_FILEIO_ERR; }
		if(lens[1]) { fclose(file); return LCF_BAD_ARCH; }

		if(lens[0] < var -> min_arr_length || lens[0] > var -> max_arr_length) {
			fclose(file);
			return LCF_BAD_LEN;
		}

		if(*(void **)(var -> data_ptr)) free(*(void **)(var -> data_ptr));

		*(void **)(var -> data_ptr) = malloc(var -> var_length * lens[0]);
		if(!*(void **)(var -> data_ptr)) {fclose(file); return LCF_MALLOC_ERR;}

		ret = fread(*(void **)(var -> data_ptr), var -> var_length, lens[0], file);
		if(ret != lens[0]) { fclose(file); return LCF_FILEIO_ERR; }
		
		if(!var -> data_is_string) *(var -> arr_length) = lens[0];
	}
}

LCf_var_t *find_var(LCf_var_t *vars, size_t var_length, char *name) {
	for(size_t i = 0; i < var_length; i++) {
		if(!strcmp(vars[i].name, name)) return &vars[i];
	}

	return NULL;
}