/* -*-c-*- -------------- mixgtk_cmd_dispatcher.c :
 * Implementation of the functions declared in mixgtk_external.h
 * ------------------------------------------------------------------
 * Copyright (C) 2006, 2007, 2008, 2009 Free Software Foundation, Inc.
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 *
 */

#include <stdlib.h>

#include <mixlib/mix.h>

#include "mixgtk_config.h"
#include "mixgtk_cmd_dispatcher.h"
#include "mixgtk_external.h"

enum {
  ext_wdg_ed_chooser,
  ext_wdg_ed_entry,
  ext_wdg_asm_chooser,
  ext_wdg_asm_ls,
  ext_wdg_no
};

static GtkWidget *ext_dlg_ = NULL;
static GtkWidget *ext_wdg_[ext_wdg_no] = {NULL, NULL, NULL, NULL};
static const gchar *ext_wdg_names_[] = {
  "editor_chooser", "editor_entry", "mixasm_chooser", "mixasm_mls"
};

static const gchar *EDITOR_KEY_ = "Editor";
static const gchar *MIXASM_KEY_ = "Mixasm";

static mix_vm_cmd_dispatcher_t *dispatcher_;

static void init_widgets_ (void);
static void update_dialog_ (void);
static void read_config_ (void);
static void update_config_ (void);
static void update_editor_ (const gchar *cmd);
static void update_asm_ (const gchar *cmd);


void
mixgtk_external_init (mix_vm_cmd_dispatcher_t *dispatcher)
{
  g_assert (dispatcher);
  dispatcher_ = dispatcher;
  init_widgets_ ();
  read_config_ ();
}

void
on_external_programs_activate ()
{
  if (ext_dlg_)
    {
      update_dialog_ ();

      gtk_widget_show (ext_dlg_);

      if (gtk_dialog_run (GTK_DIALOG (ext_dlg_)) == GTK_RESPONSE_OK)
        {
          update_config_ ();
        }
      gtk_widget_hide (ext_dlg_);
    }
}


void
init_widgets_ (void)
{
  gint k;
  GtkFileFilter *filter = gtk_file_filter_new ();

  g_assert (filter);

  ext_dlg_ = mixgtk_widget_factory_get_dialog (MIXGTK_EXTERNPROG_DIALOG);
  g_assert (ext_dlg_ != NULL);

  for (k = 0; k < ext_wdg_no; ++k)
    {
      ext_wdg_[k] = mixgtk_widget_factory_get_child_by_name
        (MIXGTK_EXTERNPROG_DIALOG, ext_wdg_names_[k]);
      g_assert (ext_wdg_[k]);
    }

  gtk_file_filter_add_mime_type (filter, "application/x-executable");
#ifdef EXTRA_EXEC_MIME_TYPE
  gtk_file_filter_add_mime_type (filter, "application/octet-stream");
#endif
  gtk_file_chooser_set_filter
    (GTK_FILE_CHOOSER (ext_wdg_[ext_wdg_asm_chooser]), filter);
  gtk_file_chooser_set_filter
    (GTK_FILE_CHOOSER (ext_wdg_[ext_wdg_ed_chooser]), filter);

#if GTK_CHECK_VERSION (2,18,0)
  gtk_file_chooser_set_create_folders
    (GTK_FILE_CHOOSER (ext_wdg_[ext_wdg_asm_chooser]), FALSE);
  gtk_file_chooser_set_create_folders
    (GTK_FILE_CHOOSER (ext_wdg_[ext_wdg_ed_chooser]), FALSE);
#endif // GTK_CHECK_VERSION
}

void
update_editor_ (const gchar *cmd)
{
  mix_vm_cmd_dispatcher_set_editor (dispatcher_, cmd);
  mixgtk_config_update (EDITOR_KEY_, cmd);
}

void
update_asm_ (const gchar *cmd)
{
  mix_vm_cmd_dispatcher_set_assembler (dispatcher_, cmd);
  mixgtk_config_update (MIXASM_KEY_, cmd);
}

void
read_config_ (void)
{
  const gchar *editor = mixgtk_config_get (EDITOR_KEY_);
  const gchar *assem = mixgtk_config_get (MIXASM_KEY_);

  if (!editor)
    {
      static const gchar *ENV[] = {"MDK_EDITOR", "X_EDITOR", NULL};
      gchar *edit = NULL;

      int k = 0;
      while (!edit && ENV[k]) edit = getenv (ENV[k++]);

      if (edit) edit = g_strconcat (edit, " %s", NULL);
      else edit = g_strdup (DEFAULT_EDITOR_CMD);

      update_editor_ (edit);

      g_free (edit);
    }
  else
    {
      update_editor_ (editor);
    }

  update_asm_ (assem? assem : DEFAULT_ASM_CMD);
}

void
update_dialog_ (void)
{
  const gchar *editor = mixgtk_config_get (EDITOR_KEY_);
  const gchar *assem = mixgtk_config_get (MIXASM_KEY_);

  if (editor && assem && ext_dlg_)
    {
      gchar **parts = g_strsplit_set (editor, " \t", 2);
      gboolean is_active;

      gtk_file_chooser_select_filename
        (GTK_FILE_CHOOSER (ext_wdg_[ext_wdg_ed_chooser]), parts[0]);
      gtk_entry_set_text
        (GTK_ENTRY (ext_wdg_[ext_wdg_ed_entry]), parts[1]);

      g_strfreev (parts);

      parts = g_strsplit_set (assem, " \t", 3);
      gtk_file_chooser_select_filename
        (GTK_FILE_CHOOSER (ext_wdg_[ext_wdg_asm_chooser]), parts[0]);

      is_active =
        parts[1] && g_ascii_strcasecmp (g_strstrip (parts[1]), "-l") == 0;

      gtk_toggle_button_set_active
        (GTK_TOGGLE_BUTTON (ext_wdg_[ext_wdg_asm_ls]), is_active);

      g_strfreev (parts);
    }
}

void
update_config_ (void)
{
  gchar *prog = gtk_file_chooser_get_filename
    (GTK_FILE_CHOOSER (ext_wdg_[ext_wdg_ed_chooser]));
  const gchar *args = gtk_entry_get_text (GTK_ENTRY (ext_wdg_[ext_wdg_ed_entry]));
  gchar *cmd = g_strdup_printf ("%s %s", prog, args);

  update_editor_ (cmd);

  g_free (cmd);
  g_free (prog);

  prog = gtk_file_chooser_get_filename
    (GTK_FILE_CHOOSER (ext_wdg_[ext_wdg_asm_chooser]));
  args =
    gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (ext_wdg_[ext_wdg_asm_ls]))
    ? " -l "
    : " ";
  cmd = g_strdup_printf ("%s%s%%s", prog, args);

  update_asm_ (cmd);

  g_free (cmd);
  g_free (prog);
}
