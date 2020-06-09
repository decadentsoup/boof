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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifdef USE_POSIX
#  include <unistd.h>
#endif

#ifdef USE_WINDOWS
#  include <windows.h>
#endif

#ifdef __STDC__
#  define PARAMS(params) params
#else
#  define PARAMS(params) ()
#endif

#if defined(__GCC__) || defined(__clang__)
#  define NORET __attribute__((noreturn))
#else
#  define NORET /* empty */
#endif

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

static const char *program_name = "?";
static const char *input_name = NULL;
static char *code = NULL;
static struct data_part *data = NULL;
static struct loop_pointer *loop_pointers = NULL;
static unsigned long line = 0, column = 0;

static void handle_exit PARAMS((void));
static void parse_options PARAMS((int argc, char **argv));
static size_t load_program PARAMS((void));
static void print_help PARAMS((void));
static void print_version PARAMS((void));
static const char *get_base_name PARAMS((const char *name));
static void error PARAMS((const char *message)) NORET;
static void errorp PARAMS((const char *message)) NORET;
static void abort_program PARAMS((void)) NORET;

int main(argc, argv)
   int argc;
   char **argv;
{
   size_t cursor = 0;
   unsigned char *memory = NULL;
   char input = 0, input_bits = 8;
   char output = 0, output_bits = 0;
   size_t offset = 0;
   size_t loop_max_depth = 0, loop_depth = 0;
   size_t program_size = 0;

   program_name = get_base_name(argv[0]);

   if (atexit(handle_exit) != 0) {
      errorp("failed to register exit callback");
   }

   parse_options(argc, argv);
   program_size = load_program();

   data = calloc(1, sizeof(struct data_part));
   if (data == NULL) {
      errorp("calloc");
   }

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
            if (input & 1 << input_bits++) {
               memory[cursor / 8] |= 1 << cursor % 8;
            }
            else {
               memory[cursor / 8] &= ~(1 << cursor % 8);
            }
            break;
         case ';': /* write bit */
            if (memory[cursor / 8] & 1 << cursor % 8) {
               output |= 1 << output_bits++;
            }
            else {
               output &= ~(1 << output_bits++);
            }
            if (output_bits == 8) {
               putchar(output);
               output = 0;
               output_bits = 0;
            }
            break;
         case '<': /* select left */
            if (cursor > 0) {
               --cursor;
            }
            else {
               if (data->prev == NULL) {
                  data->prev = calloc(1, sizeof(struct data_part));
                  if (data->prev == NULL) {
                     errorp("calloc");
                  }
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
            }
            else {
               if (data->next == NULL) {
                  data->next = calloc(1, sizeof(struct data_part));
                  if (data->next == NULL) {
                     errorp("calloc");
                  }
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
               if (loop_pointers == NULL) {
                  errorp("realloc");
               }
            }
            if (memory[cursor / 8] & 1 << cursor % 8) {
               loop_pointers[loop_depth].line = line;
               loop_pointers[loop_depth].column = column;
               loop_pointers[loop_depth++].offset = offset;
            }
            else {
               int depth = 0;
               while (offset++ < program_size) {
                  if (code[offset] == '[') {
                     ++depth;
                  }
                  else if (code[offset] == ']' && depth-- == 0) {
                     break;
                  }
               }
            }
            break;
         case ']': /* recursive loop end */
            if (loop_depth > 0) {
               if (memory[cursor / 8] & 1 << cursor % 8) {
                  line = loop_pointers[loop_depth - 1].line;
                  column = loop_pointers[loop_depth - 1].column;
                  offset = loop_pointers[loop_depth - 1].offset;
               }
               else {
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
   if (code != NULL) {
      free(code);
   }

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

   if (loop_pointers != NULL) {
      free(loop_pointers);
   }
}

static void parse_options(argc, argv)
   int argc;
   char **argv;
{
   int i = 1, end_of_args = 0;

   for (; i < argc; ++i) {
      if (argv[i][0] == '-' && !end_of_args) {
         if (argv[i][1] == '-') {
            if (argv[i][2] == 0) {
               end_of_args = 1;
            }
            else if (!strcmp(argv[i], "--help")) {
               print_help();
               exit(EXIT_SUCCESS);
            }
            else if (!strcmp(argv[i], "--version")) {
               print_version();
               exit(EXIT_SUCCESS);
            }
            else {
               error("unrecognized option (accepts either --help or --version)");
            }
         }
         else if (argv[i][1] == 'h') {
            print_help();
            exit(EXIT_SUCCESS);
         }
         else if (argv[i][1] == 'V') {
            print_version();
            exit(EXIT_SUCCESS);
         }
         else {
            error("unrecognized option (accepts either -h or -V)");
         }
      }
      else if (input_name == NULL) {
         input_name = argv[i];

         if (getenv("POSIXLY_CORRECT") != NULL) {
            end_of_args = 1;
         }
      }
      else {
         error("too many parameters (see --help for usage)");
      }
   }
}

static size_t load_program()
{
   size_t program_size = 0;
   FILE *input_file = NULL;

   if (input_name == NULL) {
      input_name = "<stdin>";
      input_file = stdin;
   }
   else {
      input_file = fopen(input_name, "r");
      if (input_file == NULL) {
         errorp(input_name);
      }
   }

   if (input_file != stdin && fseek(input_file, 0, SEEK_END) == 0) {
      long tell_result = ftell(input_file);
      if (tell_result < 0) {
         errorp(input_name);
      }

      program_size = (size_t)tell_result;

      code = malloc(program_size);
      if (code == NULL) {
         fclose(input_file);
         errorp("malloc");
      }

      if (fseek(input_file, 0, SEEK_SET) < 0) {
         fclose(input_file);
         errorp(input_name);
      }

      if (fread(code, program_size, 1, input_file) != 1) {
         fclose(input_file);
         errorp(input_name);
      }

      fclose(input_file);
   }
   else {
      size_t limit = 0;

      while (!feof(input_file)) {
         if (program_size >= limit) {
            limit += 512;
            code = realloc(code, limit);
            if (code == NULL) {
               fclose(input_file);
               errorp("realloc");
            }
         }

         program_size += fread(&code[limit - 512], 1, 512, input_file);

         if (ferror(input_file)) {
            fclose(input_file);
            errorp(input_name);
         }
      }
   }

   return program_size;
}

static void print_help()
{
   printf("Usage: %s [<input-name>]\n", program_name);
   puts("");
   puts("Interpret a Boolf### program. If no input file is given, reads \
standard input.");
   puts("");
   puts("Options:");
   puts("  -h, --help     print this help message and exit");
   puts("  -V, --version  print version message and exit");
}

static void print_version()
{
   puts("development");
}

static const char *get_base_name(name)
   const char *name;
{
   const char *base = strrchr(name, '/');

   if (base == NULL) {
      base = strrchr(name, '\\');

      if (base == NULL || base[1] == ':') {
         return name;
      }
   }

   return base + 1;
}

static void error(message)
   const char *message;
{
#ifdef USE_POSIX
   if (isatty(fileno(stderr))) {
      fprintf(stderr, "\033[1m%s: \033[31merror: \033[0;1m%s\033[0m\n",
              program_name, message);
      abort_program();
   }
#endif

#ifdef USE_WINDOWS
   HANDLE hstdout = GetStdHandle(STD_OUTPUT_HANDLE);
   SetConsoleTextAttribute(hstdout, FOREGROUND_RED|FOREGROUND_INTENSITY);
#endif

   fprintf(stderr, "%s: error: %s\n", program_name, message);

#ifdef USE_WINDOWS
   SetConsoleTextAttribute(hstdout, 0);
#endif

   abort_program();
}

static void errorp(message)
   const char *message;
{
   const char *error = strerror(errno);

#ifdef USE_WINDOWS
   HANDLE hstdout = GetStdHandle(STD_OUTPUT_HANDLE);
#endif

#ifdef USE_POSIX
   if (isatty(fileno(stderr))) {
      fprintf(stderr, "\033[1m%s: \033[31merror: \033[0;1m%s: %s\033[0m\n",
              program_name, message, error);
      abort_program();
   }
#endif

#ifdef USE_WINDOWS
   SetConsoleTextAttribute(hstdout, FOREGROUND_RED|FOREGROUND_INTENSITY);
#endif

   fprintf(stderr, "%s: error: %s: %s\n", program_name, message,
           error);

#ifdef USE_WINDOWS
   SetConsoleTextAttribute(hstdout, 0);
#endif

   abort_program();
}

static void abort_program()
{
#ifdef USE_WINDOWS
   HANDLE hstdout = INVALID_HANDLE;
#endif

   if (line == 0) {
      exit(EXIT_FAILURE);
   }

#ifdef USE_POSIX
   if (isatty(fileno(stderr))) {
      fprintf(stderr,
              "\033[1m%s: aborting at line %lu, column %lu\033[0m\n",
              program_name, line, column);
      exit(EXIT_FAILURE);
   }
#endif

#ifdef USE_WINDOWS
   hstdout = GetStdHandle(STD_OUTPUT_HANDLE);
   SetConsoleTextAttribute(hstdout, FOREGROUND_INTENSITY);
#endif

   fprintf(stderr, "%s: aborting at line %lu, column %lu\n", program_name,
           line, column);

#ifdef USE_WINDOWS
   SetConsoleTextAttribute(hstdout, 0);
#endif

   exit(EXIT_FAILURE);
}

/* vim: set sts=3 sw=3 et: */
