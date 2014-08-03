/* -*-c-*- ------------------ mix_vm.c :
 * Implementation of the functions declared in mix_vm.h
 * ------------------------------------------------------------------
 * Copyright (C) 2000, 2001, 2004, 2007, 2014 Free Software Foundation, Inc.
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

#include "mix.h"
#include "xmix_vm.h"

#define vm_reset_(vm) vm_reset_reload_ (vm, FALSE);

static void
vm_reset_reload_ (mix_vm_t *vm, gboolean is_reload)
{
  guint k;
  set_status_ (vm, MIX_VM_EMPTY);
  set_rA_ (vm, MIX_WORD_ZERO);
  set_rX_ (vm, MIX_WORD_ZERO);
  set_rJ_ (vm, MIX_WORD_ZERO);
  for ( k = 1; k < IREG_NO_+1; ++k ) set_rI_ (vm, k, MIX_WORD_ZERO);
  set_over_ (vm, FALSE);
  set_cmp_ (vm, mix_EQ);
  set_loc_ (vm, MIX_WORD_ZERO);
  set_last_error_ (vm, MIX_VM_ERROR_NONE);
  for ( k = 0; k < MEM_CELLS_NO_; ++k) set_cell_ (vm, k, MIX_WORD_ZERO);

  if (vm->symbol_table != NULL )
    {
      mix_symbol_table_delete (vm->symbol_table);
      vm->symbol_table = NULL;
    }

  if (vm->line_table != NULL)
    {
      g_tree_destroy (vm->line_table);
      vm->line_table = NULL;
    }

  if (vm->address_table != NULL)
    {
      g_tree_destroy (vm->address_table);
      vm->address_table = NULL;
    }

  if (vm->src_file != NULL)
    {
      mix_src_file_delete (vm->src_file);
      vm->src_file = NULL;
    }

  for (k = 0; k < BD_NO_; ++k)
    if (vm->devices[k] != NULL)
      {
        mix_device_type_t type = mix_device_type (vm->devices[k]);
        if (!is_reload || (type != mix_dev_CONSOLE
                           && type != mix_dev_PRINTER
                           && type != mix_dev_CARD_WR))
          {
            mix_device_delete (vm->devices[k]);
            vm->devices[k] = NULL;
          }
      }

  if (!is_reload)
    mix_vm_clear_all_breakpoints (vm);

  if (vm->address_list)
    {
      g_slist_free (vm->address_list);
      vm->address_list = NULL;
    }
}


/* Create/destroy a mix vm */
mix_vm_t *
mix_vm_new (void)
{
  int i;

  mix_vm_t *vm = g_new (struct mix_vm_t,1);
  vm->line_table = NULL;
  vm->address_table = NULL;
  vm->symbol_table = NULL;
  vm->src_file = NULL;
  vm->pred_list = mix_predicate_list_new (vm);
  vm->address_list = NULL;
  vm->last_error = MIX_VM_ERROR_NONE;

  for (i = 0; i < BD_NO_; ++i)
    vm->devices[i] = NULL;

  vm->clock = mix_vm_clock_new ();
  vm->factory = mix_device_new;

  vm_reset_ (vm);

  return vm;
}


void
mix_vm_delete (mix_vm_t * vm)
{
  int i;

  g_return_if_fail (vm != NULL);

  if (vm->line_table != NULL) g_tree_destroy (vm->line_table);
  if (vm->address_table != NULL) g_tree_destroy (vm->address_table);
  if (vm->symbol_table != NULL) mix_symbol_table_delete (vm->symbol_table);
  if (vm->src_file != NULL) mix_src_file_delete (vm->src_file);
  if (vm->pred_list != NULL) mix_predicate_list_delete (vm->pred_list);
  if (vm->address_list != NULL) g_slist_free (vm->address_list);
  for (i = 0; i < BD_NO_; ++i)
    mix_device_delete (vm->devices[i]);
  mix_vm_clock_delete (vm->clock);
  g_free (vm);
}

