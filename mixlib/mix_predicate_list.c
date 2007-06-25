/* -*-c-*- -------------- mix_predicate_list.c :
 * Implementation of the functions declared in mix_predicate_list.h
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


#include "mix_predicate_list.h"

/* the predicate list type */
struct mix_predicate_list_t
{
  GSList *predicates;
  const mix_vm_t *vm;
  const mix_predicate_t *last;
};

/* a list of predicates */
mix_predicate_list_t *
mix_predicate_list_new (const mix_vm_t *vm)
{
  mix_predicate_list_t *result;
  g_return_val_if_fail (vm != NULL, NULL);
  result = g_new (mix_predicate_list_t, 1);
  result->predicates = NULL;
  result->vm = vm;
  result->last = NULL;
  return result;
}

void
mix_predicate_list_delete (mix_predicate_list_t *list)
{
  g_return_if_fail (list != NULL);
  g_slist_free (list->predicates);
  g_free (list);
}

/* evaluate the predicate list */
gboolean
mix_predicate_list_eval (mix_predicate_list_t *list)
{
  GSList *node;

  g_return_val_if_fail (list != NULL, FALSE);
  node = list->predicates;
  while (node) {
    mix_predicate_t *pred = (mix_predicate_t *)(node->data);
    if (mix_predicate_eval (pred, list->vm))
      {
	list->last = pred;
	return TRUE;
      }
    node = node->next;
  }
  list->last = NULL;
  return FALSE;
}

/* add/remove predicates to the list */
void
mix_predicate_list_add (mix_predicate_list_t *list, mix_predicate_t *predicate)
{
  g_return_if_fail (list != NULL);
  g_return_if_fail (predicate != NULL);
  if (!g_slist_find (list->predicates, predicate))
    list->predicates = g_slist_append (list->predicates, (gpointer)predicate);
  (void)mix_predicate_eval (predicate, list->vm);
}

gboolean
mix_predicate_list_remove (mix_predicate_list_t *list,
			   mix_predicate_t *predicate)
{
  g_return_val_if_fail (list != NULL, FALSE);
  g_return_val_if_fail (predicate != NULL, FALSE);
  if (g_slist_find (list->predicates, predicate))
    {
      list->predicates = g_slist_remove (list->predicates, predicate);
      return TRUE;
    }
  else
    return FALSE;
}

void
mix_predicate_list_clear (mix_predicate_list_t *list)
{
  g_return_if_fail (list != NULL);
  g_slist_free (list->predicates);
  list->predicates = NULL;
  list->last = NULL;
}

const mix_predicate_t *
mix_predicate_list_last_true_eval (const mix_predicate_list_t *list)
{
  g_return_val_if_fail (list != NULL, NULL);
  return list->last;
}
