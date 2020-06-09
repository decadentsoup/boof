#include <stdio.h>

int
main(int argc, char **argv)
{
	int c;

	while ((c = getchar()) != EOF)
		switch (c) {
		case '+': puts(">[>]+<[+<]>>>>>>>>>[+]<<<<<<<<<"); break;
		case '-': puts(">>>>>>>>>+<<<<<<<<+[>+]<[<]>>>>>>>>>[+]<<<<<<<<<"); break;
		case '<': puts("<<<<<<<<<"); break;
		case '>': puts(">>>>>>>>>"); break;
		case ',': puts(">,>,>,>,>,>,>,>,<<<<<<<<"); break;
		case '.': puts(">;>;>;>;>;>;>;>;<<<<<<<<"); break;
		case '[': puts(">>>>>>>>>+<<<<<<<<+[>+]<[<]>>>>>>>>>[+<<<<<<<<[>]+<[+<]"); break;
		case ']': puts(">>>>>>>>>+<<<<<<<<+[>+]<[<]>>>>>>>>>]<[+<]"); break;
		}

	return 0;
}
