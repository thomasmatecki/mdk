/* -*-c-*- -------------- mix_parser.c :
 * Implementation of the functions declared in mix_parser.h and
 * xmix_parser.h
 * ------------------------------------------------------------------
 * Copyright (C) 2000, 2001, 2003, 2004, 2006, 2007, 2009 Free Software Foundation, Inc.
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

#include <string.h>

#include "mix.h"
#include "mix_code_file.h"
#include "xmix_parser.h"

/* The flex-generated scanner, according to file mix_scanner.l */
extern mix_parser_err_t
mix_flex_scan (mix_parser_t *parser);

/*------------ mixparser.h functions -------------------------------------*/

/* error messages */
static const gchar * const ERR_MESSAGE_[] = {
  N_("successful compilation"),
  N_("file not yet compiled"),
  N_("internal error"),
  N_("unable to open MIX source file"),
  N_("unable to open MIX output file"),
  N_("unexpected end of file"),
  N_("invalid location field"),
  N_("duplicated symbol"),
  N_("symbol too long"),
  N_("missing operator field"),
  N_("unexpected location symbol"),
  N_("invalid address field"),
  N_("invalid index field"),
  N_("invalid f-specification"),
  N_("invalid operation field"),
  N_("invalid expression"),
  N_("undefined symbol"),
  N_("mismatched parenthesis"),
  N_("unexpected f-specfication"),
  N_("missing symbol name"),
  N_("symbol is an instruction name"),
  N_("failed write access to code file"),
  N_("operand of ALF pseudo instruction has less than 5 chars"),
  N_("operand of ALF pseudo instruction has more than 5 chars"),
  N_("operand of ALF pseudo instruction must be quoted")
};

static const guint NO_OF_MESSAGES_ = sizeof(ERR_MESSAGE_)/sizeof (gchar*);

const gchar *
mix_parser_err_string (mix_parser_err_t error)
{
  return (error < NO_OF_MESSAGES_) ? _(ERR_MESSAGE_[error]) : NULL;
}

guint
mix_parser_err_count (const mix_parser_t *parser)
{
  return (parser) ? parser->err_count : 0;
}

guint
mix_parser_warning_count (const mix_parser_t *parser)
{
  return (parser) ? parser->warn_count : 0;
}

const gchar *
mix_parser_src_file_base_name (const mix_parser_t *parser)
{
  return (parser) ? mix_file_base_name (parser->in_file) : NULL;
}

const gchar *
mix_parser_src_file_extension (const mix_parser_t *parser)
{
  return (parser) ? mix_file_extension (parser->in_file) : NULL;
}


/* Create/destroy a mix_parser */
/* compare function for the table of ins */
static gint
compare_shorts_ (gconstpointer s1, gconstpointer s2)
{
  mix_short_t a = (mix_short_t)GPOINTER_TO_UINT (s1);
  mix_short_t b = (mix_short_t)GPOINTER_TO_UINT (s2);
  if ( mix_short_sign (a) == mix_short_sign (b) )
    return mix_short_magnitude (a) - mix_short_magnitude (b);
  else if ( mix_short_magnitude (a) == 0 && mix_short_magnitude (b) == 0 )
    return 0;
  else if ( mix_short_is_positive (a) )
    return 1;
  return -1;
}

mix_parser_t *
mix_parser_new (const gchar *in_file)
{
  mix_parser_t *result;
  mix_file_t *f = mix_file_new_with_def_ext (in_file, mix_io_READ,
					     MIX_SRC_DEFEXT);

  if ( f == NULL ) return NULL;

  result = g_new (mix_parser_t, 1);
  result->symbol_table = mix_symbol_table_new ();
  result->ls_table = mix_symbol_table_new ();
  result->cur_ls = 0;
  result->future_refs = g_hash_table_new (g_str_hash, g_str_equal);
  result->ins_table = g_tree_new (compare_shorts_);
  if ( result->symbol_table == NULL || result->future_refs == NULL
       || result->ins_table == NULL || result->ls_table == NULL )
    {
      mix_symbol_table_delete (result->symbol_table);
      mix_symbol_table_delete (result->ls_table);
      g_hash_table_destroy (result->future_refs);
      g_tree_destroy (result->ins_table);
      mix_file_delete (f);
      g_free (result);
      g_warning (_("No system resources"));
      return NULL;
    }
  result->con_list = NULL;
  result->alf_list = NULL;
  result->in_file = f;
  result->loc_count = MIX_SHORT_ZERO;
  result->start = MIX_SHORT_ZERO;
  result->end = MIX_SHORT_ZERO;
  result->status = MIX_PERR_NOCOMP;
  result->err_line = 0;
  result->err_count = 0;
  result->warn_count = 0;
  return result;
}

