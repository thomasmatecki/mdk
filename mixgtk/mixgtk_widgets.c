/* -*-c-*- -------------- mixgtk_widgets.c :
 * Implementation of the functions declared in mixgtk_widgets.h
 * ------------------------------------------------------------------
 * Copyright (C) 2001, 2004, 2006, 2007, 2014 Free Software Foundation, Inc.
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


#include <unistd.h>
#include <glade/glade.h>
#include <mixlib/mix.h>
#include "mixgtk_config.h"
#include "mixgtk_widgets.h"

/* dialog names */
static const gchar * dnames_[] = {
  "main_window",
  "mixvm_window",
  "mixal_window",
  "devices_window",
  "word_dialog",
  "about_dialog",
  "goto_dialog",
  "fontsel_dialog",
  "devform_dialog",
  "external_dialog",
  "symbols_dialog",
  "input_dialog",
  "devdir_dialog"
};

#define DLG_NO_ (sizeof (dnames_) / sizeof(dnames_[0]))

/* widget names */
static const gchar *names_[] = {
  "main_statusbar",
  "attach_button",
  "detach_button",
  "main_notebook",
  "mixvm_widget",
  "mixvm_container",
  "command_prompt",
  "command_log",
  "dev_container",
  "mixal_container",
  "mixal_statusbar",
  "rA_entry",
  "rX_entry",
  "rJ_entry",
  "rI1_entry",
  "rI2_entry",
  "rI3_entry",
  "rI4_entry",
  "rI5_entry",
  "rI6_entry",
  "lesser_radio",
  "equal_radio",
  "greater_radio",
  "over_toggle",
  "memory_cells",
  "elapsed_label",
  "program_label",
  "uptime_label",
  "devices_container",
  "loc_entry",
  "goto_entry",
  NULL
};

#define WIDGET_NO_ (sizeof (names_) / sizeof (names_[0]))

/* the glade specs */
static GladeXML *xml_[DLG_NO_] = {NULL};

/* the xml file name */
static const gchar *file_ = NULL;

static void init_xml_ (mixgtk_dialog_id_t dlg)
{
  xml_[dlg] = glade_xml_new (file_, dnames_[dlg], NULL);
  glade_xml_signal_autoconnect (xml_[dlg]);
  g_assert (xml_[dlg] != NULL);
}

/* create a new factory from an xml glade file */
gboolean
mixgtk_widget_factory_init (void)
{
  if (!file_)
    {
      const gchar *glade_file = GLADE_FILE;
      glade_init ();
      if (access (glade_file, R_OK)) {
	if (access (LOCAL_GLADE_FILE, R_OK)) return FALSE;
	glade_file = LOCAL_GLADE_FILE;
      }
      file_ = g_strdup (glade_file);
    }
  else
    {
      gint k;
      for (k = 0; k < DLG_NO_; ++k) if (xml_[k]) xml_[k] = NULL;
    }

  init_xml_ (MIXGTK_MAIN);
  return TRUE;
}

/* get a widget */
GtkWidget *
mixgtk_widget_factory_get (mixgtk_dialog_id_t dlg, mixgtk_widget_id_t widget)
{
  g_assert ((long)widget < WIDGET_NO_);

  return mixgtk_widget_factory_get_child_by_name (dlg, names_[widget]);
}


GtkWidget *
mixgtk_widget_factory_get_dialog (mixgtk_dialog_id_t dlg)
{
  g_assert (dlg < DLG_NO_);

  if (!xml_[dlg]) init_xml_ (dlg);
  return glade_xml_get_widget (xml_[dlg], dnames_[dlg]);
}

GtkWidget *
mixgtk_widget_factory_get_child_by_name (mixgtk_dialog_id_t dlg,
					 const gchar *name)
{
  g_assert (dlg < DLG_NO_);
  g_assert (name != NULL);
  if (!xml_[dlg]) init_xml_ (dlg);
  return glade_xml_get_widget (xml_[dlg], name);
}
