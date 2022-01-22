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

#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mtio.h>
#include <sys/ioctl.h>

#include "defs.h"

#if !defined(MTSETBLK) && defined(MTSETBSIZ)
#define MTSETBLK MTSETBSIZ
#endif
#if !defined(MTSETDENSITY) && defined(MTSETDNSTY)
#define MTSETDENSITY MTSETDNSTY
#endif

static struct mtop mt_weof = { MTWEOF, 1 };
static struct mtop mt_setblk = { MTSETBLK, 0 };
static struct mtop mt_setden = { MTSETDENSITY, 0x02 };
static int marks;

int
open_tape (const char *device)
{
  int fd;

  fd = open (device, O_RDONLY);
  if (fd == -1)
    {
      fprintf (stderr, "Couldn't open %s: %s\n", device, strerror (errno));
      exit (1);
    }

  if (ioctl (fd, MTIOCTOP, &mt_setblk) == -1)
    {
      fprintf (stderr, "Couldn't set variable block size: %s\n",
               strerror (errno));
      exit (1);
    }

  marks = 0;
  return fd;
}

int
read_record (int fd, void *buffer, size_t n)
{
  int m;
  m = read (fd, buffer, n);
  return m;
}

void
tape_write_mark (int fd)
{
  if (ioctl (fd, MTIOCTOP, &mt_weof) == -1)
    fprintf (stderr, "Error writing mark: %s\n", strerror (errno));
  marks++;
}

void
tape_write_record (int fd, void *buffer, size_t n)
{
  int m;
  m = write (fd, buffer, n);
  if (m == -1)
    fprintf (stderr, "Error writing record: %s\n", strerror (errno));
  marks = 0;
}

void
tape_write_eot (int fd)
{
  int i;
  for (i = marks; i < 2; i++)
    tape_write_mark (fd);
}

const char *
tape_drive (int fd)
{
  struct mtget m;

  if (ioctl (fd, MTIOCGET, &m) == -1)
    {
      fprintf (stderr, "Couldn't get tape status: %s\n", strerror (errno));
      return "unknown";
    }

  switch (m.mt_type)
    {
#ifdef MT_ISQIC02
    case MT_ISQIC02: return "Generic QIC-02 tape streamer";
#endif
#ifdef MT_ISWT5150
    case MT_ISWT5150: return "Wangtek 5150EQ, QIC-150, QIC-02";
#endif
#ifdef MT_ISARCHIVE_5945L2
    case MT_ISARCHIVE_5945L2: return "Archive 5945L-2, QIC-24, QIC-02?. ";
#endif
#ifdef MT_ISCMSJ500
    case MT_ISCMSJ500: return "CMS Jumbo 500 (QIC-02?)";
#endif
#ifdef MT_ISTDC3610
    case MT_ISTDC3610: return "Tandberg 6310, QIC-24";
#endif
#ifdef MT_ISARCHIVE_VP60I
    case MT_ISARCHIVE_VP60I: return "Archive VP60i, QIC-02";
#endif
#ifdef MT_ISARCHIVE_2150L
    case MT_ISARCHIVE_2150L: return "Archive Viper 2150L";
#endif
#ifdef MT_ISARCHIVE_2060L
    case MT_ISARCHIVE_2060L: return "Archive Viper 2060L";
#endif
#ifdef MT_ISARCHIVESC499
    case MT_ISARCHIVESC499: return "Archive SC-499 QIC-36 controller. ";
#endif
#ifdef MT_ISQIC02_ALL_FEATURES
    case MT_ISQIC02_ALL_FEATURES: return "Generic QIC-02 with all features. ";
#endif
#ifdef MT_ISWT5099EEN24
    case MT_ISWT5099EEN24: return "Wangtek 5099-een24, 60MB, QIC-24. ";
#endif
#ifdef MT_ISTEAC_MT2ST
    case MT_ISTEAC_MT2ST: return "Teac MT-2ST ";
#endif
#ifdef MT_ISEVEREX_FT40A
    case MT_ISEVEREX_FT40A: return "Everex FT40A (QIC-40)";
#endif
#ifdef MT_ISDDS1
    case MT_ISDDS1: return "DDS device without partitions";
#endif
#ifdef MT_ISDDS2
    case MT_ISDDS2: return "DDS device with partitions";
#endif
#ifdef MT_ISSCSI1
    case MT_ISSCSI1: return "Generic ANSI SCSI-1 tape unit";
#endif
#ifdef MT_ISSCSI2
    case MT_ISSCSI2: return "Generic ANSI SCSI-2 tape unit";
#endif
#ifdef MT_ISTS
    case MT_ISTS: return "TS-11";
#endif
#ifdef MT_ISHT
    case MT_ISHT: return "TM03 Massbus: TE16, TU45, TU77";
#endif
#ifdef MT_ISTM
    case MT_ISTM: return "TM11/TE10 Unibus";
#endif
#ifdef MT_ISMT
    case MT_ISMT: return "TM78/TU78 Massbus";
#endif
#ifdef MT_ISUT
    case MT_ISUT: return "SI TU-45 emulation on Unibus";
#endif
#ifdef MT_ISCPC
    case MT_ISCPC: return "SUN";
#endif
#ifdef MT_ISAR
    case MT_ISAR: return "SUN, also generic SCSI";
#endif
#ifdef MT_ISTMSCP
    case MT_ISTMSCP: return "DEC TMSCP protocol (TU81, TK50)";
#endif
#ifdef MT_ISCY
    case MT_ISCY: return "CCI Cipher";
#endif
#ifdef MT_ISCT
    case MT_ISCT: return "HP 1/4 tape";
#endif
#ifdef MT_ISFHP
    case MT_ISFHP: return "HP 7980 1/2 tape";
#endif
#if defined(MT_ISEXABYTE) && !defined(MT_ISEXA8200)
    case MT_ISEXABYTE: return "Exabyte";
#endif
#ifdef MT_ISEXA8200
    case MT_ISEXA8200: return "Exabyte EXB-8200";
#endif
#ifdef MT_ISEXA8500
    case MT_ISEXA8500: return "Exabyte EXB-8500";
#endif
#ifdef MT_ISVIPER1
    case MT_ISVIPER1: return "Archive Viper-150";
#endif
#ifdef MT_ISPYTHON
    case MT_ISPYTHON: return "Archive Python (DAT)";
#endif
#ifdef MT_ISHPDAT
    case MT_ISHPDAT: return "HP 35450A DAT drive";
#endif
#ifdef MT_ISWANGTEK
    case MT_ISWANGTEK: return "WANGTEK 5150ES";
#endif
#ifdef MT_ISCALIPER
    case MT_ISCALIPER: return "Caliper CP150";
#endif
#ifdef MT_ISWTEK5099
    case MT_ISWTEK5099: return "WANGTEK 5099ES";
#endif
#ifdef MT_ISVIPER2525
    case MT_ISVIPER2525: return "Archive Viper 2525";
#endif
#if 0
#ifdef MT_ISMFOUR
    case MT_ISMFOUR: return "M4 Data 1/2 9track drive";
#endif
#ifdef MT_ISTK50
    case MT_ISTK50: return "DEC SCSI TK50";
#endif
#ifdef MT_ISMT02
    case MT_ISMT02: return "Emulex MT02 SCSI tape controller";
#endif
#endif
    default: return "unknown";
    }
}