/* connect devices to a virtual machine */
mix_device_t *
mix_vm_connect_device (mix_vm_t *vm, mix_device_t *device)
{
  mix_device_t *old;
  mix_device_type_t type;

  g_return_val_if_fail (vm != NULL, NULL);
  g_return_val_if_fail (device != NULL, NULL);

  type = mix_device_type (device);
  old = vm->devices[type];
  vm->devices[type] = device;

  return old;
}

/* get device */
mix_device_t *
mix_vm_get_device (const mix_vm_t *vm, mix_device_type_t dev)
{
  g_return_val_if_fail (vm != NULL, NULL);
  g_return_val_if_fail (dev < mix_dev_INVALID, NULL);
  return vm->devices[dev];
}

/* install a device factory for automatic connection */
void
mix_vm_set_device_factory (mix_vm_t *vm, mix_device_factory_t factory)
{
  g_return_if_fail (vm != NULL);
  g_return_if_fail (factory != NULL);
  vm->factory = factory;
}

/* Reset a vm  (set state as of a newly created one) */
void
mix_vm_reset (mix_vm_t * vm)
{
  g_return_if_fail (vm != NULL);
  vm_reset_ (vm);
}

/* Set start address for execution */
void
mix_vm_set_start_addr (mix_vm_t *vm, mix_address_t addr)
{
  g_return_if_fail (vm != NULL);
  set_loc_ (vm, addr);
}

/* Access to the vm's registers */
mix_word_t
mix_vm_get_rA (const mix_vm_t *vm)
{
  g_return_val_if_fail (vm != NULL, MIX_WORD_ZERO);
  return get_rA_ (vm);
}

mix_word_t
mix_vm_get_rX (const mix_vm_t *vm)
{
  g_return_val_if_fail (vm != NULL, MIX_WORD_ZERO);
  return get_rX_ (vm);
}

mix_short_t
mix_vm_get_rJ (const mix_vm_t *vm)
{
  g_return_val_if_fail (vm != NULL, MIX_SHORT_ZERO);
  return mix_word_to_short_fast (get_rJ_ (vm));
}

mix_short_t
mix_vm_get_rI (const mix_vm_t *vm, guint idx)
{
  g_return_val_if_fail (vm != NULL, MIX_SHORT_ZERO);
  g_return_val_if_fail (IOK_ (idx), MIX_SHORT_ZERO);
  return mix_word_to_short_fast (get_rI_ (vm, idx));
}

void
mix_vm_set_rA (mix_vm_t *vm, mix_word_t value)
{
  g_return_if_fail (vm != NULL);
  set_rA_ (vm, value);
}

void
mix_vm_set_rX (mix_vm_t *vm, mix_word_t value)
{
  g_return_if_fail (vm != NULL);
  set_rX_ (vm, value);
}

void
mix_vm_set_rJ (mix_vm_t *vm, mix_short_t value)
{
  g_return_if_fail (vm != NULL);
  g_return_if_fail (mix_short_is_positive (value));
  set_rJ_ (vm, mix_short_to_word_fast (value));
}

void
mix_vm_set_rI (mix_vm_t *vm, guint idx, mix_short_t value)
{
  g_return_if_fail (vm != NULL);
  g_return_if_fail (IOK_ (idx));
  set_rI_ (vm, idx, mix_short_to_word_fast (value));
}

/* Access to the comparison flag and overflow toggle */
mix_cmpflag_t
mix_vm_get_cmpflag (const mix_vm_t *vm)
{
  g_return_val_if_fail (vm != NULL, mix_EQ);
  return get_cmp_ (vm);
}

void
mix_vm_set_cmpflag (mix_vm_t *vm, mix_cmpflag_t value)
{
  g_return_if_fail (vm != NULL);
  set_cmp_ (vm, value);
}

