## Process this file with automake to produce Makefile.in

# Copyright (C) 2000, 2001, 2006, 2014, 2019 Free Software Foundation, Inc.
#
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without
# modifications, as long as this notice is preserved.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.


SUBDIRS = testsuite
AM_CFLAGS += -I$(includedir) -I$(top_srcdir) -DG_LOG_DOMAIN=\"libmix\"

CLEANFILES = mix_scanner.c mix_eval_scanner.c

noinst_LIBRARIES = libmix.a
libmix_a_SOURCES = gettext.h \
                   completion.h completion.c \
                   mix.h mix.c \
		   mix_types.h mix_types.c \
		   mix_ins.h mix_ins.c     \
	           mix_vm.h mix_vm.c xmix_vm.h xmix_vm.c \
		   mix_vm_dump.h mix_vm_dump.c \
		   mix_io.h mix_io.c xmix_io.h xmix_io.c \
		   mix_symbol_table.h mix_symbol_table.c \
		   mix_file.h mix_file.c \
                   mix_code_file.h mix_code_file.c \
		   mix_parser.h xmix_parser.h mix_parser.c mix_scanner.l \
                   mix_device.h mix_device.c xmix_device.h xmix_device.c \
                   mix_eval.h mix_eval.c xmix_eval.h mix_eval_scanner.l \
		   mix_src_file.c mix_src_file.h \
                   mix_vm_clock.c mix_vm_clock.h \
		   mix_vm_command.c mix_vm_command.h \
		   xmix_vm_command.c xmix_vm_command.h \
                   xmix_vm_handlers.h xmix_vm_handlers.c \
		   mix_config.c mix_config.h \
                   mix_predicate.c mix_predicate.h \
                   mix_predicate_list.c mix_predicate_list.h
