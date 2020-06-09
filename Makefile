.POSIX:
.PHONY: all clean

CFLAGS = -DVERSION=\"git-`git rev-parse HEAD`\"

all: boof

boof: boof.c

clean:
	rm -f boof
