# <LC_args.h>

`LC_args.h` is the primary header file for libClame's command-line argument parsing algorithms. This document will detail how to interface your program with it, both within the C code as well as the grammar of specifying values on the command line. If you'd like further information that what's written here, two good places for further reading are the comments in the header file `inc/LC_args.h` and the demonstration program `demo/args.c`.

## The Interface in C

The library defines a structure type called `LCa_flag_t`, you'll want to create an array, where each element details the properties for a flag in terms of its behaviour. That means what function it calls and how it sets variables or arrays. The following is a list of the array elements copied from the header `inc/LC_args.h`:

```c
typedef struct {
	/* The long flag is what you specify when you type `--<something>`
	 * while `-<something> specifies a series of short flags`, each of
	 * which is one character long. */

	const char *long_flag; // Set to NULL to not use this.
	char short_flag; // Set to 0 to not use this.

	/* If you have a function that takes no parameters and returns an
	 * integer, you can point to it here to be run when the flag is found.
	 * If you don't have anything you want to run, set the pointer to NULL
	 * so that we can detect that and avoid causing a segfault. */

	int (*function)();

	/* Note about pointers: While we do check that you're not giving us
	 * NULL pointers in places that it doesn't make sense, we can't check
	 * that the memory it points to is either the correct variable or a
	 * place in memory that can actually be written to. So, make sure to
	 * check that as you're writing your code. */

	/* If the function executed correctly, go ahead and return 0. Any other
	 * value will be treated as an error. When that occurs, we will save
	 * the returned value as well as the function pointer as specified
	 * later on in the header, for you to process through later. */

	#define LCA_FUNCTION_OK 0
	#define LCA_FUNCTION_ERR (!LCA_FUNCTION_OK)

	/* If the flag is used to set a variable in the code, then you can
	 * specify the variable's name as well as its type and location with
	 * these parameters. */

	void *var_ptr; // Set this to NULL if you don't have a variable.
	int var_type; // This value is not used if you don't have a variable.

	/* If the variable you want to get is a string constant (i.e. a
	 * pointer to the start of a null-terminated set of characters in
	 * memory) we can set the pointer for you directly without needing a
	 * format string to pass to sscanf. In this case, var_ptr is
	 * transparently treated as a (char *)* type. We can also set a
	 * boolean value directly to the value in bool_value, wherein var_ptr
	 * is of type (bool *). */

	#define LCA_STRING_VAR 1
	#define LCA_BOOL_VAR 2
	#define LCA_OTHER_VAR 3

	bool value; // This isn't used if the variable's type isn't a bool.

	/* For all other types of variables, we'll need a format string that'll
	 * get passed as the second argument to sscanf where the first argument
	 * is the string that we've gotten corresponding to the specified data
	 * and the third argument is `var_ptr`. For example, if you want to
	 * get an integer value, you would set var_ptr to &your_integer and
	 * var_ptr would thus transparently be an int* type. */

	const char *fmt_string; // Set this to NULL if you don't have anything.

	/* Note: Because C doesn't give us many powers when it comes to
	 * runtime debugging, it's up to you to make sure that your format
	 * string is correct. The best we can do is error out if you give us a
	 * pointer to null, but otherwise you're on your own. */

	/* If you want to get a set of values as a dynamically allocated array
	 * then you'll just need to give us the length of each array member as
	 * well as a size variable to store the final array size in. Then, if
	 * you want an array of typename T values, you'll want to set var_ptr
	 * to be a pointer to a variable of type T*, i.e. var_ptr will
	 * transparently be of type T**. */

	/* Note that if you have dynamically-sized arrays, we will free the
	 * current allocations if (*(char ***) flag -> var_ptr) isn't NULL so
	 * bewarned that this may cause a segfault if you statically allocate
	 * an array. (Alternatively, if (*(void **)) isn't NULL.) */

	size_t *arr_length; // Set to NULL if it isn't an array.
	size_t var_length; // Set to 0 if it isn't an array.

	/* If you want minimum or maximum length constraints specified, you can
	 * do so with these two. There is however some nuance with the way they
	 * work that you would want to read about in <docs/LC_flags.md>. */

	size_t min_arr_length; // Set to 0 to disable checking.
	size_t max_arr_length; // Set to SIZE_MAX to disable checking.

	/* We need a boolean to keep track of whether a variable has already
	 * been written to, so that we don't allow conflicting flags. */

	bool readonly; // Set this to false by default.

} LCa_flag_t;
```

Once you've defined your array, you'll want to store a pointer to its base element in the variable `LC_args`, as well as its length in the variable `LC_args_length`. You can then run `LCa_read(argc, argv)` to get your arguments. The following is a list of the values the function might return depending on the type of error it may encounter, reproduced from the header file:


