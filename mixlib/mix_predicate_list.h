/* -*-c-*- ---------------- mix_predicate_list.h :
 * A list of predicates.
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


#ifndef MIX_PREDICATE_LIST_H
#define MIX_PREDICATE_LIST_H

#include "mix.h"
#include "mix_vm.h"
#include "mix_predicate.h"

/* the predicate list type */
typedef struct mix_predicate_list_t mix_predicate_list_t;

/* create/destroy a list of predicates */
extern mix_predicate_list_t *
mix_predicate_list_new (const mix_vm_t *vm);

extern void
mix_predicate_list_delete (mix_predicate_list_t *list);

/* evaluate the predicate list */
extern gboolean
mix_predicate_list_eval (mix_predicate_list_t *list);

extern const mix_predicate_t *
mix_predicate_list_last_true_eval (const mix_predicate_list_t *list);

#define mix_predicate_list_last_true_eval_type(list) \
  mix_predicate_get_type (mix_predicate_list_last_true_eval (list))

/* add/remove predicates to the list */
extern void
mix_predicate_list_add (mix_predicate_list_t *list, mix_predicate_t *predicate);

extern gboolean
mix_predicate_list_remove (mix_predicate_list_t *list,
			   mix_predicate_t *predicate);

extern void
mix_predicate_list_clear (mix_predicate_list_t *list);

#endif /* MIX_PREDICATE_LIST_H */

