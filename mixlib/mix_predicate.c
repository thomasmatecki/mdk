/* -*-c-*- -------------- mix_predicate.c :
 * Implementation of the functions declared in mix_predicate.h
 * ------------------------------------------------------------------
 * Copyright (C) 2001, 2002, 2006, 2007 Free Software Foundation, Inc.
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

#include "mix_vm.h"
#include "mix_predicate.h"

/* predicate data */
typedef union pred_data_t
{
  mix_word_t regA;
  mix_word_t regX;
  mix_short_t regI;
  mix_cmpflag_t cmp;
  gboolean over;
  mix_word_t mem;
} pred_data_t;

/* the predicate function type */
typedef gboolean (*mix_predicate_fun_t) (mix_predicate_t *pred,
					 const mix_vm_t *vm);

/* the predicate type */
struct mix_predicate_t
{
  mix_predicate_type_t type;
  pred_data_t data;
  guint control;
};

/* predicate funcs */
static gboolean
pred_func_rA (mix_predicate_t *pred, const mix_vm_t *vm)
{
  mix_word_t val = mix_vm_get_rA (vm);
  if (pred->data.regA == val) return FALSE;
  pred->data.regA = val;
  return TRUE;
}

static gboolean
pred_func_rX (mix_predicate_t *pred, const mix_vm_t *vm)
{
  mix_word_t val = mix_vm_get_rX (vm);
  if (pred->data.regX == val) return FALSE;
  pred->data.regX = val;
  return TRUE;
}

static gboolean
pred_func_rI (mix_predicate_t *pred, const mix_vm_t *vm)
{
  mix_short_t val = (pred->control == 0) ? mix_vm_get_rJ (vm)
    : mix_vm_get_rI (vm, pred->control);
  if (pred->data.regI == val) return FALSE;
  pred->data.regI = val;
  return TRUE;
}

static gboolean
pred_func_mem (mix_predicate_t *pred, const mix_vm_t *vm)
{
  mix_word_t val =
    mix_vm_get_addr_contents (vm, (mix_address_t)pred->control);
  if (pred->data.mem == val) return FALSE;
  pred->data.mem = val;
  return TRUE;
}

static gboolean
pred_func_cmp (mix_predicate_t *pred, const mix_vm_t *vm)
{
  mix_cmpflag_t val = mix_vm_get_cmpflag (vm);
  if (pred->data.cmp == val) return FALSE;
  pred->data.cmp = val;
  return TRUE;
}

static gboolean
pred_func_over (mix_predicate_t *pred, const mix_vm_t *vm)
{
  gboolean val = mix_vm_get_overflow (vm);
  if (pred->data.over == val) return FALSE;
  pred->data.over = val;
  return TRUE;
}

static mix_predicate_fun_t PRED_FUNCS_[] = {
  pred_func_rA, pred_func_rX, pred_func_rI, pred_func_rI, pred_func_rI,
  pred_func_rI, pred_func_rI, pred_func_rI, pred_func_rI,
  pred_func_over, pred_func_cmp, pred_func_mem
};

/* create predicates based on vm status */
mix_predicate_t *
mix_predicate_new (mix_predicate_type_t type)
{
  mix_predicate_t *result;
  g_return_val_if_fail (type <= MIX_PRED_MEM, NULL);
  result = g_new (mix_predicate_t, 1);
  result->type = type;
  result->data.regA = MIX_WORD_ZERO;
  if (type >= MIX_PRED_REG_I1 && type <= MIX_PRED_REG_I6)
    result->control = 1 + type - MIX_PRED_REG_I1;
  else
    result->control = 0;
  return result;
}

/* delete a predicate */
void
mix_predicate_delete (mix_predicate_t *predicate)
{
  g_return_if_fail (predicate != NULL);
  g_free (predicate);
}

/* return the predicate's type */
mix_predicate_type_t
mix_predicate_get_type (const mix_predicate_t *pred)
{
  g_return_val_if_fail (pred != NULL, MIX_PRED_INVALID);
  return pred->type;
}

/* test a predicate */
gboolean
mix_predicate_eval(mix_predicate_t *pred, const mix_vm_t *vm)
{
  g_return_val_if_fail (pred != NULL, FALSE);
  g_return_val_if_fail (vm != NULL, FALSE);
  return PRED_FUNCS_[pred->type] (pred, vm);
}

/* change mem address of a MIX_PRED_MEM predicate */
void
mix_predicate_set_mem_address (mix_predicate_t *predicate,
			       mix_address_t address)
{
  g_return_if_fail (predicate != NULL);
  predicate->control = address;
}

/* get message about predicate evaluation */
const gchar *
mix_predicate_get_message (const mix_predicate_t *predicate)
{
  enum {SIZE = 256};
  static gchar BUFFER[SIZE];
  static const gchar *CMP_STRINGS[] = { "L", "E", "G"};

  g_return_val_if_fail (predicate != NULL, NULL);

  switch (predicate->type)
    {
    case MIX_PRED_REG_A:
      g_snprintf (BUFFER, SIZE, _("Register A changed to %s%ld"),
		mix_word_is_negative (predicate->data.regA)? "-" : "+",
		mix_word_magnitude (predicate->data.regA));
      break;
    case MIX_PRED_REG_X:
      g_snprintf (BUFFER, SIZE, _("Register X changed to %s%ld"),
		mix_word_is_negative (predicate->data.regX)? "-" : "+",
		mix_word_magnitude (predicate->data.regX));
      break;
    case MIX_PRED_REG_J:
      g_snprintf (BUFFER, SIZE, _("Register J changed to %d"),
		mix_short_magnitude (predicate->data.regI));
      break;
    case MIX_PRED_REG_I1: case MIX_PRED_REG_I2: case MIX_PRED_REG_I3:
    case MIX_PRED_REG_I4: case MIX_PRED_REG_I5: case MIX_PRED_REG_I6:
      g_snprintf (BUFFER, SIZE, _("Register I%d changed to %s%d"),
		predicate->control,
		mix_short_is_negative (predicate->data.regI)? "-" : "+",
		mix_short_magnitude (predicate->data.regI));
      break;
    case MIX_PRED_CMP:
      g_snprintf (BUFFER, SIZE, _("Comparison flag changed to %s"),
		CMP_STRINGS[predicate->data.cmp]);
      break;
    case MIX_PRED_OVER:
      g_snprintf (BUFFER, SIZE, _("Overflow toggled %s"),
		predicate->data.over ? "ON" : "OFF");
      break;
    case MIX_PRED_MEM:
      g_snprintf (BUFFER, SIZE, _("Memory address %d changed to %s%ld"),
		predicate->control,
		mix_word_is_negative (predicate->data.mem)? "-" : "+",
		mix_word_magnitude (predicate->data.mem));
      break;
    default:
      g_assert_not_reached ();
    }
  return BUFFER;
}

