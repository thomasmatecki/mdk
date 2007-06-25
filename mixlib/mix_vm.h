/* ---------------------- mix_vm.h :
 * Types and functions implementing the MIX virtual machine
 * ------------------------------------------------------------------
 * Copyright (C) 2000, 2001, 2004, 2007 Free Software Foundation, Inc.
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


#ifndef MIX_VM_H
#define MIX_VM_H

#include "mix_types.h"
#include "mix_ins.h"
#include "mix_device.h"
#include "mix_code_file.h"
#include "mix_src_file.h"
#include "mix_symbol_table.h"
#include "mix_vm_clock.h"

/* Comparison flag */
typedef enum { mix_LESS, mix_EQ, mix_GREAT } mix_cmpflag_t;

/* Number of memory cells in the virtual machine */
enum { MIX_VM_CELL_NO = 4000 };

/* Opaque type for the mix virtual machine */
typedef struct mix_vm_t mix_vm_t;

/* Create/destroy a mix vm */
extern mix_vm_t *
mix_vm_new(void);

extern void
mix_vm_delete(mix_vm_t * vm);

/* connect devices to a virtual machine */
extern mix_device_t * /* previously connected device */
mix_vm_connect_device (mix_vm_t *vm, mix_device_t *device);

/* get device */
extern mix_device_t *
mix_vm_get_device (const mix_vm_t *vm, mix_device_type_t dev);

/* install a device factory for automatic connection */
typedef mix_device_t * (* mix_device_factory_t) (mix_device_type_t device);
extern void
mix_vm_set_device_factory (mix_vm_t *vm, mix_device_factory_t factory);

/* Reset a vm (set state as of a newly created one) */
extern void
mix_vm_reset(mix_vm_t * vm);

/* Set start address for execution */
extern void
mix_vm_set_start_addr(mix_vm_t *vm, mix_address_t addr);

/* Access to the vm's registers */
extern mix_word_t
mix_vm_get_rA(const mix_vm_t *vm);

extern mix_word_t
mix_vm_get_rX(const mix_vm_t *vm);

extern mix_short_t
mix_vm_get_rJ(const mix_vm_t *vm);

extern mix_short_t
mix_vm_get_rI(const mix_vm_t *vm, guint idx);

extern void
mix_vm_set_rA(mix_vm_t *vm, mix_word_t value);

extern void
mix_vm_set_rX(mix_vm_t *vm, mix_word_t value);

extern void
mix_vm_set_rJ(mix_vm_t *vm, mix_short_t value);

extern void
mix_vm_set_rI(mix_vm_t *vm, guint idx, mix_short_t value);

/* Access to the comparison flag and overflow toggle */
extern mix_cmpflag_t
mix_vm_get_cmpflag(const mix_vm_t *vm);

extern void
mix_vm_set_cmpflag(mix_vm_t *vm, mix_cmpflag_t value);

extern gboolean
mix_vm_get_overflow(const mix_vm_t *vm);

extern void
mix_vm_set_overflow(mix_vm_t *vm, gboolean value);

extern void
mix_vm_toggle_overflow(mix_vm_t *vm);

extern gboolean
mix_vm_is_halted(const mix_vm_t *vm);

/* Access to memory cells */
extern mix_word_t
mix_vm_get_addr_contents(const mix_vm_t *vm, mix_address_t addr);

extern void
mix_vm_set_addr_contents(mix_vm_t *vm, mix_address_t addr, mix_word_t value);

/* Execution of instructions and programs */
extern gboolean /* TRUE if success */
mix_vm_exec_ins(mix_vm_t *vm, const mix_ins_t *ins);

/* Load a code file into memory (-name- does not need the default extension)
 * resetting the vm's state
 */
extern gboolean
mix_vm_load_file(mix_vm_t *vm, const gchar *name);

/* Get the source file object corresponding to the last loaded code file */
extern const mix_src_file_t *
mix_vm_get_src_file (const mix_vm_t *vm);

/* Get symbol table of loaded file */
extern const mix_symbol_table_t *
mix_vm_get_symbol_table (const mix_vm_t *vm);

/* Get current program counter */
extern mix_address_t
mix_vm_get_prog_count (const mix_vm_t *vm);

/* Get the source line number for a given address */
extern guint
mix_vm_get_address_lineno (const mix_vm_t *vm, mix_address_t addr);