static void
delete_list_vals_ (gpointer key, gpointer value, gpointer data)
{
  g_free (key);
  g_slist_free ((GSList*)value);
}

static int
delete_tree_vals_ (gpointer key, gpointer value, gpointer data)
{
  g_free (value);
  return FALSE;
}

void
mix_parser_delete (mix_parser_t *parser)
{
  g_return_if_fail (parser != NULL);
  /* clear the GSList values of future_refs and its keys */
  g_hash_table_foreach (parser->future_refs, delete_list_vals_, NULL);
  /* clear the ins_node_'s of the ins tree */
  g_tree_foreach (parser->ins_table, delete_tree_vals_, NULL);
  /* destroy the tree and hash tables */
  g_tree_destroy (parser->ins_table);
  mix_symbol_table_delete (parser->symbol_table);
  mix_symbol_table_delete (parser->ls_table);
  g_hash_table_destroy (parser->future_refs);
  g_slist_free (parser->con_list);
  g_slist_free (parser->alf_list);
  mix_file_delete (parser->in_file);
  g_free (parser);
}

/* Compile a mix source file */
static void
update_future_refs_value_ (mix_parser_t *parser, const gchar *name,
			   mix_short_t value, gboolean remove)
{
  GSList *list = NULL;
  gpointer *plist = (gpointer *)(&list);
  gpointer key;

  g_assert (parser != NULL && name != NULL);
  if ( g_hash_table_lookup_extended (parser->future_refs, name, &key, plist) )
    {
      GSList *tmp = list;
      ins_node_ *node;
      while ( tmp != NULL )
	{
	  node =
	    (ins_node_ *)g_tree_lookup (parser->ins_table, tmp->data);
	  g_assert (node);
	  if (mix_get_ins_address (node->ins) == 1) {
	    value = mix_short_negative (value);
	    node->ins = mix_word_set_field (node->ins,
					    MIX_WORD_ZERO,
					    mix_fspec_new (1,2));
	  }
	  mix_word_add_address (node->ins, value);
	  g_tree_insert (parser->ins_table, tmp->data, (gpointer)node);
	  tmp = g_slist_next (tmp);
	}
      if (remove) {
        g_hash_table_remove (parser->future_refs, name);
        g_free (key);
      }
      g_slist_free (list);
    }
}

#define update_future_refs_(parser,name,rem) \
  update_future_refs_value_(parser, name, (parser)->loc_count, rem)

static void
add_raw_ (mix_parser_t *parser, mix_word_t word, guint lineno)
{
  if ( parser->status == MIX_PERR_NOCOMP || parser->status == MIX_PERR_OK )
    {
      ins_node_ *node = g_new (ins_node_, 1);
      node->ins = word;
      node->lineno = lineno;
      g_tree_insert (parser->ins_table,
                     GUINT_TO_POINTER ((guint)parser->loc_count),
		     (gpointer)node);
    }
}

static void
update_ls_ (gpointer symbol, gpointer value, gpointer parser)
{ /* add an instruction on current location and update refs to it */
  mix_word_t w = (mix_word_t) GPOINTER_TO_UINT (value);
  mix_parser_t *par = (mix_parser_t *) parser;
  update_future_refs_ (par, (const gchar *)symbol, TRUE);
  add_raw_ (par, w, 0);
  par->loc_count++;
}

static gboolean
undef_warning_ (gpointer symbol, gpointer value, gpointer data)
{
  mix_parser_t *parser = (mix_parser_t *)data;
  const gchar *name = (const gchar *)symbol;
  mix_ins_t ins;

  mix_parser_log_error (parser, MIX_PERR_UNDEF_SYM, 0, name, TRUE);

  mix_word_to_ins_uncheck (MIX_WORD_ZERO, ins);
  update_future_refs_ (parser, name, FALSE);
  mix_parser_add_ins (parser, &ins, 0);
  mix_symbol_table_insert (parser->symbol_table, name,
                           mix_short_to_word_fast (parser->loc_count));

  parser->loc_count++;
  return TRUE;
}

