/* -*-c-*- -------------- mixgtk_mixal.c :
 * Implementation of the functions declared in mixgtk_mixal.h
 * ------------------------------------------------------------------
 * Copyright (C) 2001, 2002, 2004, 2006, 2007 Free Software Foundation, Inc.
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


#include <stdlib.h>
#include <string.h>

#include "mixgtk_widgets.h"
#include "mixgtk_config.h"
#include "mixgtk_fontsel.h"
#include "mixgtk_mixal.h"

static mix_vm_t *vm_ = NULL;
static GtkTreeView *clist_ = NULL;
static GtkListStore *clist_store_ = NULL;

static gulong lineno_;
static GtkStatusbar *status_ = NULL;
static gint status_context_ = 0;
static GPtrArray *tips_text_ = NULL;

static GtkWidget *symbols_dlg_ = NULL;
static GtkTreeView *symbols_view_ = NULL;
static GtkListStore *symbols_store_ = NULL;
static const gchar *SYMBOLS_VIEW_NAME_ = "symbols_view";

static const gchar *bp_stock_id_ = NULL;
static const gchar *pc_stock_id_ = NULL;
static const guint bp_stock_size_ = GTK_ICON_SIZE_MENU;

enum {
  CLIST_BP_ID_COL,
  CLIST_BP_SIZE_COL,
  CLIST_ADDRESS_COL,
  CLIST_BYTECODE_COL,
  CLIST_CODE_COL,
  CLIST_ADDRESS_NO_COL,
  CLIST_LINE_NO_COL,
  CLIST_COL_NO
};

enum {
  SYMBOLS_NAME_COL,
  SYMBOLS_VALUE_COL,
  SYMBOLS_WORD_COL,
  SYMBOLS_COL_NO
};


static gboolean
mixal_event_ (GtkWidget *w, GdkEvent *event, gpointer data);


static void
init_symbols_ (void)
{
  GtkCellRenderer *renderer = gtk_cell_renderer_text_new ();

  symbols_dlg_ = mixgtk_widget_factory_get_dialog (MIXGTK_SYMBOLS_DIALOG);
  g_assert (symbols_dlg_);
  symbols_view_ = GTK_TREE_VIEW
    (mixgtk_widget_factory_get_child_by_name
     (MIXGTK_SYMBOLS_DIALOG, SYMBOLS_VIEW_NAME_));
  g_assert (symbols_view_);

  mixgtk_fontsel_set_font (MIX_FONT_SYMBOLS, GTK_WIDGET (symbols_view_));

  symbols_store_ = gtk_list_store_new (SYMBOLS_COL_NO,
                                       G_TYPE_STRING,
                                       G_TYPE_STRING,
                                       G_TYPE_STRING);

  gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (symbols_store_),
                                        SYMBOLS_NAME_COL,
                                        GTK_SORT_ASCENDING);

  gtk_tree_view_set_model (symbols_view_, GTK_TREE_MODEL (symbols_store_));
  g_object_unref (G_OBJECT (symbols_store_));
  gtk_tree_view_append_column
    (symbols_view_,
     gtk_tree_view_column_new_with_attributes ("Name", renderer,
                                               "text", SYMBOLS_NAME_COL,
                                               NULL));
  gtk_tree_view_append_column
    (symbols_view_,
     gtk_tree_view_column_new_with_attributes ("Value", renderer,
                                               "text", SYMBOLS_VALUE_COL,
                                               NULL));
  gtk_tree_view_append_column
    (symbols_view_,
     gtk_tree_view_column_new_with_attributes ("", renderer,
                                               "text", SYMBOLS_WORD_COL,
                                               NULL));
}

static void
insert_symbol_ (gpointer symbol, gpointer value, gpointer data)
{
  enum {DEC_SIZE = 25, WORD_SIZE = 20};
  static gchar DEC[DEC_SIZE], WORD[WORD_SIZE];

  GtkTreeIter iter;

  mix_word_t w = (mix_word_t)GPOINTER_TO_INT (value);
  g_snprintf (DEC, DEC_SIZE, "%s%ld",
              mix_word_is_negative (w)? "-" : "+",
              mix_word_magnitude (w));
  mix_word_print_to_buffer (w, WORD);

  gtk_list_store_append (symbols_store_, &iter);
  gtk_list_store_set (symbols_store_, &iter,
                      SYMBOLS_NAME_COL, (const gchar*)symbol,
                      SYMBOLS_VALUE_COL, DEC,
                      SYMBOLS_WORD_COL, WORD,
                      -1);
}

static void
fill_symbols_ (const mix_symbol_table_t *table)
{
  if (symbols_view_)
    gtk_list_store_clear (symbols_store_);
  else
    init_symbols_ ();

  mix_symbol_table_foreach (((mix_symbol_table_t *)table),
			    insert_symbol_, NULL);
}

static void
init_clist_ (void)
{
  GtkStockItem item;

  GtkCellRenderer *renderer = gtk_cell_renderer_text_new ();
  GtkCellRenderer *bprenderer = gtk_cell_renderer_pixbuf_new ();

  clist_ = GTK_TREE_VIEW (gtk_tree_view_new ());

  clist_store_ = gtk_list_store_new (CLIST_COL_NO,
                                     G_TYPE_STRING,
                                     G_TYPE_INT,
                                     G_TYPE_STRING,
                                     G_TYPE_STRING,
                                     G_TYPE_STRING,
                                     G_TYPE_INT,
                                     G_TYPE_INT);

  gtk_tree_view_set_model (clist_, GTK_TREE_MODEL (clist_store_));
  g_object_unref (G_OBJECT (clist_store_));
  gtk_tree_view_set_rules_hint (clist_, TRUE);

  gtk_tree_view_append_column
    (clist_,
     gtk_tree_view_column_new_with_attributes ("", bprenderer,
                                               "stock-id", CLIST_BP_ID_COL,
                                               "stock-size", CLIST_BP_SIZE_COL,
                                               NULL));
  gtk_tree_view_append_column
    (clist_,
     gtk_tree_view_column_new_with_attributes ("Address", renderer,
                                               "text", CLIST_ADDRESS_COL,
                                               NULL));
  gtk_tree_view_append_column
    (clist_,
     gtk_tree_view_column_new_with_attributes ("Bytecode", renderer,
                                               "text", CLIST_BYTECODE_COL,
                                               NULL));
  gtk_tree_view_append_column
    (clist_,
     gtk_tree_view_column_new_with_attributes ("Source", renderer,
                                               "text", CLIST_CODE_COL,
                                               NULL));

  if (gtk_stock_lookup (GTK_STOCK_STOP, &item))
    {
      bp_stock_id_ = item.stock_id;
    }

  if (gtk_stock_lookup (GTK_STOCK_GO_FORWARD, &item))
    {
      pc_stock_id_ = item.stock_id;
    }

  symbols_dlg_ = NULL;
  symbols_view_ = NULL;

  g_signal_connect (G_OBJECT (clist_), "event",
                    G_CALLBACK (mixal_event_), NULL);


  mixgtk_fontsel_set_font (MIX_FONT_MIXAL, GTK_WIDGET (clist_));

  gtk_widget_show (GTK_WIDGET (clist_));

}


/* initialise the mixal widgets */
GtkWidget *
mixgtk_mixal_init (mix_vm_t *vm)
{
  if (vm != NULL) vm_ = vm;

  if (vm_ == NULL) return FALSE;

  if (clist_ == NULL) init_clist_ ();

  status_ = NULL;

  return GTK_WIDGET (clist_);
}

