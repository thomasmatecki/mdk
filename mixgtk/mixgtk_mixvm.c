/* -*-c-*- -------------- mixgtk_mixvm.c :
 * Implementation of the functions declared in mixgtk_mixvm.h
 * ------------------------------------------------------------------
 * Copyright (C) 2001, 2002, 2004, 2006, 2007, 2008 Free Software Foundation, Inc.
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
#include <mixlib/xmix_vm.h>
#include "mixgtk_widgets.h"
#include "mixgtk_fontsel.h"
#include "mixgtk_mixvm.h"
#include "mixgtk_cmd_dispatcher.h"
#include "mixgtk_input.h"


/* Local variables */

#define REGISTER_NO_ (MIXGTK_WIDGET_rI6 - MIXGTK_WIDGET_rA + 1)

static GtkEntry *reg_entries_[REGISTER_NO_];
static GtkEntry *loc_entry_;
static GtkWidget *goto_ = NULL;
static GtkEntry *goto_entry_ = NULL;
static GtkToggleButton *over_button_;
static GtkToggleButton *cmp_buttons_[3];
static GtkTreeView *memory_;
static GtkListStore *mem_store_;
static GtkLabel *laptime_;
static GtkLabel *progtime_;
static GtkLabel *uptime_;
static mix_vm_t *vm_;

#define HAVE_OLD_GTK !GTK_CHECK_VERSION(2, 12, 0)

#if HAVE_OLD_GTK
static GtkTooltips *tips_ = NULL;
#endif


/* Static function prototypes */
static void init_goto_ (void);
static void update_register_ (mixgtk_widget_id_t reg);
static gboolean init_mem_ (void);


enum {
  MEM_ADDRESS_COL,
  MEM_CONTENTS_COL,
  MEM_COL_NO
};


/* initialise the mixvm widgets */
gboolean
mixgtk_mixvm_init (mix_vm_t *vm)
{
  int k;

  g_assert (vm != NULL);

  vm_ = vm;

#if HAVE_OLD_GTK
  if (!tips_) tips_ = gtk_tooltips_new ();
#endif

  for (k = 0; k < REGISTER_NO_; ++k)
    {
      reg_entries_[k] =
        GTK_ENTRY (mixgtk_widget_factory_get(MIXGTK_MIXVM_DIALOG,
                                             MIXGTK_WIDGET_rA + k));
      g_assert (reg_entries_[k] != NULL);
      mixgtk_fontsel_set_font (MIX_FONT_MIXVM,
                               GTK_WIDGET (reg_entries_[k]));
    }

  loc_entry_ =
    GTK_ENTRY (mixgtk_widget_factory_get (MIXGTK_MIXVM_DIALOG,
                                          MIXGTK_WIDGET_LOC));
  g_assert (loc_entry_ != NULL);
  mixgtk_fontsel_set_font (MIX_FONT_MIXVM, GTK_WIDGET (loc_entry_));

  uptime_ =
    GTK_LABEL (mixgtk_widget_factory_get (MIXGTK_MIXVM_DIALOG,
                                          MIXGTK_WIDGET_UPTIME));
  g_assert (uptime_ != NULL);

  progtime_ =
    GTK_LABEL (mixgtk_widget_factory_get (MIXGTK_MIXVM_DIALOG,
                                          MIXGTK_WIDGET_PROGTIME));
  g_assert (progtime_ != NULL);

  laptime_ =
    GTK_LABEL (mixgtk_widget_factory_get (MIXGTK_MIXVM_DIALOG,
                                          MIXGTK_WIDGET_LAPTIME));
  g_assert (laptime_ != NULL);

  over_button_ =
    GTK_TOGGLE_BUTTON
    (mixgtk_widget_factory_get (MIXGTK_MIXVM_DIALOG, MIXGTK_WIDGET_OVER));
  g_assert (over_button_ != NULL);

  for (k = 0; k < 3; ++k)
    {
      cmp_buttons_[k] =	GTK_TOGGLE_BUTTON
        (mixgtk_widget_factory_get (MIXGTK_MIXVM_DIALOG,
                                    MIXGTK_WIDGET_CMP_L +k));
      g_assert (cmp_buttons_[k] != NULL);
    }
  init_mem_ ();

  return TRUE;
}

static GtkWidget **
text_widgets_ (gint *n)
{
  static GtkWidget *w[REGISTER_NO_ + 2] = { NULL};

  if (w[0] == NULL)
    {
      gint k;
      w[0] = GTK_WIDGET (loc_entry_);
      w[1] = GTK_WIDGET (memory_);
      for (k = 0; k < REGISTER_NO_; ++k)
        w[k + 2] = GTK_WIDGET (reg_entries_[k]);
  };

  if (n) *n = REGISTER_NO_ + 2;

  return w;
}

