#!/usr/bin/env sh

if [ "$1" != clean ]; then
   clang -Werror -Weverything interp.c -o interp-clang -O0 -g -DUSE_POSIX
   clang -Werror -Weverything -ansi -pedantic interp.c -o interp-clang-ansi -O0 -g
   gcc -Werror -Wall -Wextra interp.c -o interp-gcc -O0 -g -DUSE_POSIX
   gcc -Werror -Wall -Wextra -ansi -pedantic interp.c -o interp-gcc-ansi -O0 -g
   tcc interp.c -o interp-tcc -g
else
   rm -f interp-clang interp-clang-ansi interp-gcc interp-gcc-ansi interp-tcc
fi

# vim: set sts=3 sw=3 et:
