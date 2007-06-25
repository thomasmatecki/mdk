/* -*-c-*- ---------------- xmix_eval.h :
 * Definition of opaque types in mix_eval.h
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


#ifndef XMIX_EVAL_H
#define XMIX_EVAL_H

#include "mix_eval.h"

/* the evaluator type */
struct mix_eval_t
{
  mix_symbol_table_t *table;	/* symbol table */
  gboolean towner;		/* true if owns the table */
  mix_eval_result_t result;	/* last evaluation result */
  gint errpos;			/* location of last error */
  mix_word_t value;		/* last computed value */
};

/* flex scanner data/result struct */
typedef struct mix_eval_data_
{
  gchar *expr;
  const mix_symbol_table_t *table;
  mix_word_t value;
  mix_short_t loc;
  gint errpos;
} mix_eval_data_;

/* flex scanner prototype */
extern
mix_eval_result_t mix_eval_expr (mix_eval_data_ *data);

#endif /* XMIX_EVAL_H */

