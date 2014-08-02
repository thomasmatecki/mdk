## Process this file with automake to produce Makefile.in

# Copyright (C) 2001, 2004, 2006, 2008, 2014 Free Software Foundation, Inc.
#
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without
# modifications, as long as this notice is preserved.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

EXTRA_DIST = mixgtk.glade

if MAKE_GUILE
GUILED = -DMAKE_GUILE
GUILEL = $(top_builddir)/mixguile/libmixguile.a
else
GUILED =
GUILEL =
endif

if DARWIN_SYSTEM
EDITOR_CMD = "/usr/bin/open -a TextEdit %s"
EXTRA_EXECD = -DEXTRA_EXEC_MIME_TYPE
else
EDITOR_CMD = "/usr/bin/xterm -e vi %s"
EXTRA_EXECD =
endif

if MAKE_GUI

pkgdata_DATA = mixgtk.glade

AM_CFLAGS += -I$(includedir) -I$(top_srcdir) \
             -DGLADE_FILE=\""$(pkgdatadir)/mixgtk.glade"\"\
             -DLOCAL_GLADE_FILE=\""$(srcdir)/mixgtk.glade"\"\
	     -DDEFAULT_EDITOR_CMD=\"$(EDITOR_CMD)\"\
	     -DDEFAULT_ASM_CMD="\"$(bindir)/mixasm %s"\"\
             $(GUILED) $(EXTRA_EXECD)

LDADD = $(top_builddir)/mixlib/libmix.a $(top_builddir)/lib/libreplace.a\
        $(GUILEL) $(INTLLIBS)

if ! DARWIN_SYSTEM
AM_LDFLAGS = -Wl,--export-dynamic
endif

bin_PROGRAMS = gmixvm
gmixvm_SOURCES = gmixvm.c mixgtk.h mixgtk.c \
                 mixgtk_config.h mixgtk_config.c \
                 mixgtk_gen_handlers.h mixgtk_gen_handlers.c \
		 mixgtk_external.h mixgtk_external.c \
		 mixgtk_cmd_dispatcher.h mixgtk_cmd_dispatcher.c \
                 mixgtk_widgets.h mixgtk_widgets.c \
                 mixgtk_device.h mixgtk_device.c \
                 mixgtk_mixvm.h mixgtk_mixvm.c \
                 mixgtk_mixal.h mixgtk_mixal.c \
                 mixgtk_input.h mixgtk_input.c \
		 mixgtk_fontsel.h mixgtk_fontsel.c \
                 mixgtk_wm.h mixgtk_wm.c
endif
