/* -*-c-*- ---------------- mix_predicate.h :
 * Predicates and lists of predicates testing vm status.
 * ------------------------------------------------------------------
 * Copyright (C) 2001, 2007 Free Software Foundation, Inc.
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


#ifndef MIX_PREDICATE_H
#define MIX_PREDICATE_H

#include "mix.h"
#include "mix_types.h"
#include "mix_ins.h"

/* the predicate type */
typedef struct mix_predicate_t mix_predicate_t;

/* predicate types */
typedef enum {
  MIX_PRED_REG_A,
  MIX_PRED_REG_X,
  MIX_PRED_REG_J,
  MIX_PRED_REG_I1,
  MIX_PRED_REG_I2,
  MIX_PRED_REG_I3,
  MIX_PRED_REG_I4,
  MIX_PRED_REG_I5,
  MIX_PRED_REG_I6,
  MIX_PRED_OVER,
  MIX_PRED_CMP,
  MIX_PRED_MEM,
  MIX_PRED_INVALID
} mix_predicate_type_t;

/* create predicates based on vm status */
extern mix_predicate_t *
mix_predicate_new (mix_predicate_type_t type);

/* delete a predicate */
extern void
mix_predicate_delete (mix_predicate_t *predicate);

/* return the predicate's type */
extern mix_predicate_type_t
mix_predicate_get_type (const mix_predicate_t *pred);

/* change mem address of a MIX_PRED_MEM predicate */
extern void
mix_predicate_set_mem_address (mix_predicate_t *predicate,
			       mix_address_t address);

/* get message about predicate evaluation */
extern const gchar *
mix_predicate_get_message (const mix_predicate_t *predicate);

/* test a predicate */
#include "mix_vm.h"

extern gboolean
mix_predicate_eval(mix_predicate_t *pred, const mix_vm_t *vm);

#endif /* MIX_PREDICATE_H */

