.POSIX:
.PHONY: all clean

all: boof brain2bool

boof: boof.c

brain2bool: brain2bool.c

clean:
	rm -f boof brain2bool

.c:
	$(CC) $(CFLAGS) $(LDFLAGS) -DVERSION=\"git-`git rev-parse HEAD`\" -o $@ $<
