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
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include "defs.h"

#define RECORD_MARK   0x00000000  /* Tape mark. */
#define RECORD_LMASK  0x7FFFFFFF  /* Record length mask. */
#define RECORD_ERR    0x80000000  /* Error. */
#define RECORD_EMASK  0x00FFFFFF  /* Error mask. */
#define RECORD_EOM    0xFFFFFFFF  /* End of medium. */

static int marks = 0;

static void
write_reclen (int fd, size_t n)
{
  unsigned char size[4];

  size[0] = n & 0377;
  size[1] = (n >> 8) & 0377;
  size[2] = (n >> 16) & 0377;
  size[3] = (n >> 24) & 0377;

  n = write (fd, size, 4);
  if (n == -1)
    fprintf (stderr, "Write error: %s\n", strerror (errno));
}

void
write_mark (int fd)
{
  marks++;
  write_reclen (fd, RECORD_MARK);
}

void
write_record (int fd, void *buffer, size_t n)
{
  int m;
  n &= RECORD_LMASK;
  if (n == 0)
    {
      fprintf (stderr, "Can't write empty record.\n");
      return;
    }
  marks = 0;
  write_reclen (fd, n);
  m = write (fd, buffer, n);
  if (m == -1)
    fprintf (stderr, "Write error: %s\n", strerror (errno));
  write_reclen (fd, n);
}

void
write_eot (int fd)
{
  int i;
  for (i = marks; i < 2; i++)
    write_mark (fd);
}

void
write_eom (int fd)
{
  write_reclen (fd, RECORD_EOM);
}

void
write_error (int fd, unsigned error)
{
  error &= RECORD_EMASK;
  write_reclen (fd, error | RECORD_ERR);
}
