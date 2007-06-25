/* -*-c-*- -------------- mix_symbol_table.c :
 * Implementation of the functions declared in mix_symbol_table.h
 * ------------------------------------------------------------------
 * Copyright (C) 2000, 2001, 2004, 2006, 2007 Free Software Foundation, Inc.
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

#include <ctype.h> /* isdigit */
#include <string.h>

#include "mix_symbol_table.h"

/* Create an empty table */
mix_symbol_table_t *
mix_symbol_table_new (void)
{
  return g_hash_table_new (g_str_hash, g_str_equal);
}

/* Create a table and populate it with the contents of a table stored
   in -file- using mix_symbol_table_print (table, MIX_SYM_LINE, file)
*/
mix_symbol_table_t *
mix_symbol_table_new_from_file (FILE *file)
{
  mix_symbol_table_t *result = mix_symbol_table_new ();
  if ( result != NULL )
    {
      gchar sym[MIX_SYM_MAX_LEN + 1];
      glong val;

      while ( getc (file) == ',' )
	{
	  if ( fscanf (file, "%s =%ld", sym, &val) != EOF )
	      mix_symbol_table_add (result, sym, mix_word_new (val));
	}
    }
  return result;
}

/* Delete a table */
static void
delete_hash_keys_ (gpointer key, gpointer value, gpointer data)
{
  g_free (key);
}

void
mix_symbol_table_delete (mix_symbol_table_t *table)
{
  g_hash_table_foreach (table, delete_hash_keys_, NULL);
  g_hash_table_destroy (table);
}

/* add/remove symbols from other table */
static void
add_symbol_ (gpointer symbol, gpointer value, gpointer target)
{
  mix_symbol_table_t *t = (mix_symbol_table_t *)target;
  gchar *s = (gchar *)symbol;
  mix_word_t v = (mix_word_t)GPOINTER_TO_UINT (value);
  mix_symbol_table_insert (t, s, v);
}

static void
remove_symbol_ (gpointer symbol, gpointer value, gpointer target)
{
  mix_symbol_table_t *t = (mix_symbol_table_t *)target;
  gchar *s = (gchar *)symbol;
  mix_symbol_table_remove (t, s);
}

gboolean
mix_symbol_table_merge_table (mix_symbol_table_t *table,
			      const mix_symbol_table_t *from)
{
  g_return_val_if_fail (table != NULL, FALSE);
  if (from != NULL)
    {
      mix_symbol_table_foreach ((gpointer)from, add_symbol_, table);
    }
  return TRUE;
}

gboolean
mix_symbol_table_substract_table (mix_symbol_table_t *table,
				  const mix_symbol_table_t *other)
{
  g_return_val_if_fail (table != NULL, FALSE);
  if (other != NULL)
    {
      mix_symbol_table_foreach ((gpointer)other, remove_symbol_, table);
    }
  return TRUE;
}


/* Add/remove symbols one by one */
gint
mix_symbol_table_add (mix_symbol_table_t *table,
		     const gchar *sym, mix_word_t value)
{
  gpointer key, val;

  if ( table == NULL || sym == NULL ) return MIX_SYM_FAIL;
  if ( strlen (sym) > MIX_SYM_MAX_LEN ) return  MIX_SYM_LONG;

  if ( !g_hash_table_lookup_extended (table, sym, &key, &val) )
    {
      key = g_strdup (sym);
      g_hash_table_insert (table, key, GUINT_TO_POINTER (value));
      return MIX_SYM_OK;
    }
  else
    return MIX_SYM_DUP;
}

/* Add or modify symbol if it exists */
gint
mix_symbol_table_insert (mix_symbol_table_t *table,
			const gchar *sym, mix_word_t new_value)
{
  gpointer key, val;

  if ( table == NULL || sym == NULL ) return MIX_SYM_FAIL;
  if ( strlen (sym) > MIX_SYM_MAX_LEN ) return MIX_SYM_LONG;

  if ( !g_hash_table_lookup_extended (table, sym, &key, &val) )
      key = g_strdup (sym);
  g_hash_table_insert (table, key, GUINT_TO_POINTER (new_value));
  return MIX_SYM_OK;
}


/* Symbols lookup */
gboolean
mix_symbol_table_is_defined (const mix_symbol_table_t *table, const gchar *sym)
{
  gpointer key, val;
  return g_hash_table_lookup_extended((GHashTable *)table, sym, &key, &val);
}

/* Print table */
#define is_local_sym_(sym) \
  ((sym) && (strlen (sym)==2) && (sym[1] == 'B') && isdigit (sym[0]))

static gboolean skip_ = FALSE;

static void
print_sym_rows_ (gpointer symbol, gpointer value, gpointer file)
{
  char *s = (char *)symbol;
  if (skip_ && !is_local_sym_ (s)) {
    mix_word_t word = (mix_word_t)GPOINTER_TO_UINT (value);
    fprintf((FILE *)file, "%-20s:  %s%ld\n", s,
	    mix_word_is_negative (word)? "-":"",
	    mix_word_magnitude (word));
  }
}

static void
print_sym_line_ (gpointer symbol, gpointer value, gpointer file)
{
  char *s = (char *)symbol;
  if (skip_ && !is_local_sym_ (s)) {
    mix_word_t word = (mix_word_t)GPOINTER_TO_UINT (value);
    fprintf((FILE *)file, ",%s =%s%ld", s,
	    mix_word_is_negative (word)? "-":"",
	    mix_word_magnitude (word));
  }
}

void
mix_symbol_table_print (const mix_symbol_table_t *table, gint mode,
		       FILE *file, gboolean skiplocal)
{
  GHFunc func = (mode == MIX_SYM_LINE)? print_sym_line_ : print_sym_rows_;
  skip_ = skiplocal;
  if ( table != NULL )
    g_hash_table_foreach ((GHashTable *)table, func, (gpointer)file);
  if ( mode == MIX_SYM_LINE ) putc (';', file); /* to mark end-of-table */
}


