// Copyright (c) 2020 Megan Ruggiero. All rights reserved.
//
// Permission to use, copy, modify, and distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
// ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
// OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

static void parse_options(int, char **);

int
main(int argc, char **argv)
{
	int c;

	parse_options(argc, argv);

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

	return EXIT_SUCCESS;
}

static void
parse_options(int argc, char **argv)
{
	enum { OPT_HELP, OPT_VERSION, OPT_OUTPUT = 'o' };

	static const struct option options[] = {
		{ "help", no_argument, 0, OPT_HELP },
		{ "version", no_argument, 0, OPT_VERSION },
		{ "output", no_argument, 0, OPT_OUTPUT }
	};

	int c;

	while ((c = getopt_long(argc, argv, "o:", options, NULL)) != -1)
		switch (c) {
		case OPT_HELP:
			printf("Usage: %s [-o OUTPUT-FILE] INPUT-FILE\n\n", argv[0]);
			puts("Options:");
			puts("  -o, --output   set output file name (default: stdout)");
			puts("      --help     print help message");
			puts("      --version  print version message");
			exit(EXIT_SUCCESS);
		case OPT_VERSION:
			puts("brain2bool " VERSION);
			exit(EXIT_SUCCESS);
		case OPT_OUTPUT:
			if (!freopen(optarg, "wb", stdout))
				err(EXIT_FAILURE, "%s", optarg);
			break;
		case '?':
			exit(EXIT_FAILURE);
		default:
			errx(EXIT_FAILURE, "getopt returned %c", c);
		}

	if (optind < argc && !freopen(argv[optind], "rb", stdin))
		err(EXIT_FAILURE, "%s", argv[optind]);
}
