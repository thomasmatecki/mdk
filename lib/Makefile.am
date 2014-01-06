## Process this file with automake to produce Makefile.in

# Copyright (C) 2001, 2014 Free Software Foundation, Inc.
#
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without
# modifications, as long as this notice is preserved.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

EXTRA_DIST = getopt_long.c getopt.h

noinst_LIBRARIES = libreplace.a
if DARWIN_SYSTEM
libreplace_a_SOURCES = darwin.c
else
libreplace_a_SOURCES =
endif
libreplace_a_LIBADD = @LIBOBJS@
