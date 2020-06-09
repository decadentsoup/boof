.POSIX:
.PHONY: all clean

all: boof

boof: boof.c
	$(CC) $(CFLAGS) $(LDFLAGS) -DVERSION=\"git-`git rev-parse HEAD`\" -o $@ $<

clean:
	rm -f boof
