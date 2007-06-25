/* -*-c-*- ---------------- mix_symbol_table.h :
 * Type mix_symbol_table_t and functions to manipulate it.
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


#ifndef MIX_SYMBOL_TABLE_H
#define MIX_SYMBOL_TABLE_H

#include <stdio.h>
#include "mix_types.h"

/* A symbol table shall be implemented as a hash table */
typedef GHashTable mix_symbol_table_t ;

/* Maximum length of stored symbols */
#define MIX_SYM_MAX_LEN  10

/* Create an empty table */
extern mix_symbol_table_t *
mix_symbol_table_new (void);

/* Create a table and populate it with the contents of a table stored
   in -file- using mix_symbol_table_print (table, MIX_SYM_LINE, file)
*/
extern mix_symbol_table_t *
mix_symbol_table_new_from_file (FILE *file);


/* Delete a table */
extern void
mix_symbol_table_delete (mix_symbol_table_t *table);

/* add/remove symbols from other table */
extern gboolean
mix_symbol_table_merge_table (mix_symbol_table_t *table,
			      const mix_symbol_table_t *from);

extern gboolean
mix_symbol_table_substract_table (mix_symbol_table_t *table,
				  const mix_symbol_table_t *other);

/* Add/remove symbols one by one */
/* possible outcomes: */
enum {
  MIX_SYM_FAIL, /* attempt failed */
  MIX_SYM_OK,
  MIX_SYM_DUP,  /* duplicated symbol */
  MIX_SYM_LONG  /* symbol too long: only MIX_SYM_MAX_LEN chars used */
};

extern gint
mix_symbol_table_add (mix_symbol_table_t *table,
		      const gchar *sym, mix_word_t value);

#define mix_symbol_table_remove(table, sym) \
  g_hash_table_remove (table, sym)


/* Add or modify symbol if it exists */
extern gint
mix_symbol_table_insert (mix_symbol_table_t *table,
			 const gchar *sym, mix_word_t new_value);

/* Add or modify symbol if it exists, without copying sym */
#define mix_symbol_table_insert_static(table,sym,value)\
	g_hash_table_insert (table,(gpointer)sym,GUINT_TO_POINTER (value))

/* Symbols lookup */
extern gboolean
mix_symbol_table_is_defined (const mix_symbol_table_t *table, const gchar *sym);

#define mix_symbol_table_value(table,sym)				    \
  (mix_word_t)GPOINTER_TO_UINT  (g_hash_table_lookup ((GHashTable *)table, \
                                (gpointer)sym))

/* Traverse the table */
#define mix_symbol_table_foreach(table,func,data) \
	g_hash_table_foreach (table,func,data)

/* Print the table */
enum {
  MIX_SYM_ROWS, /* each symbol/value in a row */
  MIX_SYM_LINE  /* {,symbol =value}*; in a single row */
};

extern void
mix_symbol_table_print (const mix_symbol_table_t *table, gint mode,
			FILE *file, gboolean skiplocal);


#endif /* MIX_SYMBOL_TABLE_H */