/* Get the address for a given source line number */
extern mix_address_t
mix_vm_get_lineno_address (const mix_vm_t *vm, guint lineno);

/* continue execution of instructions in memory */
/* Possible outcomes */
typedef enum {
  MIX_VM_ERROR,			/* error executing instructions */
  MIX_VM_BREAK,			/* breakpoint found */
  MIX_VM_COND_BREAK,		/* conditional breakpoint found */
  MIX_VM_HALT,			/* end of execution */
  MIX_VM_RUNNING,		/* successful instruction execution */
  MIX_VM_LOADED,		/* program loaded */
  MIX_VM_EMPTY			/* no program loaded */
} mix_vm_status_t;

/* execution errors */
typedef enum {
  MIX_VM_ERROR_NONE,            /* no error */
  MIX_VM_ERROR_BAD_ACCESS,      /* bad memory address */
  MIX_VM_ERROR_BAD_DEVICE_NO,   /* bad device number */
  MIX_VM_ERROR_BAD_FSPEC,       /* invalid fspec */
  MIX_VM_ERROR_BAD_M,           /* invalid M-value */
  MIX_VM_ERROR_DEV_CTL,         /* error accessing device for ioctl */
  MIX_VM_ERROR_DEV_READ,        /* error accessing device for reading */
  MIX_VM_ERROR_DEV_WRITE,       /* error accessing device for writing */
  MIX_VM_ERROR_UNEXPECTED       /* unexpected error */
} mix_vm_error_t;

extern mix_vm_error_t
mix_vm_get_last_error (const mix_vm_t *vm);

extern const gchar *
mix_vm_get_last_error_string (const mix_vm_t *vm);

extern const gchar *
mix_vm_get_error_string (mix_vm_error_t code);

/* run until next breakpoint or end of execution */
extern mix_vm_status_t
mix_vm_run (mix_vm_t *vm);

/* execute next memory instruction */
extern mix_vm_status_t
mix_vm_exec_next (mix_vm_t *vm);

/* get the current execution status */
extern mix_vm_status_t
mix_vm_get_run_status (const mix_vm_t *vm);

/* get the line no. of the last break or 0 if not found */
extern gulong
mix_vm_get_break_lineno (const mix_vm_t *vm);


/* Breakpoints */
/* possible error outcomes */
enum {
  MIX_VM_BP_ERROR = -4,		/* internal error */
  MIX_VM_BP_NDEBUG = -3,	/* no debug info */
  MIX_VM_BP_INV_ADDRESS = -2,   /* address out of range */
  MIX_VM_BP_INV_LINE = -1,	/* invalid line no. */
  MIX_VM_BP_OK = 0		/* success */
};


extern gint /* if >0 the line no. of the break point */
mix_vm_set_breakpoint (mix_vm_t *vm, guint lineno);

extern gint /* one of MIX_VM_BP_ */
mix_vm_set_breakpoint_address (mix_vm_t *vm, guint address);

extern gint /* one of MIX_VM_BP_ */
mix_vm_clear_breakpoint (mix_vm_t *vm, guint lineno);

extern gint /* one of MIX_VM_BP_ */
mix_vm_clear_breakpoint_address (mix_vm_t *vm, guint address) ;

extern gboolean
mix_vm_has_breakpoint_at_address (const mix_vm_t *vm, guint address);

extern void
mix_vm_clear_all_breakpoints (mix_vm_t *vm);

#include "mix_predicate.h"
extern gboolean
mix_vm_set_conditional_breakpoint (mix_vm_t *vm, mix_predicate_t *pred);

extern gboolean
mix_vm_clear_conditional_breakpoint (mix_vm_t *vm, mix_predicate_t *pred);

extern const gchar *
mix_vm_get_last_breakpoint_message (const mix_vm_t *vm);

extern mix_predicate_type_t
mix_vm_get_last_conditional_breakpoint_type (const mix_vm_t *vm);

/* Get the vm uptime, defined as the time spent executing instructions */
extern mix_time_t
mix_vm_get_uptime (const mix_vm_t *vm);

/* Get the list of addresses for executed instructions */
extern const GSList *
mix_vm_get_backtrace (const mix_vm_t *vm);


#endif /* MIX_VM_H */

