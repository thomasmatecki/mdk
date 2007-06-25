/* -*-c-*- -------------- mixgtk_input.c :
 * Implementation of the functions declared in mixgtk_input.h
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

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "mixgtk_widgets.h"
#include "mixgtk_input.h"

enum {
  MSG_, SIGN_, B1_, B2_, B3_, B4_, B5_, DEC_, RESET_, OK_, SIZE_
};

static const gchar *WGT_NAMES_[SIZE_] = {
  "word_label",  "word_sign", "word_b1", "word_b2", "word_b3",
  "word_b4", "word_b5", "word_decimal",  "word_reset", "word_ok"
};

static GtkDialog *dialog_ = NULL;
static GtkWidget *childs_[SIZE_];
static input_callback_t callback_;
static gpointer data_;


static void init_dialog_ (void);
static void set_word_ (mix_word_t word);


/* init */
void
mixgtk_input_init (void)
{
  dialog_ = NULL;
}

/* get a word */
void
mixgtk_input_word (const gchar *message, mix_word_t def,
		   input_callback_t cb, gpointer data)
{
  if (!dialog_) init_dialog_ ();
  gtk_label_set_text (GTK_LABEL (childs_[MSG_]), message);
  set_word_ (def);
  callback_ = cb;
  data_ = data;
  gtk_widget_show (childs_[B1_]);
  gtk_widget_show (childs_[B2_]);
  gtk_widget_show (childs_[B3_]);
  gtk_widget_show (GTK_WIDGET (dialog_));
}

/* get a short */
void
mixgtk_input_short (const gchar *message, mix_short_t def,
		    input_callback_t cb, gpointer data)
{
  if (!dialog_) init_dialog_ ();
  gtk_label_set_text (GTK_LABEL (childs_[MSG_]), message);
  set_word_ (mix_word_to_short_fast (def));
  callback_ = cb;
  data_ = data;
  gtk_widget_show (GTK_WIDGET (dialog_));
  gtk_widget_hide (childs_[B1_]);
  gtk_widget_hide (childs_[B2_]);
  gtk_widget_hide (childs_[B3_]);
}

#include <gdk/gdkkeysyms.h>

/* dec and bytes input handler */
void
on_word_dec_changed (GtkEditable *dec, gpointer *data)
{
  gchar *txt = gtk_editable_get_chars (dec, 0, -1);

  if (strlen (txt) > 0)
    {
      gint val = atoi (txt);
      set_word_ (mix_word_new (val));
    }
  g_free (txt);
}

void
on_word_byte_changed (GtkEditable *byte, gpointer *data)
{
  gchar *txt = gtk_editable_get_chars (byte, 0, 2);

  if (strlen (txt) > 0)
    {
      mix_byte_t bytes[5];
      gint k;
      const gchar *s;
      mix_word_t w;

      for (k = 0; k < 5; ++k)
	{
	  const gchar *text = gtk_entry_get_text (GTK_ENTRY (childs_[B1_ + k]));
	  bytes[k] = mix_byte_new (atoi (text));
	}
      w = mix_bytes_to_word (bytes, 5);
      s = gtk_entry_get_text (GTK_ENTRY (childs_[SIGN_]));
      if (s && s[0] == '-') mix_word_reverse_sign (w);
      set_word_ (w);
    }
  g_free (txt);
}

void
on_word_sign_changed (GtkEditable *sign, gpointer *data)
{
  gchar *txt = gtk_editable_get_chars (sign, 0, 1);

  if (strlen (txt) > 0)
    {
      gint pos = 0;
      mix_word_t w =
        mix_word_new (atoi (gtk_entry_get_text (GTK_ENTRY (childs_[DEC_]))));

      g_signal_handlers_block_by_func (GTK_OBJECT (sign),
                                       on_word_sign_changed, data);

      if (txt[0] != '+' && txt[0] != '-') txt[0] = '+';
      gtk_editable_delete_text (sign, 0, 1);
      gtk_editable_insert_text (sign, txt, 1, &pos);

      g_signal_handlers_unblock_by_func (GTK_OBJECT (sign),
                                         on_word_sign_changed, data);
      set_word_ (txt[0] == '-' ? mix_word_negative (w) : w);
    }
  g_free (txt);
}

void
on_word_ok_clicked (GtkWidget *widget, gpointer *data)
{
  const gchar *text = gtk_entry_get_text (GTK_ENTRY (childs_[DEC_]));
  mix_word_t w = mix_word_new (atoi (text));
  if (w == MIX_WORD_ZERO)
    {
      const gchar *sign = gtk_entry_get_text (GTK_ENTRY (childs_[SIGN_]));
      if (sign && sign[0] == '-') mix_word_reverse_sign (w);
    }
  callback_ (w, data_);
  gtk_widget_hide (GTK_WIDGET (dialog_));
}

void
on_word_cancel_clicked (GtkWidget *widget, gpointer *data)
{
  gtk_widget_hide (GTK_WIDGET (dialog_));
}

void
on_word_reset_clicked (GtkWidget *w, gpointer *data)
{
  set_word_ (MIX_WORD_ZERO);
}

static void
init_dialog_ (void)
{
  gint k;

  dialog_ = GTK_DIALOG (mixgtk_widget_factory_get_dialog (MIXGTK_WORD_DIALOG));
  g_assert (dialog_ != NULL);
  for (k = 0; k < SIZE_; ++k)
    {
      childs_[k] = mixgtk_widget_factory_get_child_by_name
	(MIXGTK_WORD_DIALOG, WGT_NAMES_[k]);
      g_assert (childs_[k] != NULL);
    }
}

static void
set_word_ (mix_word_t word)
{
  enum {SIZE = 50};
  static gchar BUFFER[SIZE] = {0};
  gint k;
  gint val = mix_word_magnitude (word);
  gboolean neg = mix_word_is_negative (word);
  g_snprintf (BUFFER, SIZE, "%s%d", neg ? "-" : "", val);


  g_signal_handlers_block_by_func (GTK_OBJECT (childs_[DEC_]),
                                   on_word_dec_changed, NULL);
  g_signal_handlers_block_by_func (GTK_OBJECT (childs_[SIGN_]),
                                   on_word_sign_changed, NULL);
  for (k = B1_; k <= B5_; ++k)
    g_signal_handlers_block_by_func (GTK_OBJECT (childs_[k]),
                                     on_word_byte_changed, NULL);

  gtk_entry_set_text (GTK_ENTRY (childs_[DEC_]), BUFFER);
  gtk_entry_set_text (GTK_ENTRY (childs_[SIGN_]), neg ? "-" : "+");

  for (k = 1; k < 6; ++k)
    {
      mix_byte_t b = mix_word_get_byte (word, k);
      g_snprintf (BUFFER, SIZE, "%d", (int)b);
      gtk_entry_set_text (GTK_ENTRY (childs_[SIGN_ + k]), BUFFER);
    }

  for (k = B1_; k <= B5_; ++k)
    g_signal_handlers_unblock_by_func (GTK_OBJECT (childs_[k]),
                                       on_word_byte_changed, NULL);
  g_signal_handlers_unblock_by_func (GTK_OBJECT (childs_[DEC_]),
                                     on_word_dec_changed, NULL);
  g_signal_handlers_unblock_by_func (GTK_OBJECT (childs_[SIGN_]),
                                     on_word_sign_changed, NULL);
}