```c
#define LCA_OK 0 // No errors occurred.
#define LCA_NO_ARGS 1 // LC_flags is a NULL pointer.
#define LCA_MALLOC_ERR 2 // malloc() returned a NULL pointer.

#define LCA_BAD_FLAG 3 // A malformed flag was supplied to the program.
#define LCA_VAR_RESET 4 // A variable was set twice on the command line.
#define LCA_NO_VAL 5 // No value was supplied to a flag that sets a variable.
#define LCA_BAD_VAL 6 // A malformed value was supplied.
#define LCA_LESS_VALS 7 // Fewer values were supplied than the flag accepts.
#define LCA_MORE_VALS 8 // More values were supplied than the flag accepts.
#define LCA_FUNC_ERR 9 // A user-defined function returned an error.

#define LCA_BAD_VAR_TYPE 10 // The specified flag var_type is invalid.
#define LCA_NULL_FORMAT_STR 11 // A NULL pointer was was given for sscanf.
```

`LCA_NO_ARGS` and `LCA_BAD_FLAG` mean that we caught non-fatal user errors while `LCA_MALLOC_ERR` will be passed on a failure to allocate memory. (This could be a result of memory scarcity, but is probably a result of something going very wrong in libc.) Everything else is a type of user error. All put together, an example program might look like the one sketched out below:

```c
LCa_flags_t flags[] = {
	{...},
	...
};

int main(int argc, char **argv) {
	LCa_flags = &flags[0];
	LCa_flags_length = sizeof(flags) / sizeof(LCa_flag_t);

	int ret = LCa_read(argc, argv);

	switch(ret) {
	case LCA_OK:
		break;

	default:
		/* Do something about an error. */
		...
	}

	...
}

...
```

It's worth noting that flagless arguments and the program name are passed along in the following variables: (Quoted from the headerfile.)

```c
extern char **LCa_flagless_args;
extern size_t LCa_flagless_args_length;

/* We also get the program name out of argv[0]. */
extern char *LCa_prog_name;
```

Lastly, if `LCa_read()` returned an error because a user specified function returned an error, then the information of which function and what it returned is specified in the following variables: (Quoted from the headerfile.)

```c
extern int (*LCa_err_function)();
extern int LCa_function_errno;
```

## The Command-Line Grammar

For making the grammar a bit simpler let's talk about the different types of flags and the way that they each behave individually, as well as some of the abbreviations of syntax that can arise as a result of them. Before we start, a beginning point is that a flag can either be long or short. A long flag is defined as a `--` followed by a series of ascii-characters that doesn't include the equals-to sign, while a short flag is a `-` followed by any ascii character that isn't the hyphen.

The following are examples of long and short flags: `--cat` / `-c`; `--Dog` / `-D`; `--h8xxor` / `-8`; `--%+)[}]+` / `-%`; etc.

### Argument-less flags.

Let `--cat, -c`, `--dog -d` and `--elephant, -e` be flags that don't need to be specified with any value to make sense. Then, you can actually combine the short flags into one compounded expression; the series `--cat --dog --elephant` means the same thing as `-cde`.

### Single-value flags.

Let `--ants / -a [VALUE]` and `--bats / -b [VALUE]` be flags that take one value respectively. Then, the following strings are all equivalent: `--ants [VALUE] --bats [VALUE]`, `--ants=[VALUE] --bats=[VALUE]`, `-a [VALUE] -b [VALUE`, `-a[VALUE] -b[VALUE]`.

When combined with argument-less flags, the following are the same: `--cat --ants=[VALUE] --dog --bats=[VALUE]` and `-ca[VALUE] -db[VALUE]`.

### Array-valued flags.

Let `--pigeons [VALUE1] [VALUE2] ...` be a flag that takes a series of arguments to make sense. If these values aren't strings, then the end of the values can either be inferred by the thing following them not making sense to scan or interpret as a value. (Say, for example, if we see a string or a flag while the values are supposed to be integers.) Or it can be specified by the string `--` being its own arg. If the values are strings, then the end of the array can only be interpreted by the string `--` being its own argument.

Incidentally, if you want to pass a `--` as a string to `--pigeons`, you would do it as such: `--pigeons=-- [OTHER VALUES]` or `-p-- [OTHER VALUES]`. This implies the next point, that the following mean the same things: `--pigeons [VALUE1] [VALUE2] [...]`, `--pigeons=[VALUE1] [VALUE2] [...]` and `-p[VALUE1] [VALUE2] [...]`.

### Flagless arguments.

Arguments following flags that either don't take values or arguments that don't make sense to analyse as values for a flag are called flagless arguments and get stored separately. Since some flags might be confused for things like filenames which are often flagless, the end of the flags can be signaled by a `--`. Note though, that if you're specifying both the end of an array of values and the end of the flags, then you will need two `--`s consecutively. For example: `--pigeons=[VALUE1] [VALUE2] -- -- Flagless1 Flagless2`.