void
mixgtk_mixal_reparent (GtkStatusbar *status)
{
  g_assert (status != NULL);
  mixgtk_mixal_pop_status ();
  status_ = status;
  status_context_ = gtk_statusbar_get_context_id (status_, "MIXAL status");
}

void
mixgtk_mixal_update_fonts (void)
{
  mixgtk_fontsel_set_font (MIX_FONT_MIXAL, GTK_WIDGET (clist_));
  mixgtk_fontsel_set_font (MIX_FONT_SYMBOLS, GTK_WIDGET (symbols_view_));
}

void
mixgtk_mixal_pop_status (void)
{
  if (status_ != NULL)
    gtk_statusbar_pop (status_, status_context_);
}

/* load the corresponding mixal file */
static void
update_tips_ (const mix_symbol_table_t *table,
	      const gchar *line)
{
  enum {SIZE = 256};
  static gchar BUFFER[256];
  static const gchar *DELIMITERS = " /+*=-()\t,:\n";
  if (line)
    {
      guint k = 0;
      gchar *tip = g_strdup ("");
      gchar *new_tip;
      gchar **tokens;
      gchar *text = g_strdup (line);
      text = g_strdelimit (text, DELIMITERS, ' ');
      tokens = g_strsplit (g_strstrip (text), " ", -1);
      while (tokens[k])
	{
	  if (mix_symbol_table_is_defined (table, tokens[k]))
	    {
	      mix_word_t val = mix_symbol_table_value (table, tokens[k]);
	      g_snprintf (BUFFER, SIZE, "[ %s = %s%ld ]", tokens[k],
			mix_word_is_negative (val)? "-" : "+",
			mix_word_magnitude (val));
	      new_tip = g_strconcat (tip, " ", BUFFER, NULL);
	      g_free (tip);
	      tip = new_tip;
	    }
	  ++k;
	}
      g_ptr_array_add (tips_text_, (gpointer)tip);
      g_strfreev (tokens);
      g_free (text);
    }
}

