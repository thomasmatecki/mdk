## Process this file with automake to produce Makefile.in

# Copyright (C) 2000, 2001, 2003, 2004, 2006, 2008, 2013 Free Software
#               Foundation, Inc.
#
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without
# modifications, as long as this notice is preserved.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

SUBDIRS = img
SUFFIXES = .html

info_TEXINFOS = mdk.texi
mdk_TEXINFOS = mdk_intro.texi mdk_ack.texi mdk_tut.texi mdk_gstart.texi \
               mdk_mixvm.texi mdk_emacs.texi mdk_mixasm.texi mdk_bugs.texi \
               mdk_index.texi mdk_gmixvm.texi mdk_install.texi \
               mdk_mixguile.texi mdk_copying.texi mdk_findex.texi

EXTRA_DIST = MIX.DOC COPYING.MIX.DOC

gendocs: $(mdk_TEXINFOS)
	(cd $(top_srcdir)/doc && \
         ./gendocs.sh --email bug-mdk@gnu.org mdk 'GNU MDK Users Manual')