mix_parser_err_t
mix_parser_compile (mix_parser_t *parser)
{
  g_return_val_if_fail (parser != NULL, MIX_PERR_INTERNAL);
  g_return_val_if_fail (parser->in_file != NULL, MIX_PERR_NOIN);
  g_return_val_if_fail (parser->symbol_table != NULL, MIX_PERR_INTERNAL);
  g_return_val_if_fail (parser->future_refs != NULL, MIX_PERR_INTERNAL);
  g_return_val_if_fail (parser->ins_table != NULL, MIX_PERR_INTERNAL);

  parser->status = mix_flex_scan (parser);

  if ( parser->status == MIX_PERR_OK )
    {
      parser->loc_count = parser->end;
      mix_symbol_table_foreach (parser->ls_table, update_ls_, (gpointer)parser);
      if ( g_hash_table_size (parser->future_refs) > 0)
        {
          g_hash_table_foreach_remove (parser->future_refs,
                                       undef_warning_, (gpointer)parser);
        }
    }

  return parser->status;
}

/* Write a compiled source to a code file */
struct write_code_context_
{
  mix_code_file_t *file;
  mix_parser_t *parser;
};

static gint
write_code_ (gpointer address, gpointer ins_node, gpointer context)
{
  mix_ins_desc_t desc;
  struct write_code_context_ *cntx = (struct write_code_context_ *)context;
  desc.ins = ((ins_node_ *)ins_node)->ins;
  desc.lineno = ((ins_node_ *)ins_node)->lineno;
  desc.address = (mix_address_t)GPOINTER_TO_UINT (address);
  if ( mix_code_file_write_ins (cntx->file, &desc) )
    return FALSE;
  else
    {
      cntx->parser->status = MIX_PERR_NOWRITE;
      return TRUE;
    }
}

mix_parser_err_t
mix_parser_write_code (mix_parser_t *parser, const gchar *code_file,
		       gboolean debug)
{
  struct write_code_context_ context;
  const gchar *cfname = (code_file) ?
    code_file : mix_file_base_name (parser->in_file);
  gchar *source_path;

  g_return_val_if_fail (parser != NULL, MIX_PERR_INTERNAL);
  if  (parser->status != MIX_PERR_OK ) return parser->status;
  context.parser = parser;
  if (!g_path_is_absolute (mix_file_base_name (parser->in_file)))
    {
      gchar *dir = g_get_current_dir ();
      source_path = g_strconcat (dir, G_DIR_SEPARATOR_S,
				 mix_file_base_name (parser->in_file), NULL);
      g_free (dir);
    }
  else
    source_path = g_strdup (mix_file_base_name (parser->in_file));

  context.file = mix_code_file_new_write (cfname, parser->start, source_path,
					  debug, parser->symbol_table);
  g_free (source_path);

  if (context.file == NULL) return MIX_PERR_NOOUT;
  g_tree_foreach (parser->ins_table, write_code_, (gpointer)&context);
  mix_code_file_delete (context.file);
  return parser->status;
}

/* Produce a listing file summarising the compilation */
typedef struct
{
  FILE *file;
  mix_parser_t *parser;
} listing_context_t;