gboolean
mix_vm_get_overflow (const mix_vm_t *vm)
{
  g_return_val_if_fail (vm != NULL, TRUE);
  return get_over_ (vm);
}

void
mix_vm_set_overflow (mix_vm_t *vm, gboolean value)
{
  g_return_if_fail (vm != NULL);
  set_over_ (vm, value);
}

void
mix_vm_toggle_overflow (mix_vm_t *vm)
{
  g_return_if_fail (vm != NULL);
  set_over_ (vm, !get_over_ (vm));
}

/* Access to memory cells */
mix_word_t
mix_vm_get_addr_contents (const mix_vm_t *vm, mix_address_t addr)
{
  g_return_val_if_fail (vm != NULL, MIX_WORD_ZERO);
  return (MEMOK_ (addr))? get_cell_ (vm, addr) : MIX_WORD_ZERO;
}

void
mix_vm_set_addr_contents (mix_vm_t *vm, mix_address_t addr, mix_word_t value)
{
  g_return_if_fail (vm != NULL);
  if (MEMOK_ (addr)) set_cell_ (vm, addr, value);
}

gboolean
mix_vm_is_halted (const mix_vm_t *vm)
{
  return is_halted_ (vm);
}

/* Execution of instructions */
gboolean /* TRUE if success */
mix_vm_exec_ins (mix_vm_t *vm, const mix_ins_t *ins)
{
  g_return_val_if_fail (vm != NULL, FALSE);
  g_return_val_if_fail (ins != NULL, FALSE);
  return (*ins_handlers_[ins->opcode]) (vm,ins);
}

/* comparison function for the line and address tables tree */
static gint
cmp_uint_ (gconstpointer a, gconstpointer b)
{
  return GPOINTER_TO_UINT (a) - GPOINTER_TO_UINT (b);
}

gboolean
mix_vm_load_file (mix_vm_t *vm, const gchar *name)
{
  mix_code_file_t *file;
  mix_src_file_t *sfile = NULL;
  mix_ins_desc_t ins;
  const gchar *sp;
  gboolean reload = FALSE;

  g_return_val_if_fail (vm != NULL, FALSE);
  file = mix_code_file_new_read (name);
  if (file == NULL)
    {
      set_status_ (vm, MIX_VM_ERROR);
      return FALSE;
    }
  sp = mix_code_file_get_source_path (file);

  if (sp != NULL)
    {
      sfile = mix_src_file_new_for_read (sp);
      reload = (vm->src_file
		&& !strcmp (mix_src_file_get_path (vm->src_file),
			    mix_src_file_get_path (sfile)));
    }

  vm_reset_reload_ (vm, reload);

  if ( mix_code_file_is_debug (file) )
    {
      vm->symbol_table = mix_code_file_get_symbol_table (file);
      vm->line_table = g_tree_new (cmp_uint_);
      vm->address_table = g_tree_new (cmp_uint_);
    }

  vm->src_file = sfile;

  while ( mix_code_file_get_ins (file, &ins) )
    {
      set_cell_ (vm, ins.address, ins.ins);
      if ( vm->line_table != NULL )
	{
	  g_tree_insert (vm->line_table,
			 GUINT_TO_POINTER (ins.lineno),
			 GUINT_TO_POINTER ((guint)ins.address));
	  g_tree_insert (vm->address_table,
			 GUINT_TO_POINTER ((guint)ins.address),
			 GUINT_TO_POINTER (ins.lineno));
	}
    }
  set_loc_ (vm, mix_code_file_get_start_addr (file));
  set_start_ (vm, get_loc_ (vm));
  mix_code_file_delete (file);

  set_status_ (vm, MIX_VM_LOADED);
  return TRUE;
}

const mix_src_file_t *
mix_vm_get_src_file (const mix_vm_t *vm)
{
  g_return_val_if_fail (vm != NULL, NULL);
  return vm->src_file;
}

