# Boolf### interpreter

This is a Boolf### interpreter that I wrote for fun. Boof is a derivative of
the Turing tarpit esoteric programming language Brainf### designed around bit
manipulating instead of whole integers.

The Boof language is described here: http://samuelhughes.com/boof/

To compile the program, simply pass interp.c through any C89-compatible
compiler, such as GCC or Clang. Execute `build.sh` to run through GCC, Clang,
and Tiny C Compiler, skipping those that are unavailable. Run `build.sh clean`
to delete the generated files.

Multiple examples are provided with this distribution and may be passed to the
interpreter like so:

	./interp examples/examplename.boof

When no input name is given, the interpreter will read until the first end of
file indicator, then continue reading for data.
