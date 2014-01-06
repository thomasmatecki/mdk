## Process this file with automake to produce Makefile.in

# Copyright (C) 2000, 2001, 2006, 2014 Free Software Foundation, Inc.
#
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without
# modifications, as long as this notice is preserved.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.


AM_CFLAGS +=  -I$(includedir) -I$(top_srcdir) -I$(top_srcdir)/mixlib
LDADD = $(top_builddir)/mixlib/libmix.a

check_PROGRAMS = mixtypest mixinstest mixvminstest mixparsertest mixdevtest mixevaltest
TESTS = $(check_PROGRAMS)

MIXAL_FILES= "\"tests/bt\",\"tests/cbp\",\"tests/stress0\",\"tests/stress1\",\"tests/stress2\",\"tests/stress4\",\"tests/stress5\",\"tests/stress6\",\"tests/ldan\",\"tests/lockonw\",\"tests/negwrite\",\"hello\",\"echo\",\"primes\",\"isains\""

AM_CFLAGS += -DMIX_TEST_MIXAL_FILES=$(MIXAL_FILES) -DMIX_TEST_SAMPLES_DIR="\"$(top_srcdir)/samples\""

mixtypest_SOURCES = test.h mix_types_t.c
mixinstest_SOURCES = test.h mix_ins_t.c
mixvminstest_SOURCES = test.h mix_vm_ins_t.c
mixdevtest_SOURCES = test.h mix_device_t.c
mixevaltest_SOURCES = test.h mix_eval_t.c
mixparsertest_SOURCES = test.h mix_parser_t.c

distclean-local:
	rm -f *.mix
