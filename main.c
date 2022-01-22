/* Copyright (C) 2022 Lars Brinkhoff <lars@nocrew.org>

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "defs.h"

#define SIZE 640*1024 /* 640K ought to be enough for anyone. */
static unsigned char record[SIZE];

static int errors = 0;

static void
copy_record (int fd)
{
  int n;
  n = read_record (fd, record, SIZE);
  if (n == -1)
    {
      if (errors == 20)
        {
          fprintf (stderr, "Guessing physical end of tape.\n");
          write_eom (1);
          exit (0);
        }
      else
        {
          fprintf (stderr, "Read error: %s\n", strerror (errno));
          write_error (1, errno);
          errors++;
        }
    }
  else if (n == 0)
    {
      fprintf (stderr, "Tape mark.\n");
      errors = 0;
      write_mark (1);
    }
  else
    {
      fprintf (stderr, "Record: %d frames.\n", n);
      errors = 0;
      write_record (1, record, n);
    }
}

static void
readtape (const char *device)
{
  int fd;
  fd = open_tape (device);
  fprintf (stderr, "Tape drive: %s\n", tape_drive (fd));
  for (;;)
    copy_record (fd);
}

static void
usage (const char *name)
{
  fprintf (stderr, "Usage: %s device [output]\n", name);
  exit (1);
}

int
main (int argc, char **argv)
{
  if (argc < 2)
    usage (argv[0]);
  if (argc == 3)
    {
      close (1);
      open (argv[2], O_WRONLY);
    }

  readtape (argv[1]);
  return 0;
}
