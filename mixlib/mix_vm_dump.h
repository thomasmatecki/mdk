/* ---------------------- mix_vm_dump.h :
 * This file declares types and functions for dumping the contents
 * of a mix virtual machine.
 * ------------------------------------------------------------------
** Copyright (C) 2000, 2001, 2007 Free Software Foundation, Inc.
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
**
*/


#ifndef MIX_VM_DUMP_H
#define MIX_VM_DUMP_H

#include <stdio.h>
#include "mix_vm.h"

/* Dump context: a structure defining the properties of dumping */
typedef struct mix_dump_context_t mix_dump_context_t;
struct mix_dump_context_t
{
  /* flags activating dumps (see enum below) */
  guint32 options;
  /* range of addresses dumped: [begin,end) */
  mix_address_t begin;
  mix_address_t end;
  /* IO channel for dumping */
  FILE *channel;
};

/* Flags for activating dumps */
#define MIX_DUMP_NONE 0
#define MIX_DUMP_rA  1
#define MIX_DUMP_rX  (1<<1)
#define MIX_DUMP_rJ  (1<<2)
#define MIX_DUMP_rI1 (1<<3)
#define MIX_DUMP_rI2 (1<<4)
#define MIX_DUMP_rI3 (1<<5)
#define MIX_DUMP_rI4 (1<<6)
#define MIX_DUMP_rI5 (1<<7)
#define MIX_DUMP_rI6 (1<<8)
#define MIX_DUMP_rIa MIX_DUMP_rI1|MIX_DUMP_rI2|MIX_DUMP_rI3  \
		      |MIX_DUMP_rI4|MIX_DUMP_rI5|MIX_DUMP_rI6
#define MIX_DUMP_rALL MIX_DUMP_rA|MIX_DUMP_rX|MIX_DUMP_rJ|MIX_DUMP_rIa
#define MIX_DUMP_OVER (1<<9)
#define MIX_DUMP_CMP  (1<<10)
#define MIX_DUMP_CELLS (1<<11)
#define MIX_DUMP_ALL  MIX_DUMP_rALL|MIX_DUMP_OVER|MIX_DUMP_CMP|MIX_DUMP_CELLS
#define MIX_DUMP_ALL_NOMEM MIX_DUMP_rALL|MIX_DUMP_OVER|MIX_DUMP_CMP

/* Default output channel (stdout) */
#define MIX_DUMP_DEF_CHANNEL (stdout)

/* Create/destroy a dump context */
extern mix_dump_context_t *
mix_dump_context_new (FILE *fd, mix_address_t begin, mix_address_t end,
		      guint32 options);

extern void
mix_dump_context_delete(mix_dump_context_t *dc);

/* Modify an existing dump context */
#define mix_dump_context_add_opt(dc,opt) ((dc)->options |= (opt))
#define mix_dump_context_del_opt(dc,opt) ((dc)->options &= ~(opt))
#define mix_dump_context_set_opt(dc,opt) ((dc)->options = (opt))
#define mix_dump_context_range(dc,first,last) 	\
do {						\
  (dc)->begin = first;				\
  (dc)->end = last;				\
} while (FALSE)


/* Use the dump context */
extern void
mix_vm_dump(const mix_vm_t *vm, const mix_dump_context_t *dc);


#endif /* MIX_VM_DUMP_H */