const mix_symbol_table_t *
mix_vm_get_symbol_table (const mix_vm_t *vm)
{
  g_return_val_if_fail (vm != NULL, NULL);
  return vm->symbol_table;
}

mix_address_t
mix_vm_get_prog_count (const mix_vm_t *vm)
{
  g_return_val_if_fail (vm != NULL, MIX_SHORT_ZERO);
  return get_loc_ (vm);
}

/* Get the source line number for a given address */
guint
mix_vm_get_address_lineno (const mix_vm_t *vm, mix_address_t addr)
{
  gpointer gp_addr = GUINT_TO_POINTER ((guint)addr);
  guint lineno;

  g_return_val_if_fail (vm != NULL, 0);
  if (!(MEMOK_ (addr))) return 0;
  lineno = GPOINTER_TO_UINT (g_tree_lookup (vm->address_table, gp_addr));
  return lineno;
}

/* Get the address for a given source line number */
typedef struct
{
  guint lineno;
  mix_address_t result;
} addr_traverse_t;

static gint
get_address_ (gpointer key, gpointer value, gpointer data)
{
  addr_traverse_t *tr = (addr_traverse_t *)data;
  if (GPOINTER_TO_UINT (key) == tr->lineno)
    {
      tr->result = mix_short_new (GPOINTER_TO_UINT (value));
      return TRUE;
    }
  return (GPOINTER_TO_UINT (key) < tr->lineno)? FALSE:TRUE;
}

mix_address_t
mix_vm_get_lineno_address (const mix_vm_t *vm, guint lineno)
{
  addr_traverse_t tr;

  g_return_val_if_fail (vm != NULL, MIX_VM_CELL_NO);
  if (!vm->line_table) return MIX_VM_CELL_NO;
  tr.lineno = lineno;
  tr.result = MIX_VM_CELL_NO;
  g_tree_foreach (vm->line_table, get_address_, (gpointer)&tr);
  return tr.result;
}


/* continue execution of instructions in memory */
mix_vm_status_t
mix_vm_run (mix_vm_t *vm)
{
  mix_ins_t ins;
  g_return_val_if_fail (vm != NULL, MIX_VM_ERROR);

  while ( !is_halted_ (vm) )
    {
      mix_word_to_ins_uncheck (get_cell_ (vm, get_loc_ (vm)), ins);
      vm->address_list =
	g_slist_prepend (vm->address_list,
			 GINT_TO_POINTER ((gint)get_loc_ (vm)));
      if ( !(*ins_handlers_[ins.opcode]) (vm,&ins) )
	return set_status_ (vm, MIX_VM_ERROR);
      else
	update_time_ (vm, &ins);
      if (bp_is_set_ (vm, get_loc_ (vm)))
	return set_status_ (vm, MIX_VM_BREAK);
      if (mix_predicate_list_eval (get_pred_list_ (vm)))
	return set_status_ (vm, MIX_VM_COND_BREAK);
      if (get_loc_ (vm) >= MIX_VM_CELL_NO) halt_ (vm, TRUE);
    }
  return set_status_ (vm, MIX_VM_HALT);
}

/* execute next memory instruction */
mix_vm_status_t
mix_vm_exec_next (mix_vm_t *vm)
{
  mix_ins_t ins;
  g_return_val_if_fail (vm != NULL, MIX_VM_ERROR);
  if (get_loc_ (vm) >= MIX_VM_CELL_NO) halt_ (vm, TRUE);
  if (is_halted_ (vm)) return set_status_ (vm, MIX_VM_HALT);
  vm->address_list =
    g_slist_prepend (vm->address_list,
		     GINT_TO_POINTER ((gint)get_loc_ (vm)));
  mix_word_to_ins_uncheck (get_cell_ (vm, get_loc_ (vm)), ins);
  if (!(*ins_handlers_[ins.opcode]) (vm, &ins))
    return set_status_ (vm, MIX_VM_ERROR);
  else
    update_time_ (vm, &ins);
  if (is_halted_ (vm)) return set_status_ (vm, MIX_VM_HALT);
  if (bp_is_set_ (vm, get_loc_ (vm))) return set_status_ (vm, MIX_VM_BREAK);
  if (mix_predicate_list_eval (get_pred_list_ (vm)))
    return set_status_ (vm, MIX_VM_COND_BREAK);
  return set_status_ (vm, MIX_VM_RUNNING);
}

