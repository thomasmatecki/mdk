## Process this file with automake to produce Makefile.in

# Copyright (C) 2001 Free Software Foundation, Inc. 
#  
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without 
# modifications, as long as this notice is preserved.
# 
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

# $Id: Makefile.am,v 1.6 2001/09/30 20:29:30 jao Exp $

SCM_FILES = mixguile.scm mixguile-commands.scm mixguile-vm-stat.scm
EXTRA_DIST = $(SCM_FILES)

noinst_LIBRARIES = libmixguile.a

if MAKE_GUILE

SCM_PATHS = -DSCM_FILE=\""$(pkgdatadir)/mixguile.scm"\"\
            -DLOCAL_SCM_FILE=\"mixguile.scm\"

pkgdata_DATA = $(SCM_FILES)
INCLUDES = -I$(includedir) -I$(top_srcdir) $(SCM_PATHS)
libmixguile_a_INCLUDES = -I$(includedir) -I$(top_srcdir) \
                         -DG_LOG_DOMAIN=\"libmixguile\"
libmixguile_a_SOURCES = mixguile.h mixguile.c \
                        mixguile_cmd_dispatcher.h mixguile_cmd_dispatcher.c \
                        xmixguile_cmd_dispatcher.h xmixguile_cmd_dispatcher.c

bin_PROGRAMS = mixguile
mixguile_LDADD = $(top_builddir)/mixlib/libmix.a \
                 $(top_builddir)/lib/libreplace.a \
                 $(top_builddir)/mixguile/libmixguile.a $(INTLLIBS)
mixguile_SOURCES = mixguile_main.c 

else

libmixguile_a_SOURCES =

endif
