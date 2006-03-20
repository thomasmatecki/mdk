## Process this file with automake to produce Makefile.in

# Copyright (C) 2000, 2001 Free Software Foundation, Inc.
#  
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without 
# modifications, as long as this notice is preserved.
# 
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

# $Id: Makefile.am,v 1.2 2001/09/29 01:53:51 jao Exp $

INCLUDES =  -I$(includedir) -I$(top_srcdir) -I$(top_srcdir)/mixlib
LDADD = $(top_builddir)/mixlib/libmix.a 

check_PROGRAMS = mixtypest mixinstest mixvminstest mixparsertest mixdevtest mixevaltest
TESTS = $(check_PROGRAMS)

mixtypest_SOURCES = test.h mix_types_t.c
mixinstest_SOURCES = test.h mix_ins_t.c
mixvminstest_SOURCES = test.h mix_vm_ins_t.c
mixparsertest_SOURCES = test.h mix_parser_t.c
mixdevtest_SOURCES = test.h mix_device_t.c
mixevaltest_SOURCES = test.h mix_eval_t.c






