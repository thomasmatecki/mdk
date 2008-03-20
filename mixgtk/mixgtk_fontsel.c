/* -*-c-*- -------------- mixgtk_fontsel.c :
 * Implementation of the functions declared in mixgtk_fontsel.h
 * ------------------------------------------------------------------
 * Copyright (C) 2001, 2004, 2006, 2007, 2008 Free Software Foundation, Inc.
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

#include <mixlib/mix.h>
#include "mixgtk_widgets.h"
#include "mixgtk_config.h"
#include "mixgtk_mixvm.h"
#include "mixgtk_mixal.h"
#include "mixgtk_cmd_dispatcher.h"
#include "mixgtk_device.h"
#include "mixgtk_fontsel.h"

static GtkFontSelectionDialog *fontsel_dialog_ = NULL;

static const gchar *keys_[MIX_FONT_NO] = {
  "MIX.font", "Prompt.font", "Log.font", "MIXAL.font", "Devices.font",
  "Symbols.font", "Default.font"
};

static const gchar *default_font_ = "Monospace 10";

/* initialise the font selection dialog */
static void
init_fontsel_ (void)
{
  fontsel_dialog_ =
    GTK_FONT_SELECTION_DIALOG
    (mixgtk_widget_factory_get_dialog (MIXGTK_FONTSEL_DIALOG));
  g_assert (fontsel_dialog_ != NULL);
}

void
mixgtk_fontsel_set_font (mixgtk_font_t f, GtkWidget *w)
{
  const gchar *font;
  PangoFontDescription *font_desc;

  if (f >= MIX_FONT_NO || w == NULL) return;

  font = mixgtk_config_get (keys_[f]);
  if (!font)
    {
      mixgtk_config_update (keys_[f], default_font_);
      font = mixgtk_config_get (keys_[f]);
      g_assert (font);
    }

  font_desc = pango_font_description_from_string (font);

  if (font_desc)
    {
      gtk_widget_modify_font (w, font_desc);
      gtk_widget_queue_draw (w);
      pango_font_description_free (font_desc);
    }
  else
    mixgtk_config_remove (keys_[f]);

}

gboolean
mixgtk_fontsel_query_font (mixgtk_font_t f, GtkWidget **w, size_t no)
{
  gint result = GTK_RESPONSE_APPLY;
  const gchar *current;
  gboolean ret = FALSE;

  if (f >= MIX_FONT_NO) return FALSE;
  if (!fontsel_dialog_) init_fontsel_ ();

  current = mixgtk_config_get (keys_[f]);
  gtk_font_selection_dialog_set_font_name (fontsel_dialog_,
                                           current? current : default_font_);

  while (result == GTK_RESPONSE_APPLY)
    {
      result = gtk_dialog_run (GTK_DIALOG (fontsel_dialog_));
      if (result != GTK_RESPONSE_CANCEL)
        {
          gint k;
          mixgtk_config_update
            (keys_[f],
             gtk_font_selection_dialog_get_font_name (fontsel_dialog_));
          for (k = 0; k < no; ++k)
            mixgtk_fontsel_set_font (f, w[k]);
          ret = TRUE;
        }
    }
  gtk_widget_hide (GTK_WIDGET (fontsel_dialog_));
  return ret;
}

void
on_all_fonts_activate (void)
{
  gint result = GTK_RESPONSE_APPLY;
  const gchar *current;

  if (!fontsel_dialog_) init_fontsel_ ();

  current = mixgtk_config_get (keys_[MIX_FONT_DEFAULT]);
  gtk_font_selection_dialog_set_font_name (fontsel_dialog_,
                                           current? current : default_font_);

  while (result == GTK_RESPONSE_APPLY)
    {
      result = gtk_dialog_run (GTK_DIALOG (fontsel_dialog_));
      if (result != GTK_RESPONSE_CANCEL)
        {
          gint k;
          current = gtk_font_selection_dialog_get_font_name (fontsel_dialog_);

          for (k = 0; k < MIX_FONT_NO; ++k)
            mixgtk_config_update (keys_[k], current);

          mixgtk_mixal_update_fonts ();
          mixgtk_mixvm_update_fonts ();
          mixgtk_device_update_fonts ();
          mixgtk_cmd_dispatcher_update_fonts ();
        }
    }
  gtk_widget_hide (GTK_WIDGET (fontsel_dialog_));
}
