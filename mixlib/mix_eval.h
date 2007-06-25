/* -*-c-*- ---------------- mix_eval.h :
 * mix_eval_t is an evaluator of MIX W-expressions
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


#ifndef MIX_EVAL_H
#define MIX_EVAL_H

#include "mix.h"
#include "mix_types.h"
#include "mix_symbol_table.h"

/* the evaluator type */
typedef struct mix_eval_t mix_eval_t;

/* possible evaluation outcomes*/
typedef enum {
  MIX_EVAL_OK, /* sucess*/
  MIX_EVAL_SYNTAX, /* syntax error */
  MIX_EVAL_INV_FSPEC, /* out of range fspec */
  MIX_EVAL_MIS_PAREN, /* mismatched parenthesis */
  MIX_EVAL_UNDEF_SYM, /* undefined symbol */
  MIX_EVAL_INTERN /* internal error */
} mix_eval_result_t;

/* create a new evaluator */
extern mix_eval_t *
mix_eval_new (void);

/* create a new evaluator with an external symbol table */
extern mix_eval_t *
mix_eval_new_with_table (mix_symbol_table_t *table);

/* delete */
extern void
mix_eval_delete (mix_eval_t *eval);

/* provide a new symbol table to be used */
extern void
mix_eval_use_symbol_table (mix_eval_t *eval,
			   mix_symbol_table_t *table);

extern const mix_symbol_table_t *
mix_eval_symbol_table (const mix_eval_t *eval);

/* eval an expression providing a value for loc counter */
extern mix_eval_result_t
mix_eval_expression_with_loc (mix_eval_t *eval, const gchar *expr,
			      mix_short_t loc);
/* eval an expression with null loc*/
#define mix_eval_expression (eval,expr) \
  mix_eval_expression_with_loc (eval, expr, MIX_SHORT_ZERO)


/* get the result of the last evaluation */
extern mix_word_t
mix_eval_value (const mix_eval_t *eval);

/* get the last eval result code */
extern mix_eval_result_t
mix_eval_last_error (const mix_eval_t *eval);

/* get the last error string */
extern const gchar*
mix_eval_last_error_string (const mix_eval_t *eval);

/* get the position of last error */
extern guint
mix_eval_last_error_pos (const mix_eval_t *eval);

/* add, or redefine, a symbol. see mix_symbol_table.h for
   possible outcomes. */
extern gint
mix_eval_set_symbol (mix_eval_t *eval, const gchar *symbol,
		     mix_word_t value);

extern gboolean
mix_eval_set_symbols_from_table (mix_eval_t *eval,
				 const mix_symbol_table_t *table);

extern void
mix_eval_remove_symbol (mix_eval_t *eval, const gchar *symbol);

extern gboolean
mix_eval_remove_symbols_from_table (mix_eval_t *eval,
				    const mix_symbol_table_t *table);



#endif /* MIX_EVAL_H */