/* get the current execution status */
mix_vm_status_t
mix_vm_get_run_status (const mix_vm_t *vm)
{
  g_return_val_if_fail (vm != NULL, MIX_VM_ERROR);
  return get_status_ (vm);
}

mix_vm_error_t
mix_vm_get_last_error (const mix_vm_t *vm)
{
  g_return_val_if_fail (vm != NULL, MIX_VM_ERROR_UNEXPECTED);
  return get_last_error_ (vm);
}

const gchar *
mix_vm_get_last_error_string (const mix_vm_t *vm)
{
  return mix_vm_get_error_string (mix_vm_get_last_error (vm));
}

const gchar *
mix_vm_get_error_string (mix_vm_error_t code)
{
  static const gchar *errors[] = {
    N_("No error"),
    N_("Invalid memory address"),
    N_("Invalid device number"),
    N_("Invalid fspec"),
    N_("Invalid M-value"),
    N_("Cannot access device"),
    N_("Cannot access device for reading"),
    N_("Cannot access device for writing"),
    N_("Unexpected error"),
    N_("Unknow error code")
  };

  return errors[code > MIX_VM_ERROR_UNEXPECTED ?
                MIX_VM_ERROR_UNEXPECTED + 1 : code];
}

/* Breakpoints */
gulong
mix_vm_get_break_lineno (const mix_vm_t *vm)
{
  g_return_val_if_fail (vm != NULL, 0);
  if (vm->address_table == NULL)
    return 0;
  else
    {
      gpointer loc = GUINT_TO_POINTER ((guint)get_loc_ (vm));
      return GPOINTER_TO_UINT (g_tree_lookup (vm->address_table,loc));
    }
}

typedef struct
{
  mix_vm_t *vm;
  guint lineno;
  gint result;
} bp_traverse_t;

static gint
set_break_ (gpointer key, gpointer value, gpointer data)
{
  bp_traverse_t *tr = (bp_traverse_t *)data;
  if (GPOINTER_TO_UINT (key) >= tr->lineno)
    {
      bp_set_ (tr->vm, mix_short_new (GPOINTER_TO_UINT (value)));
      tr->lineno = GPOINTER_TO_UINT (key);
      tr->result = MIX_VM_BP_OK;
      return TRUE;
    }
  return FALSE;
}

gint /* if >0 the line no. of the break point */
mix_vm_set_breakpoint (mix_vm_t *vm, guint lineno)
{
  bp_traverse_t tr;

  g_return_val_if_fail (vm != NULL, MIX_VM_BP_ERROR);
  if (!vm->line_table) return MIX_VM_BP_NDEBUG;
  tr.lineno = lineno;
  tr.vm = vm;
  tr.result = MIX_VM_BP_INV_LINE;
  g_tree_foreach (vm->line_table, set_break_, (gpointer)&tr);
  if (tr.result == MIX_VM_BP_OK)
    return tr.lineno;
  else
    return tr.result;
}

gint
mix_vm_set_breakpoint_address (mix_vm_t *vm, guint address)
{
  g_return_val_if_fail (vm != NULL, MIX_VM_BP_ERROR);
  if (address >= MIX_VM_CELL_NO)
    return MIX_VM_BP_INV_ADDRESS;
  else
    bp_set_ (vm, mix_short_new (address));
  return MIX_VM_BP_OK;
}

