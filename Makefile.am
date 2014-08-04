## Process this file with automake to produce Makefile.in

# Copyright (C) 2000, 2001, 2004, 2006, 2013, 2014 Free Software Foundation, Inc.
#
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without
# modifications, as long as this notice is preserved.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

EXTRA_DIST = config.rpath ABOUT-NLS intltool-extract.in intltool-merge.in \
             autogen.sh intltool-update.in ChangeLog.1 ChangeLog.2
SUBDIRS = po intl doc lib mixlib mixguile mixutils mixgtk misc samples

distclean-local:
	rm -f intltool-extract
	rm -f intltool-merge
	rm -f intltool-update

dist-hook:
	$(SHELL) $(top_srcdir)/ChangeLog > $(top_distdir)/ChangeLog

docdir = $(top_builddir)/doc

ACLOCAL_AMFLAGS = -I m4