void
mixgtk_mixvm_update_fonts (void)
{
  gint k, n = 0;
  GtkWidget **w = text_widgets_ (&n);
  for (k = 0; k < n; ++k)
    mixgtk_fontsel_set_font (MIX_FONT_MIXVM, w[k]);
}

/* update register widgets */
void
mixgtk_mixvm_update_registers (void)
{
  gint k;
  for (k = MIXGTK_WIDGET_rA; k <= MIXGTK_WIDGET_rI6; ++k)
    update_register_ (k);
}

/* update the overflow toggle */
void
mixgtk_mixvm_update_over_toggle (void)
{
  g_assert (over_button_);
  gtk_toggle_button_set_active (over_button_,
				mix_vm_get_overflow (vm_));
}

/* update the comparison flag */
void
mixgtk_mixvm_update_cmp (void)
{
  gint toggle = mix_vm_get_cmpflag (vm_);
  gtk_toggle_button_set_active (cmp_buttons_[toggle], TRUE);
}


/* update the memory cells */
void
mixgtk_mixvm_update_cells (void)
{
  static gchar BUFFER[20];
  GtkTreeIter iter;
  gboolean valid =
    gtk_tree_model_get_iter_first (GTK_TREE_MODEL (mem_store_), &iter);
  gint k = 0;

  while (valid)
    {
      mix_word_print_to_buffer (mix_vm_get_addr_contents (vm_, k++), BUFFER);
      gtk_list_store_set (mem_store_, &iter, MEM_CONTENTS_COL, BUFFER, -1);
      valid = gtk_tree_model_iter_next (GTK_TREE_MODEL (mem_store_), &iter);
    }
}

void
mixgtk_mixvm_update_cells_to_address (gint address)
{
  GtkTreePath *path = gtk_tree_path_new_from_indices (address, -1);

  if (address >= MIX_VM_CELL_NO) return;
  gtk_tree_view_scroll_to_cell (memory_, path, NULL, TRUE, 0, 0);
  gtk_tree_path_free (path);
}

/* update the loc pointer */
void
mixgtk_mixvm_update_loc (void)
{
  enum {SIZE = 10 };
  static gchar BUFFER[SIZE];
  mix_short_t locval;

  g_assert (loc_entry_);

  locval = mix_vm_get_prog_count (vm_);
  g_snprintf (BUFFER, SIZE, "%04d", mix_short_magnitude (locval));
  gtk_entry_set_text (loc_entry_, BUFFER);
}

/* update times */
void
mixgtk_mixvm_update_times (void)
{
  enum {SIZE = 20 };
  static gchar BUFFER[SIZE];
  gint uptime, progtime, laptime;

  g_assert (uptime_ && progtime_ && laptime_);
  mixgtk_cmd_dispatcher_get_times (&uptime, &progtime, &laptime);
  g_snprintf (BUFFER, SIZE, "%d", uptime);
  gtk_label_set_text (uptime_, BUFFER);
  g_snprintf (BUFFER, SIZE, "%d", progtime);
  gtk_label_set_text (progtime_, BUFFER);
  g_snprintf (BUFFER, SIZE, "%d", laptime);
  gtk_label_set_text (laptime_, BUFFER);
}


/* update all mixvm widgets */
void
mixgtk_mixvm_update_vm_widgets (void)
{
  mixgtk_mixvm_update_registers ();
  mixgtk_mixvm_update_loc ();
  mixgtk_mixvm_update_over_toggle ();
  mixgtk_mixvm_update_cmp ();
  mixgtk_mixvm_update_cells ();
  mixgtk_mixvm_update_times ();
}


/* update cmp flag */
void
on_cmp_e_toggled (GtkWidget *widget, gpointer data)
{
  if (GTK_TOGGLE_BUTTON (widget)->active)
    mix_vm_set_cmpflag (vm_, mix_EQ);
}

void
on_cmp_l_toggled (GtkWidget *widget, gpointer data)
{
  if (GTK_TOGGLE_BUTTON (widget)->active)
    mix_vm_set_cmpflag (vm_, mix_LESS);
}

void
on_cmp_g_toggled (GtkWidget *widget, gpointer data)
{
  if (GTK_TOGGLE_BUTTON (widget)->active)
    mix_vm_set_cmpflag (vm_, mix_GREAT);
}