static gint
write_listing_ (gpointer address, gpointer ins, gpointer context)
{
  guint k;
  FILE *file = ((listing_context_t *)context)->file;
  mix_parser_t *parser = ((listing_context_t *)context)->parser;
  guint end = parser->end;
  ins_node_ *ins_node = (ins_node_ *)ins;
  mix_ins_t instruct;

  fprintf (file, "%03d     %05d   %s ",
           ins_node->lineno,
           GPOINTER_TO_INT (address),
	   mix_word_is_negative (ins_node->ins)? "-":"+");
  for ( k = 1; k < 6; ++k )
    fprintf (file, "%02d ", mix_word_get_byte (ins_node->ins, k));

  if (g_slist_find (parser->con_list, GUINT_TO_POINTER (ins_node->lineno))
      || GPOINTER_TO_UINT (address) >=  end)
    fprintf (file, "\tCON\t%04d\n", (int)(ins_node->ins));
  else if (g_slist_find (parser->alf_list, GUINT_TO_POINTER (ins_node->lineno)))
    {
      size_t i;
      fprintf (file, "\tALF\t\"");
      for (i = 1; i < 6; ++i)
        fprintf (file, "%c",
                 mix_char_to_ascii (mix_byte_to_char
                                    (mix_word_get_byte (ins_node->ins, i))));
      fprintf (file, "\"\n");
    }
  else if (GPOINTER_TO_UINT (address) <  end)
    {
      gchar *instext = NULL;
      mix_ins_id_t id = mix_word_to_ins (ins_node->ins, &instruct);
      if (id != mix_INVALID_INS)
        instext = mix_ins_to_string (&instruct);
      fprintf (file, _("\t%s\n"), instext? instext : _("UNKNOWN"));
      if (instext) g_free (instext);
    }
  else
    g_assert_not_reached ();

  return FALSE;
}

mix_parser_err_t
mix_parser_write_listing (mix_parser_t *parser, const gchar *list_file)
{
  mix_file_t *mfile;
  const gchar *name;
  listing_context_t context;
  static const char *sep =
    "-----------------------------------------------------------------\n";

  g_return_val_if_fail (parser != NULL, MIX_PERR_INTERNAL);
  if (parser->status != MIX_PERR_OK ) return parser->status;
  name = (list_file) ? list_file : mix_file_base_name (parser->in_file);
  mfile =  mix_file_new_with_def_ext (name, mix_io_WRITE, MIX_LIST_DEFEXT);
  if ( mfile == NULL ) return MIX_PERR_NOOUT;
  context.file = mix_file_to_FILE (mfile);
  context.parser = parser;
  fprintf (context.file, _("*** %s%s: compilation summary ***\n\n"),
	   mix_file_base_name (parser->in_file),
	   mix_file_extension (parser->in_file));
  fputs (sep, context.file);
  fputs ( _("Src     Address  Compiled word           Symbolic rep\n"),
	  context.file);
  fputs (sep, context.file);
  g_tree_foreach (parser->ins_table, write_listing_, (gpointer)(&context));
  fputs (sep, context.file);
  fprintf (context.file, _("\n*** Start address:\t%d\n*** End address:\t%d\n"),
	   mix_short_magnitude (parser->start),
           mix_short_magnitude (parser->end));
  fprintf (context.file, _("\n*** Symbol table\n"));
  mix_symbol_table_print (parser->symbol_table, MIX_SYM_ROWS,
                          context.file, TRUE);
  fprintf (context.file, _("\n*** End of summary ***\n"));
  mix_file_delete (mfile);
  return parser->status;
}

/* load a virtual machine's memory with the contents of a compiled file */
static gint
load_vm_ (gpointer address, gpointer ins, gpointer vm)
{
  mix_vm_set_addr_contents ((mix_vm_t*)vm,
			    (mix_address_t)GPOINTER_TO_UINT (address),
			    ((ins_node_ *)ins)->ins);
  return FALSE;
}

mix_parser_err_t
mix_parser_load_vm (const mix_parser_t *parser, mix_vm_t *vm)
{
  g_return_val_if_fail (parser != NULL, MIX_PERR_INTERNAL);
  g_return_val_if_fail (vm != NULL, MIX_PERR_INTERNAL);
  g_return_val_if_fail (parser->status == MIX_PERR_OK, parser->status);
  mix_vm_reset (vm);
  g_tree_foreach (parser->ins_table, load_vm_, (gpointer)vm);
  mix_vm_set_start_addr (vm, parser->start);
  return parser->status;
}



/*------------ xmiparser.h functions -------------------------------------*/
/* functions to manipulate mix_parser_t during compilation */


/* symbol table */
/* Define a new symbol with value equal to the current loc_count
 * and update future refs to this symbol
 */
mix_parser_err_t
mix_parser_define_symbol_here (mix_parser_t *parser, const gchar *name)
{
  mix_word_t value = mix_short_to_word_fast (parser->loc_count);
  return mix_parser_define_symbol_value (parser, name, value);
}

