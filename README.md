# Project Alias

A System Programming Language

## Documentation

[Alias Language Reference](https://htmlpreview.github.io/?https://github.com/ParfenovIgor/alias-c/blob/main/docs/langref.html)

[Altlib Reference](https://htmlpreview.github.io/?https://github.com/ParfenovIgor/alias-c/blob/main/docs/altlibref.html)

## Installation

 * building from source
 * flakes.nix
 * download a pre-built binary

## Building from Source

Required dependencies:

 * gcc
 * ld
 * nasm
 * make

Build options:

 * `make compiler` - build the compiler. The compiler will appear in `build/calias`.
 * `make altlib` - build the compiler and altlib. Public version of altlib will appear in `build/altlib_ext`.
 * `make test` - build the compiler and altlib, run tests except performance tests.
 * `make perftest` - build the compiler and altlib, run performance tests.

For further usage preferably add a new environmental variable:

 * `ALTLIB` - **absolute** path to `build/altlib_ext`

## Get `calias` using Nix

Required dependencies:

* [Nix](https://nixos.org)
    * Use [this](https://github.com/DeterminateSystems/nix-installer) or [this](https://nixos.org/download/) way to install Nix

Enter the development shell for `calias`:

```
nix develop github:ParfenovIgor/alias-c
```

Check that `calias` is present:

```
calias
```

Check that `ALTLIB` is set:

```
echo "$ALTLIB"
```

## Example

*Note*. Since the compiler is in the development process, writing even the simplest programs is not trivial. This purpose of this example is only a demonstration. Read the simple and working alias project on [Alias Example Project](https://github.com/ParfenovIgor/alias-example-project).

### Simple alias program

```
include altlib."posix.al"
include altlib."stdio.al"
include altlib."test_allocator.al"

func ^._start() -> #V {
    def allocator #TestAllocator
    eval allocator&.init(1024)
    defer eval allocator&.deinit()

    def buffer := allocator&.alloc(256) as #1C
    eval reads_(buffer)
    eval puts_(buffer)

    eval posix_exit(0)
}
```

### Invoking a compilation

*Note*. `calias` can only compile, but not link (linking is very limited). You need to use `ld` instead.

`calias -c main.al -o main.o -i altlib $ALTLIB/include/`

## Contributing

[Contributing](https://github.com/ParfenovIgor/alias-c/wiki/Contributing)  page on the wiki.