void
on_overflow_toggled (GtkWidget *widget, gpointer data)
{
  mix_vm_set_overflow (vm_, GTK_TOGGLE_BUTTON (widget)->active);
}

void
on_loc_arrow_clicked (GtkWidget *widget, gpointer data)
{
  gint address = mix_short_magnitude (mix_vm_get_prog_count (vm_));
  mixgtk_mixvm_update_cells_to_address (address);
}

static void
register_AX_ (mix_word_t w, gpointer reg)
{
  gint k = GPOINTER_TO_INT (reg);
  if (k == 0)
    mix_vm_set_rA (vm_, w);
  else
    mix_vm_set_rX (vm_, w);
  update_register_ (MIXGTK_WIDGET_rA + k);
}

static void
register_j_ (mix_word_t w, gpointer data)
{
  if (mix_word_is_negative (w)) mix_word_reverse_sign (w);
  mix_vm_set_rJ (vm_, mix_word_to_short_fast (w));
  update_register_ (MIXGTK_WIDGET_rJ);
}

static void
register_i_ (mix_word_t w, gpointer reg)
{
  mix_vm_set_rI (vm_, GPOINTER_TO_INT (reg), mix_word_to_short_fast (w));
  update_register_ (MIXGTK_WIDGET_rI1 - 1 + GPOINTER_TO_INT (reg));
}

gboolean
on_register_click (GtkWidget *w, GdkEvent *e, gpointer data)
{
  enum {SIZE = 20};
  static gchar BUFFER[SIZE];
  gint k;

  if (w == GTK_WIDGET (reg_entries_[0]))
    mixgtk_input_word (_("Register A"), mix_vm_get_rA (vm_),
		       register_AX_, GINT_TO_POINTER (0));
  else if (w == GTK_WIDGET (reg_entries_[1]))
    mixgtk_input_word (_("Register X"), mix_vm_get_rX (vm_),
		       register_AX_, GINT_TO_POINTER(1));
  else if (w == GTK_WIDGET (reg_entries_[2]))
    mixgtk_input_short (_("Register J"), mix_vm_get_rJ (vm_),
			register_j_, NULL);
  else for (k = 1; k < 7; ++k)
    {
      if (w == GTK_WIDGET (reg_entries_[2 + k]))
	{
	  g_snprintf (BUFFER, SIZE, _("Register I%d"), k);
	  mixgtk_input_short (BUFFER, mix_vm_get_rI (vm_, k),
			      register_i_, GINT_TO_POINTER (k));
	  break;
	}
    }
  return TRUE;
}

static void
mem_cell_ (mix_word_t w, gpointer a)
{
  mix_short_t addr = mix_short_new (GPOINTER_TO_INT (a));
  mix_vm_set_addr_contents (vm_, addr, w);
  mixgtk_mixvm_update_cells ();
}

void
on_mix_font_activate (void)
{
  gint n;
  GtkWidget **w = text_widgets_ (&n);
  mixgtk_fontsel_query_font (MIX_FONT_MIXVM, w, n);
}



/* static functions */
static void
init_goto_ (void)
{
  goto_ = mixgtk_widget_factory_get_dialog (MIXGTK_GOTO_DIALOG);
  g_assert (goto_ != NULL);
  goto_entry_ =
    GTK_ENTRY (mixgtk_widget_factory_get (MIXGTK_GOTO_DIALOG,
                                          MIXGTK_WIDGET_GOTO_ENTRY));
  g_assert (goto_entry_ != NULL);
}

static void
update_register_ (mixgtk_widget_id_t reg)
{
  static gchar BUFFER[20];
  mix_word_t tipval = MIX_WORD_ZERO;
  switch (reg)
    {
    case MIXGTK_WIDGET_rA:
      tipval = mix_vm_get_rA (vm_);
      mix_word_print_to_buffer (tipval, BUFFER);
      break;
    case MIXGTK_WIDGET_rX:
      tipval = mix_vm_get_rX (vm_);
      mix_word_print_to_buffer (tipval, BUFFER);
      break;
    case MIXGTK_WIDGET_rJ:
      tipval = mix_short_to_word_fast (mix_vm_get_rJ (vm_));
      mix_short_print_to_buffer (mix_vm_get_rJ (vm_), BUFFER);
      break;
    case MIXGTK_WIDGET_rI1:
    case MIXGTK_WIDGET_rI2:
    case MIXGTK_WIDGET_rI3:
    case MIXGTK_WIDGET_rI4:
    case MIXGTK_WIDGET_rI5:
    case MIXGTK_WIDGET_rI6:
      {
	gint k = reg - MIXGTK_WIDGET_rI1 + 1;
	tipval = mix_short_to_word_fast (mix_vm_get_rI (vm_, k));
	mix_short_print_to_buffer (mix_vm_get_rI (vm_, k), BUFFER);
      }
      break;
    default:
      g_assert_not_reached ();
    }
  gtk_entry_set_text (reg_entries_[reg - MIXGTK_WIDGET_rA], BUFFER);
  g_snprintf (BUFFER, 20, "%s%ld", mix_word_is_negative (tipval)? "-" : "",
              mix_word_magnitude (tipval));

#if HAVE_OLD_GTK
  gtk_tooltips_set_tip (tips_,
  			GTK_WIDGET (reg_entries_[reg - MIXGTK_WIDGET_rA]),
                        BUFFER, NULL);
#else
  gtk_widget_set_tooltip_text (
    GTK_WIDGET (reg_entries_[reg - MIXGTK_WIDGET_rA]), BUFFER);
#endif
}