void
mixgtk_mixal_load_file (void)
{
  enum {ADDR_SIZE = 20, CONT_SIZE = 200, WORD_SIZE = 20};
  static gchar ADDR[ADDR_SIZE], CONT[CONT_SIZE], WORD[WORD_SIZE];

  const mix_src_file_t *file;
  GtkTreeIter iter;

  g_assert (vm_);
  g_assert (clist_);

  gtk_list_store_clear (clist_store_);
  if (tips_text_)
    {
      g_ptr_array_free (tips_text_, TRUE);
      tips_text_ = NULL;
    }

  file = mix_vm_get_src_file (vm_);
  if (file != NULL)
    {
      gint k;

      mix_address_t addr;
      const mix_symbol_table_t *table =	mix_vm_get_symbol_table (vm_);

      if (table) tips_text_ = g_ptr_array_new ();

      lineno_ = mix_src_file_get_line_no (file);

      for (k = 0; k < lineno_; ++k)
	{
	  gchar *line =
            g_strchomp ((gchar *)mix_src_file_get_line (file, k + 1));

	  g_snprintf (CONT, CONT_SIZE, " %03d:  %s", k + 1, line);
	  addr = mix_vm_get_lineno_address (vm_, k + 1);
	  if (addr != MIX_VM_CELL_NO)
	    {
	      g_snprintf (ADDR, ADDR_SIZE, "%04d", mix_short_magnitude (addr));
	      mix_word_print_to_buffer (mix_vm_get_addr_contents (vm_, addr),
					WORD);
	    }
	  else
            {
              g_snprintf (ADDR, ADDR_SIZE, _("N/A"));
              g_snprintf (WORD, WORD_SIZE, _("N/A"));
            }
          gtk_list_store_append (clist_store_, &iter);
          gtk_list_store_set (clist_store_, &iter,
                              CLIST_ADDRESS_NO_COL, mix_short_magnitude (addr),
                              CLIST_LINE_NO_COL, k + 1,
                              CLIST_ADDRESS_COL, ADDR,
                              CLIST_CODE_COL, CONT,
                              CLIST_BYTECODE_COL, WORD,
                              CLIST_BP_SIZE_COL, bp_stock_size_,
                              -1);
          if (table) update_tips_ (table, line);
	}
      if (table) fill_symbols_ (table);
    }
  else
    {
      lineno_ = 0;
      gtk_list_store_append (clist_store_, &iter);
      gtk_list_store_set (clist_store_, &iter,
                          CLIST_ADDRESS_NO_COL, MIX_VM_CELL_NO,
                          CLIST_LINE_NO_COL, 0,
                          CLIST_ADDRESS_COL, "",
                          CLIST_CODE_COL, _("Source not available"),
                          CLIST_BYTECODE_COL, "",
                          -1);
    }
}


/* update the widgets */
static gint
find_address_ (gint address)
{
  GtkTreeIter iter;
  gint addr;
  gint row = 0;
  gboolean valid =
    gtk_tree_model_get_iter_first (GTK_TREE_MODEL (clist_store_), &iter);

  while (valid)
    {
      ++row;
      gtk_tree_model_get (GTK_TREE_MODEL (clist_store_), &iter,
                          CLIST_ADDRESS_NO_COL, &addr, -1);
      if (addr == address) return row;
      valid = gtk_tree_model_iter_next (GTK_TREE_MODEL (clist_store_), &iter);
    }
  return 0;
}

static void
update_bp_ (gint row)
{
  if (row > 0)
    {
      GtkTreePath *path = gtk_tree_path_new_from_indices (row - 1, -1);
      GtkTreeIter iter;

      if (gtk_tree_model_get_iter (GTK_TREE_MODEL (clist_store_), &iter, path))
        {
          gint address;
          const gchar *id = NULL;

          gtk_tree_model_get (GTK_TREE_MODEL (clist_store_), &iter,
                              CLIST_ADDRESS_NO_COL, &address, -1);
          if (mix_vm_has_breakpoint_at_address (vm_, address))
            id = bp_stock_id_;
          else if (address ==
                   (gint) mix_short_magnitude (mix_vm_get_prog_count (vm_)))
            id = pc_stock_id_;

          gtk_list_store_set (clist_store_, &iter, CLIST_BP_ID_COL, id, -1);
        }
      gtk_tree_path_free (path);
    }
}

