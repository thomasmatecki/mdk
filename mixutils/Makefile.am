## Process this file with automake to produce Makefile.in

# Copyright (C) 2000, 2001, 2002, 2006, 2014 Free Software Foundation, Inc.
#
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without
# modifications, as long as this notice is preserved.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.


LDADD = $(top_builddir)/mixlib/libmix.a $(top_builddir)/lib/libreplace.a \
        $(INTLLIBS)

if MAKE_GUILE
AM_CFLAGS +=  -I$(includedir) -I$(top_srcdir) -DMAKE_GUILE
LDADD += $(top_builddir)/mixguile/libmixguile.a
else
AM_CFLAGS += -I$(includedir) -I$(top_srcdir)
endif

bin_PROGRAMS = mixasm mixvm
mixasm_SOURCES = mixasm.c mixasm_comp.h mixasm_comp.c
mixvm_SOURCES = mixvm.c mixvm_loop.h mixvm_loop.c mixvm_command.h \
                mixvm_command.c