static void
cells_clicked_ (GtkTreeViewColumn* col, gpointer data)
{
  if (!goto_) init_goto_ ();
  if (gtk_dialog_run (GTK_DIALOG (goto_)) == GTK_RESPONSE_OK)
    {
      const gchar *txt = gtk_entry_get_text (goto_entry_);
      mix_short_t addr = mix_short_new (atoi (txt));
      if (addr < MIX_VM_CELL_NO)
        mixgtk_mixvm_update_cells_to_address (addr);
    }
  gtk_widget_hide (goto_);
}

static gboolean
cont_clicked_ (GtkWidget *w, GdkEvent *event, gpointer data)
{
  enum {SIZE = 30};
  static gchar BUFFER[SIZE];

  if (event && event->type == GDK_BUTTON_PRESS)
    {
      GtkTreeIter iter;
      GtkTreeModel *model;
      GtkTreeSelection *selection;

      selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (memory_));

      if (gtk_tree_selection_get_selected (selection, &model, &iter))
        {
          gint addr;

          gtk_tree_model_get (model, &iter, MEM_ADDRESS_COL, &addr, -1);
          g_snprintf (BUFFER, SIZE, _("Memory cell no. %d"), addr);
          mixgtk_input_word
            (BUFFER,
             mix_vm_get_addr_contents (vm_, mix_short_new (addr)),
             mem_cell_, GINT_TO_POINTER (addr));
        }
    }

  return FALSE;
}


static gboolean
init_mem_ (void)
{
  gint k;
  GtkTreeIter iter;
  GtkCellRenderer *renderer;
  GtkTreeViewColumn *col;

  /* model */

  mem_store_ = gtk_list_store_new (MEM_COL_NO, G_TYPE_INT, G_TYPE_STRING);

  g_assert (mem_store_);

  for (k = 0; k< MIX_VM_CELL_NO; ++k)
    {
      gtk_list_store_append (mem_store_, &iter);
      gtk_list_store_set (mem_store_, &iter,
                          MEM_ADDRESS_COL, k,
                          MEM_CONTENTS_COL, "+ 00 00 00 00 00",
                          -1);
    }

  /* view */

  renderer = gtk_cell_renderer_text_new ();
  memory_ =
    GTK_TREE_VIEW (mixgtk_widget_factory_get (MIXGTK_MIXVM_DIALOG,
                                              MIXGTK_WIDGET_CELLS));

  g_assert (memory_);
  g_assert (renderer);

  gtk_tree_view_set_model (memory_, GTK_TREE_MODEL (mem_store_));
  gtk_tree_view_set_headers_clickable (memory_, TRUE);
  g_object_unref (G_OBJECT (mem_store_));

  col = gtk_tree_view_column_new_with_attributes ("Address", renderer,
                                                  "text", 0, NULL);
  gtk_tree_view_column_set_clickable (col, TRUE);
  g_signal_connect (G_OBJECT (col), "clicked",
                    G_CALLBACK (cells_clicked_), NULL);

  gtk_tree_view_append_column (memory_, col);


  col = gtk_tree_view_column_new_with_attributes ("Contents", renderer,
                                                  "text", 1, NULL);
  gtk_tree_view_column_set_clickable (col, TRUE);
  g_signal_connect (G_OBJECT (col), "clicked",
                    G_CALLBACK (cont_clicked_), NULL);

  gtk_tree_view_append_column (memory_, col);

  g_signal_connect_after (G_OBJECT (memory_), "event-after",
                          G_CALLBACK (cont_clicked_), NULL);

  mixgtk_fontsel_set_font (MIX_FONT_MIXVM, GTK_WIDGET (memory_));

  return TRUE;
}