mix_parser_err_t
mix_parser_define_symbol_value (mix_parser_t *parser, const gchar *name,
				mix_word_t value)
{
  g_assert (parser != NULL);

  if (NULL == name || strlen (name) == 0) return MIX_PERR_MIS_SYM;

  switch (mix_symbol_table_add (parser->symbol_table, name, value))
    {
    case MIX_SYM_OK:
      if (parser->status == MIX_PERR_NOCOMP)
        update_future_refs_value_ (parser, name, value, TRUE);
      return MIX_PERR_OK;
    case MIX_SYM_LONG: return MIX_PERR_LONG_SYMBOL;
    case MIX_SYM_DUP: return MIX_PERR_DUP_SYMBOL;
    default: return MIX_PERR_INTERNAL;
    }
}

/* Obtain the value of a symbol */
void
mix_parser_set_future_ref (mix_parser_t *parser, const gchar *name)
{
  const gchar *nname = name;
  GSList *list;

  g_assert (parser != NULL && name != NULL);

  if ( parser->status == MIX_PERR_NOCOMP )
    {
      list =  g_hash_table_lookup (parser->future_refs, name);
      if ( list == NULL ) nname = g_strdup (name);
      list = g_slist_prepend (list, GUINT_TO_POINTER ((guint)parser->loc_count));
      g_hash_table_insert (parser->future_refs, (gpointer)nname, list);
    }
}

/* Redefine the value of a local symbol as the current loc_count */
void
mix_parser_manage_local_symbol (mix_parser_t *parser, const gchar *name,
				mix_short_t value)
{
  gchar ref[3];
  ref[2] = 0;

  g_assert (parser != NULL && name != NULL);
  g_assert (strlen(name) == 2);

  switch (name[1])
    {
    case 'f': case 'F':
      mix_parser_set_future_ref (parser, name);
      break;
    case 'h': case 'H':
      ref[0] = name[0];
      ref[1] = 'F';
      if ( parser->status == MIX_PERR_NOCOMP )
	update_future_refs_value_ (parser, ref, value, TRUE);
      ref[1] = 'B';
      mix_symbol_table_insert (parser->symbol_table, ref,
			       mix_short_to_word_fast (value));
      break;
    default:
      return;
    }
}

/* Literal strings symbols */
void
mix_parser_define_ls (mix_parser_t *parser, mix_word_t value)
{
  gchar *name = g_strdup_printf ("%05d", parser->cur_ls++);
  mix_symbol_table_add (parser->ls_table, name, value);
  mix_parser_set_future_ref (parser, name);
  g_free (name);
}

/* Compilation */
void
mix_parser_add_ins (mix_parser_t *parser, const mix_ins_t *new_ins,
		    guint lineno)
{
  g_assert (parser != NULL && new_ins != NULL);
  add_raw_ (parser, mix_ins_to_word_uncheck (*new_ins), lineno);
}

void
mix_parser_add_raw (mix_parser_t *parser, mix_word_t word, guint lineno,
                    gboolean is_con)
{
  g_assert (parser != NULL);
  add_raw_ (parser, word, lineno);
  if ( parser->status == MIX_PERR_NOCOMP || parser->status == MIX_PERR_OK )
    {
      if (is_con)
        parser->con_list = g_slist_append (parser->con_list,
                                           GUINT_TO_POINTER (lineno));
      else
        parser->alf_list = g_slist_append (parser->alf_list,
                                           GUINT_TO_POINTER (lineno));
    }
}

/* Error handling */
void
mix_parser_log_error (mix_parser_t *parser, mix_parser_err_t error,
		      gint lineno, const gchar *comment, gboolean warn)
{
  g_assert (parser != NULL);
  if ( warn )
    parser->warn_count += 1;
  else
    {
      parser->err_count += 1;
      parser->err_line = lineno;
      parser->status = error;
    }

  fprintf (stderr, "%s%s:%d: %s: %s",
	   mix_file_base_name (parser->in_file),
	   mix_file_extension (parser->in_file),
	   lineno, warn ? _("warning"):_("error"), _(ERR_MESSAGE_[error]));

  if (comment != NULL)
    fprintf (stderr, ": %s\n", comment);
  else
    fputs ("\n", stderr);
}
