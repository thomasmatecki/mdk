/* -*-c-*- -------------- mix_eval.c :
 * Implementation of the functions declared in mix_eval.h
 * ------------------------------------------------------------------
 * Copyright (C) 2000, 2007 Free Software Foundation, Inc.
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


#include "xmix_eval.h"


static const gchar *errors_[] = {
  N_("Successful evaluation"),
  N_("Syntax error in expression"),
  N_("Out of range F-specification"),
  N_("Mismatched parenthesis"),
  N_("Undefined symbol"),
  N_("Internal error")
};


/* create a new evaluator */
mix_eval_t *
mix_eval_new (void)
{
  mix_eval_t *result = g_new (mix_eval_t, 1);
  result->table = mix_symbol_table_new ();
  if (result->table == NULL) {
    g_free (result);
    return NULL;
  }
  result->towner = TRUE;
  result->value = MIX_WORD_ZERO;
  result->errpos = 0;
  return result;
}

/* create a new evaluator with an external symbol table */
mix_eval_t *
mix_eval_new_with_table (mix_symbol_table_t *table)
{
  mix_eval_t *result = g_new (mix_eval_t, 1);
  result->table = table;
  result->towner = FALSE;
  result->value = MIX_WORD_ZERO;
  result->errpos = 0;
  return result;
}

/* delete */
void
mix_eval_delete (mix_eval_t *eval)
{
  g_return_if_fail (eval);
  if (eval->table && eval->towner) {
    mix_symbol_table_delete (eval->table);
  }
  g_free (eval);
}

/* eval an expression */
mix_eval_result_t
mix_eval_expression_with_loc (mix_eval_t *eval, const gchar *expr,
			      mix_short_t loc)
{
  mix_eval_data_ data;

  if (expr == NULL || eval == NULL)
    return MIX_EVAL_INTERN;
  data.expr = g_strdup_printf ("%s\n", expr);
  data.table = eval->table;
  data.errpos = eval->errpos;
  data.value = eval->value;
  data.loc = loc;
  eval->result = mix_eval_expr (&data);
  if (eval->result == MIX_EVAL_OK) {
    eval->value = data.value;
    eval->errpos = -1;
  } else {
    eval->errpos = data.errpos;
  }
  g_free (data.expr);

  return eval->result;
}

/* get the result of the last evaluation */
mix_word_t
mix_eval_value (const mix_eval_t *eval)
{
  g_return_val_if_fail (eval != NULL, MIX_WORD_ZERO);
  return eval->value;
}

/* get the last eval result code */
mix_eval_result_t
mix_eval_last_error (const mix_eval_t *eval)
{
  g_return_val_if_fail (eval != NULL, MIX_EVAL_INTERN);
  return eval->result;
}

/* get the last error string */
const gchar*
mix_eval_last_error_string (const mix_eval_t *eval)
{
  g_return_val_if_fail (eval != NULL, errors_[MIX_EVAL_INTERN]);
  return errors_[eval->result];
}

/* get the position of last error */
guint
mix_eval_last_error_pos (const mix_eval_t *eval)
{
  g_return_val_if_fail (eval != NULL, 0);
  return eval->errpos;
}

/* add, or redefine, a symbol. see mix_symbol_table.h for
   possible outcomes. */
gint
mix_eval_set_symbol (mix_eval_t *eval, const gchar *symbol,
		     mix_word_t value)
{
  g_return_val_if_fail (eval != NULL && eval->table != NULL,
			MIX_SYM_FAIL);
  return mix_symbol_table_insert (eval->table, symbol, value);
}

void
mix_eval_remove_symbol (mix_eval_t *eval, const gchar *symbol)
{
  g_return_if_fail (eval != NULL && eval->table != NULL);
  mix_symbol_table_remove (eval->table, symbol);
}

void
mix_eval_use_symbol_table (mix_eval_t *eval,
			   mix_symbol_table_t *table)
{
  g_return_if_fail (eval != NULL);
  if (eval->table != NULL && eval->towner)
    mix_symbol_table_delete (eval->table);
  eval->table = table;
  eval->towner = FALSE;
}

const mix_symbol_table_t *
mix_eval_symbol_table (const mix_eval_t *eval)
{
  g_return_val_if_fail (eval != NULL, NULL);
  return eval->table;
}

gboolean
mix_eval_set_symbols_from_table (mix_eval_t *eval,
				 const mix_symbol_table_t *table)
{
  g_return_val_if_fail (eval != NULL, FALSE);
  if (eval->table != NULL)
    return mix_symbol_table_merge_table (eval->table, table);
  else
    return FALSE;
}

gboolean
mix_eval_remove_symbols_from_table (mix_eval_t *eval,
				    const mix_symbol_table_t *table)
{
  g_return_val_if_fail (eval != NULL, FALSE);
  if (eval->table != NULL)
    return mix_symbol_table_substract_table (eval->table, table);
  else
    return FALSE;
}
