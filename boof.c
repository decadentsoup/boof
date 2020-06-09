// Boolf### interpreter; implements http://samuelhughes.com/boof/
// Copyright (c) 2012, 2020 Megan Ruggiero. All rights reserved.
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
#include <string.h>
#include <getopt.h>

enum
{
   DATA_PART_BODY_SIZE = 1024,
   LOOP_EXPAND_SIZE = 16
};

struct data_part
{
   struct data_part *prev, *next;
   unsigned char body[DATA_PART_BODY_SIZE];
};

struct loop_pointer
{
   unsigned long line, column;
   size_t offset;
};

static const char *input_name;
static char *code;
static struct data_part *data;
static struct loop_pointer *loop_pointers;
static unsigned long line, column;

static void handle_exit(void);
static void parse_options(int argc, char **argv);
static size_t load_program(void);

int main(int argc, char **argv)
{
   size_t cursor = 0;
   unsigned char *memory = NULL;
   char input = 0, input_bits = 8;
   char output = 0, output_bits = 0;
   size_t offset = 0;
   size_t loop_max_depth = 0, loop_depth = 0;
   size_t program_size = 0;

   if (atexit(handle_exit))
      err(EXIT_FAILURE, "failed to register exit callback");

   parse_options(argc, argv);
   program_size = load_program();

   if (!(data = calloc(1, sizeof(struct data_part))))
      err(EXIT_FAILURE, "failed to allocate memory");

   memory = data->body;

   ++line;

   while (offset < program_size) {
      ++column;

      switch (code[offset]) {
         case '+': /* flip */
            memory[cursor / 8] ^= 1 << cursor % 8;
            break;
         case ',': /* read bit */
            if (input_bits > 7) {
               int c = getchar();
               input = c == EOF ? 0 : (char)c;
               input_bits = 0;
            }

            if (input & 1 << input_bits++)
               memory[cursor / 8] |= 1 << cursor % 8;
            else
               memory[cursor / 8] &= ~(1 << cursor % 8);

            break;
         case ';': /* write bit */
            if (memory[cursor / 8] & 1 << cursor % 8)
               output |= 1 << output_bits++;
            else
               output &= ~(1 << output_bits++);

            if (output_bits == 8) {
               putchar(output);
               output = 0;
               output_bits = 0;
            }

            break;
         case '<': /* select left */
            if (cursor > 0) {
               --cursor;
            } else {
               if (data->prev == NULL) {
                  data->prev = calloc(1, sizeof(struct data_part));

                  if (data->prev == NULL)
                     err(EXIT_FAILURE, "failed to allocate memory");

                  data->prev->next = data;
               }

               data = data->prev;
               memory = data->body;
               cursor = DATA_PART_BODY_SIZE * 8 - 1;
            }

            break;
         case '>': /* select right */
            if (cursor < DATA_PART_BODY_SIZE * 8 - 1) {
               ++cursor;
            } else {
               if (data->next == NULL) {
                  data->next = calloc(1, sizeof(struct data_part));

                  if (data->next == NULL)
                     err(EXIT_FAILURE, "failed to allocate memory");

                  data->next->prev = data;
               }

               data = data->next;
               memory = data->body;
               cursor = 0;
            }

            break;
         case '[': /* recursive loop start */
            if (loop_depth >= loop_max_depth) {
               loop_max_depth += LOOP_EXPAND_SIZE;
               loop_pointers = realloc(loop_pointers, loop_max_depth * sizeof(struct loop_pointer));
               if (loop_pointers == NULL)
                  err(EXIT_FAILURE, "failed to reallocate memory");
            }
            if (memory[cursor / 8] & 1 << cursor % 8) {
               loop_pointers[loop_depth].line = line;
               loop_pointers[loop_depth].column = column;
               loop_pointers[loop_depth++].offset = offset;
            } else {
               int depth = 0;

               while (offset++ < program_size) {
                  if (code[offset] == '[')
                     ++depth;
                  else if (code[offset] == ']' && depth-- == 0)
                     break;
               }
            }

            break;
         case ']': /* recursive loop end */
            if (loop_depth > 0) {
               if (memory[cursor / 8] & 1 << cursor % 8) {
                  line = loop_pointers[loop_depth - 1].line;
                  column = loop_pointers[loop_depth - 1].column;
                  offset = loop_pointers[loop_depth - 1].offset;
               } else {
                  --loop_depth;
               }
            }

            break;
         case '\n': /* update line information */
            ++line;
            column = 0;
            break;
      }

      ++offset;
   }

   putchar('\n');

   return EXIT_SUCCESS;
}

static void handle_exit()
{
   free(code);

   if (data != NULL) {
      struct data_part *node = data->prev;
      while (node != NULL) {
         struct data_part *temp = node;
         node = node->prev;
         free(temp);
      }

      node = data->next;
      while (node != NULL) {
         struct data_part *temp = node;
         node = node->next;
         free(temp);
      }

      free(data);
   }

   free(loop_pointers);
}

static void parse_options(int argc, char **argv)
{
   enum { OPT_HELP, OPT_VERSION };

   static const struct option options[] = {
      { "help", no_argument, 0, OPT_HELP },
      { "version", no_argument, 0, OPT_VERSION }
   };

   int c, failed = 0;

   while ((c = getopt_long(argc, argv, "", options, NULL)) != -1)
      switch (c) {
      case OPT_HELP:
         printf("Usage: %s [INPUT-FILE]\n", argv[0]);
         puts("");
         puts("Interpret a Boolf### program. If no input file is given, reads standard input.");
         puts("");
         puts("Options:");
         puts("  --help     print this help message and exit");
         puts("  --version  print version message and exit");
         exit(EXIT_SUCCESS);
      case OPT_VERSION:
         puts("boof " VERSION);
         exit(EXIT_SUCCESS);
      case '?':
         failed = 1;
         break;
      default:
         errx(EXIT_FAILURE, "getopt returned %i", c);
      }

   if (failed)
      exit(EXIT_FAILURE);

   while (optind < argc) {
      if (input_name)
         errx(EXIT_FAILURE, "too many parameters (see --help for usage)");

      if (!freopen((input_name = argv[optind++]), "rb", stdin))
         err(EXIT_FAILURE, input_name);
   }
}

static size_t load_program()
{
   enum { ALLOC_SIZE = 1024 };

   if (!input_name)
      input_name = "<stdin>";

   size_t program_size = 0, limit = 0;

   while (!feof(stdin)) {
      if (program_size >= limit) {
         limit += ALLOC_SIZE;
         code = realloc(code, limit);
         if (code == NULL) {
            fclose(stdin);
            err(EXIT_FAILURE, "failed to reallocate memory");
         }
      }

      program_size += fread(&code[limit - ALLOC_SIZE], 1, ALLOC_SIZE, stdin);

      if (ferror(stdin)) {
         fclose(stdin);
         err(EXIT_FAILURE, input_name);
      }
   }

   return program_size;
}
