/* -*-c-*- ---------------- xmix_parser.h :
 * Declarations for the implementation of mix_parser_t
 * ------------------------------------------------------------------
 * Copyright (C) 2000, 2004, 2007 Free Software Foundation, Inc.
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


#ifndef XMIX_PARSER_H
#define XMIX_PARSER_H

#include "mix_ins.h"
#include "mix_file.h"
#include "mix_symbol_table.h"
#include "mix_parser.h"

/* mix_parser_t definition */
struct mix_parser_t
{
  mix_file_t  *in_file;     /* the source file to be compiled */
  mix_symbol_table_t *symbol_table;
  mix_symbol_table_t *ls_table;  /* literal strings symbols */
  guint cur_ls;             /* current literal string symbol */
  GHashTable *future_refs;  /* a map from symbol name to list of addresses */
  GTree *ins_table;         /* a table of compiled instructions */
  GSList *con_list;         /* CON instructions */
  GSList *alf_list;         /* ALF instructions */
  mix_address_t loc_count;  /* current memory location during compilation */
  mix_parser_err_t status;  /* outcome of compilation */
  guint err_line;           /* line of the last error */
  guint err_count;          /* no. of errors during compilation */
  guint warn_count;         /* no. of warnings during compilation */
  mix_address_t start;      /* start address of the compiled code */
  mix_address_t end;        /* end address of the compiled code */
};

/* each node of the ins_table stores a mix_word_t with the instruction
   and a source code line (for debugging and listing)
*/
typedef struct ins_node_
{
  mix_word_t ins;
  guint lineno;
} ins_node_;


/* functions to manipulate mix_parser_t during compilation */

/* access loc counter */
#define get_ploc_(parser) ((parser)->loc_count)

/* symbol table */
/* Define a new symbol with given value
 * and update previously set refs to this symbol
 */
extern mix_parser_err_t
mix_parser_define_symbol_value (mix_parser_t *parser, const gchar *name,
				mix_word_t value);

/* Define a new symbol with value equal to the current loc_count
 * and update previously set refs to this symbol
 */
extern mix_parser_err_t
mix_parser_define_symbol_here (mix_parser_t *parser, const gchar *name);

/* Set a reference to future symbol here */
extern void
mix_parser_set_future_ref (mix_parser_t *parser, const gchar *name);

/* Redefine the value of a local symbol as the current loc_count */
extern void
mix_parser_manage_local_symbol (mix_parser_t *parser, const gchar *name,
                                mix_short_t value);

/* Literal strings symbols */
extern void
mix_parser_define_ls (mix_parser_t *parser, mix_word_t value);

/* Compilation */
/* Add instruction with address the current loc_count */
extern void
mix_parser_add_ins (mix_parser_t *parser, const mix_ins_t *new_ins,
                    guint lineno);
extern void
mix_parser_add_raw (mix_parser_t *parser, mix_word_t word, guint lineno,
                    gboolean is_con);

/* Error handling */
extern void
mix_parser_log_error (mix_parser_t *parser, mix_parser_err_t error,
                      gint lineno, const gchar *comment, gboolean warn);




#endif /* XMIX_PARSER_H */