gboolean
mix_vm_has_breakpoint_at_address (const mix_vm_t *vm, guint address)
{
  g_return_val_if_fail (vm != NULL, FALSE);
  if (address >= MIX_VM_CELL_NO) return FALSE;
  return  (bp_is_set_ (vm, address));
}

static gint
clear_break_ (gpointer key, gpointer value, gpointer data)
{
  bp_traverse_t *tr = (bp_traverse_t *)data;
  if (GPOINTER_TO_UINT (key) == tr->lineno)
    {
      bp_clear_ (tr->vm, mix_short_new (GPOINTER_TO_UINT (value)));
      tr->result = MIX_VM_BP_OK;
      return TRUE;
    }
  else if (GPOINTER_TO_UINT (key) > tr->lineno)
    return TRUE;

  return FALSE;
}

gint /* one of MIX_VM_BP_ */
mix_vm_clear_breakpoint (mix_vm_t *vm, guint lineno)
{
  bp_traverse_t tr;

  g_return_val_if_fail (vm != NULL, MIX_VM_BP_ERROR);
  if (!vm->line_table) return MIX_VM_BP_NDEBUG;
  tr.lineno = lineno;
  tr.vm = vm;
  tr.result = MIX_VM_BP_INV_LINE;
  g_tree_foreach (vm->line_table, clear_break_, (gpointer)&tr);
  return tr.result;
}

gint
mix_vm_clear_breakpoint_address (mix_vm_t *vm, guint address)
{
  g_return_val_if_fail (vm != NULL, MIX_VM_BP_ERROR);
  if (address >= MIX_VM_CELL_NO)
    return MIX_VM_BP_INV_ADDRESS;
  else
    bp_clear_ (vm, mix_short_new (address));
  return MIX_VM_BP_OK;
}

void
mix_vm_clear_all_breakpoints (mix_vm_t *vm)
{
  g_return_if_fail (vm != NULL);
  bp_clear_all_ (vm);
  mix_predicate_list_clear (get_pred_list_ (vm));
}


gboolean
mix_vm_set_conditional_breakpoint (mix_vm_t *vm, mix_predicate_t *pred)
{
  g_return_val_if_fail (vm != NULL, FALSE);
  g_return_val_if_fail (pred != NULL, FALSE);
  mix_predicate_list_add (get_pred_list_ (vm), pred);
  return TRUE;
}

gboolean
mix_vm_clear_conditional_breakpoint (mix_vm_t *vm, mix_predicate_t *pred)
{
  g_return_val_if_fail (vm != NULL, FALSE);
  g_return_val_if_fail (pred != NULL, FALSE);
  return mix_predicate_list_remove (get_pred_list_ (vm), pred);
}

const gchar *
mix_vm_get_last_breakpoint_message (const mix_vm_t *vm)
{
  const mix_predicate_t *last = NULL;
  g_return_val_if_fail (vm != NULL, NULL);
  if ((last = mix_predicate_list_last_true_eval (get_pred_list_ (vm))) != NULL)
    return mix_predicate_get_message (last);
  return NULL;
}

mix_predicate_type_t
mix_vm_get_last_conditional_breakpoint_type (const mix_vm_t *vm)
{
  const mix_predicate_t *last = NULL;
  g_return_val_if_fail (vm != NULL, MIX_PRED_INVALID);
  last = mix_predicate_list_last_true_eval (get_pred_list_ (vm));
  return last? mix_predicate_get_type (last) : MIX_PRED_INVALID;
}

/* Get the vm uptime, defined as the time spent executing instructions */
mix_time_t
mix_vm_get_uptime (const mix_vm_t *vm)
{
  g_return_val_if_fail (vm != NULL, 0);
  return mix_vm_clock_get_time (get_clock_ (vm));
}

/* Get the list of addresses for executed instructions */
const GSList *
mix_vm_get_backtrace (const mix_vm_t *vm)
{
  g_return_val_if_fail (vm != NULL, NULL);
  return get_address_list_ (vm);
}
