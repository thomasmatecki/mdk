/* ---------------------- xmix_vm.h :
 * This file contains internal declarations used in the implementation
 * of the mix_vm_t type.
 * ------------------------------------------------------------------
 * Copyright (C) 2000, 2002, 2003, 2004, 2006, 2007 Free Software Foundation, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */


#ifndef XMIX_VM_H
#define XMIX_VM_H

#include <string.h>

#include "mix_symbol_table.h"
#include "mix_device.h"
#include "mix_src_file.h"
#include "mix_predicate_list.h"
#include "mix_vm.h"

/* The mix_vm_t type */
enum {
  IREG_NO_ = 6,
  BD_NO_ = 21,
  MEM_CELLS_NO_ = MIX_VM_CELL_NO,
  MEM_CELLS_MAX_ = MIX_VM_CELL_NO - 1
};

struct mix_vm_t
{
  mix_word_t reg[IREG_NO_+3];
  mix_word_t cell[MEM_CELLS_NO_];
  gboolean overflow;
  mix_cmpflag_t cmpflag;
  mix_short_t loc_count;
  mix_vm_status_t status;
  mix_vm_error_t last_error;
  mix_device_t * devices[BD_NO_];
  mix_address_t start_addr;	/* start address of loaded file */
  GTree *line_table;		/* source line no -> address  */
  GTree *address_table;		/* adress -> source line no */
  gint8 bp[MEM_CELLS_NO_/8];	/* each bit signals a break point */
  mix_vm_clock_t *clock;		/* the vm clock */
  mix_symbol_table_t *symbol_table;
  mix_src_file_t *src_file;	/* source of last loaded code file */
  mix_device_factory_t factory; /* the factory for new devices */
  mix_predicate_list_t *pred_list; /* predicates for conditional bps */
  GSList *address_list;		/* list of executed addresses */
};

/* Macros for accessing/modifying the above structure.
 * Warning: the arguments of these macros must not have side-effects.
 */
#define IOK_(idx) ( (idx) > 0 && (idx) < IREG_NO_+1 )
#define MEMOK_(addr) ( mix_short_is_positive(addr) && (addr) < MEM_CELLS_NO_ )
#define REGOK_(r) ( (r) >= 0 && (r) < IREG_NO_ + 3 )

enum { A_ = 0, X_, J_, I1_, I2_, I3_, I4_, I5_, I6_ };

#define get_reg_(vm, r) ((vm)->reg[r])
#define get_rA_(vm)  get_reg_(vm, A_)
#define get_rX_(vm)  get_reg_(vm, X_)
#define get_rJ_(vm)  get_reg_(vm, J_)
#define get_rI_(vm,idx) get_reg_(vm, I1_ + (idx) - 1)
#define get_cell_(vm,addr) ( MEMOK_(addr) ? vm->cell[addr] : MIX_WORD_ZERO )
#define get_cell_ptr_(vm,addr) ( MEMOK_(addr) ? (vm->cell) + addr : NULL )
#define get_cmp_(vm) (vm->cmpflag)
#define get_over_(vm) (vm->overflow)
#define get_loc_(vm)  (vm->loc_count)
#define get_clock_(vm) (vm->clock)
#define get_pred_list_(vm) (vm->pred_list)
#define get_address_list_(vm) (vm->address_list)
#define get_status_(vm) (vm->status)
#define get_last_error_(vm) (vm->last_error)
#define set_last_error_(vm,error) ((vm)->last_error = (error))

#define set_reg_(vm,r,x)			\
do {						\
  if ( REGOK_(r) ) vm->reg[r] = (x);		\
} while (FALSE)

#define set_rA_(vm,x)  set_reg_(vm,A_,x)
#define set_rX_(vm,x)  set_reg_(vm,X_,x)
#define set_rJ_(vm,x)  set_reg_(vm,J_,(x)&MIX_SHORT_MAX)
#define set_rI_(vm,idx,x)  set_reg_(vm,(idx) + I1_ - 1,x)

#define set_cell_(vm,addr,x) 			\
do {						\
  if ( MEMOK_(addr) ) (vm)->cell[addr] = (x);	\
} while (FALSE)

#define set_cmp_(vm,x) (vm)->cmpflag = (x)
#define set_over_(vm,x) (vm)->overflow = (x)
#define set_loc_(vm,x)  (vm)->loc_count = (MEMOK_(x)? (x) : MIX_SHORT_ZERO)

#define set_status_(vm,s) ((vm)->status = (s))
#define is_halted_(vm) ((vm)->status == MIX_VM_HALT)
#define halt_(vm,val) ((vm)->status = (val)? MIX_VM_HALT : MIX_VM_RUNNING)

#define inc_loc_(vm)				\
do {						\
  if (++(vm->loc_count) == MEM_CELLS_NO_)	\
    { vm->loc_count--; halt_(vm, TRUE); }	\
} while(FALSE)

#define set_start_(vm,val) ((vm)->start_addr = (val))
#define reset_loc_(vm) set_loc_ (vm, vm->start_addr)
#define update_time_(vm,ins) mix_vm_clock_add_lapse (get_clock_(vm), ins)

/* Breakpoints handling */
#define bp_clear_all_(vm)	memset (vm->bp, 0, MEM_CELLS_NO_/8)
#define bp_set_(vm,addr)	vm->bp[(addr)>>3] |= 1 << ((addr)&7)
#define bp_clear_(vm,addr)	vm->bp[(addr)>>3] &= ~(1 << ((addr)&7))
#define bp_is_set_(vm,addr)	vm->bp[(addr)>>3] & (1 << ((addr)&7))

/* Instruction handlers */
typedef gboolean (*ins_handler_t_)(mix_vm_t *,const mix_ins_t *);

extern ins_handler_t_ ins_handlers_[MIX_BYTE_MAX + 1];


#endif /* XMIX_VM_H */

