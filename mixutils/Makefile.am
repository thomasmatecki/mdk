## Process this file with automake to produce Makefile.in

# Copyright (C) 2000, 2001, 2002 Free Software Foundation, Inc.
#  
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without 
# modifications, as long as this notice is preserved.
# 
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

# $Id: Makefile.am,v 1.9 2002/04/08 00:30:34 jao Exp $

if MAKE_GUILE
INCLUDES =  -I$(includedir) -I$(top_srcdir) -DMAKE_GUILE
else
INCLUDES = -I$(includedir) -I$(top_srcdir)
endif

LDADD = $(top_builddir)/mixlib/libmix.a $(top_builddir)/lib/libreplace.a \
        $(top_builddir)/mixguile/libmixguile.a $(INTLLIBS)

bin_PROGRAMS = mixasm mixvm
mixasm_SOURCES = mixasm.c mixasm_comp.h mixasm_comp.c
mixvm_SOURCES = mixvm.c mixvm_loop.h mixvm_loop.c mixvm_command.h \
                mixvm_command.c