static void
select_row_ (gint row)
{
  static gint previous = 0;

  if (row > 0 && previous != row)
    {
      GtkTreePath *path = gtk_tree_path_new_from_indices (row - 1, -1);
      GtkTreeIter iter;

      if (gtk_tree_model_get_iter (GTK_TREE_MODEL (clist_store_), &iter, path))
        {
          gtk_list_store_set (clist_store_, &iter,
                              CLIST_BP_ID_COL, pc_stock_id_,
                              -1);
          update_bp_ (previous);
          previous = row;
          gtk_tree_view_scroll_to_cell (clist_, path, NULL, FALSE, 0.3, 0);
        }
      gtk_tree_path_free (path);
    }
}

void
mixgtk_mixal_update (void)
{
  gint addr = 0;

  g_assert (vm_);

  addr = (gint) mix_short_magnitude (mix_vm_get_prog_count (vm_));
  select_row_ (find_address_ (addr));
}

/* breakpoints */
void
mixgtk_mixal_update_bp_at_address (guint addr)
{
  update_bp_ (find_address_ ((gint)addr));
}

void
mixgtk_mixal_update_bp_at_line (guint line)
{
  update_bp_ ((gint)line);
}

void
mixgtk_mixal_update_bp_all (void)
{
  gint k;
  for (k = 1; k <= lineno_; ++k) update_bp_ (k);
  mixgtk_mixal_update ();
}

/* callbacks */
static void
mixal_row_clicked_ (GtkTreeIter *iter)
{

  gint addr = MIX_VM_CELL_NO;
  gtk_tree_model_get (GTK_TREE_MODEL (clist_store_),
                      iter, CLIST_ADDRESS_NO_COL, &addr, -1);

  if (addr < MIX_VM_CELL_NO)
    {
      gboolean isset = mix_vm_has_breakpoint_at_address (vm_, addr);
      if (isset)
        mix_vm_clear_breakpoint_address (vm_, addr);
      else
        mix_vm_set_breakpoint_address (vm_, addr);
      update_bp_ (find_address_ (addr));
    }
}

static void
mixal_motion_ (GtkTreeIter *iter)
{
  static gint last_row = 0;
  static guint last_message = 0;

  gint row = last_row;

  gtk_tree_model_get (GTK_TREE_MODEL (clist_store_), iter,
                      CLIST_LINE_NO_COL, &row, -1);

  if (row > 0 && row != last_row && tips_text_)
    {
      const gchar *msg = NULL;
      last_row = row;
      if (last_message)
        gtk_statusbar_remove (status_, status_context_, last_message);
      msg = (const gchar *)g_ptr_array_index (tips_text_, row - 1);
      if (msg)
        last_message = gtk_statusbar_push (status_, status_context_, msg);
      else
        last_message = 0;
    }
}

static gboolean
mixal_event_ (GtkWidget *w, GdkEvent *event, gpointer data)
{
  GdkEventType type = event->type;
  if ((type == GDK_BUTTON_PRESS || type == GDK_MOTION_NOTIFY)
      && (gtk_tree_view_get_bin_window (clist_) == event->any.window))
    {
      gdouble x = (type == GDK_BUTTON_PRESS)? event->button.x : event->motion.x;
      gdouble y = (type == GDK_BUTTON_PRESS)? event->button.y : event->motion.y;
      GtkTreeIter iter;
      GtkTreePath *path = NULL;
      GtkTreeViewColumn *col = NULL;
      if (gtk_tree_view_get_path_at_pos (clist_, x, y, &path, &col, NULL, NULL)
          && gtk_tree_model_get_iter (GTK_TREE_MODEL (clist_store_),
                                      &iter, path))
        {
          if (type == GDK_BUTTON_PRESS) mixal_row_clicked_ (&iter);
          else mixal_motion_ (&iter);
        }
      if (path) gtk_tree_path_free (path);
      return TRUE;
    }
  return FALSE;
}

void
on_symbol_ok_clicked ()
{
  gtk_widget_hide (symbols_dlg_);
}

void
on_symbols_activate ()
{
  if (!symbols_dlg_) init_symbols_ ();
  gtk_widget_show (symbols_dlg_);
}

void
on_mixal_leave_notify_event ()
{
  if (status_ != NULL) gtk_statusbar_pop (status_, status_context_);
}

void
on_mixal_font_activate ()
{
  GtkWidget *w[] = { GTK_WIDGET (clist_) };
  mixgtk_fontsel_query_font (MIX_FONT_MIXAL, w, 1);
}

void
on_symbols_font_activate ()
{
  if (symbols_view_ != NULL)
    {
      GtkWidget *w[] = { GTK_WIDGET (symbols_view_) };
      mixgtk_fontsel_query_font (MIX_FONT_SYMBOLS, w, 1);
    }
  else
    mixgtk_fontsel_query_font (MIX_FONT_SYMBOLS, NULL, 0);
}
