/* -*-c-*- -------------- mixgtk_gen_handlers.c :
 * Implementation of the functions declared in mixgtk_gen_handlers.h
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

#include <mixlib/mix_vm_command.h>
#include "mixgtk_gen_handlers.h"
#include "mixgtk_cmd_dispatcher.h"
#include "mixgtk_config.h"

/* grab a file with an externally provided callback */
typedef void (*file_callback_t)(const gchar *file);

static GtkFileChooser *
get_chooser_ (const gchar *title, gboolean src)
{
  static GtkFileChooser *dialog = NULL;
  static GtkFileFilter *src_filter = NULL;
  static GtkFileFilter *code_filter = NULL;

  if (dialog == NULL)
    {
      dialog = GTK_FILE_CHOOSER
        (gtk_file_chooser_dialog_new ("gmixvm",
                                      NULL,
                                      GTK_FILE_CHOOSER_ACTION_OPEN,
                                      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                      GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
                                      NULL));
      src_filter = gtk_file_filter_new ();
      gtk_file_filter_add_pattern (src_filter, "*.mixal");
      gtk_file_filter_set_name (src_filter, "MIXAL files");
      code_filter = gtk_file_filter_new ();
      gtk_file_filter_add_pattern (code_filter, "*.mix");
      gtk_file_filter_set_name (code_filter, "MIX files");
      gtk_file_chooser_add_filter (dialog, src_filter);
      gtk_file_chooser_add_filter (dialog, code_filter);
    }

  gtk_file_chooser_set_filter (dialog, src? src_filter : code_filter);
  gtk_window_set_title (GTK_WINDOW (dialog), title);

  return dialog;
}

static void
get_file_ (file_callback_t callback,
           const gchar *title,
           gboolean is_src,
           const gchar *def_file)
{
  static gchar *last_folder = NULL;

  if (callback != NULL)
    {
      GtkFileChooser *dialog = get_chooser_ (title, is_src);

      if (def_file != NULL)
        {
          gtk_file_chooser_set_filename (dialog, def_file);
        }
      else if (last_folder != NULL)
        {
          gtk_file_chooser_set_current_folder (dialog, last_folder);
        }

      gint result = gtk_dialog_run (GTK_DIALOG (dialog));
      gtk_widget_hide (GTK_WIDGET (dialog));

      if (result == GTK_RESPONSE_ACCEPT)
        {
          gchar *filename = gtk_file_chooser_get_filename (dialog);
          if (filename)
            {
              callback (filename);
              if (last_folder) g_free (last_folder);
              last_folder = g_path_get_dirname (filename);
              g_free (filename);
            }
        }
    }
}

/* exec prompt command */
static void
exec_cmd_ (mix_vm_command_t cmd, const gchar *arg)
{
  gchar *command;
  if (arg)
    command = g_strdup_printf ("%s %s",
			       mix_vm_command_to_string (cmd),
			       arg);
  else
    command = g_strdup (mix_vm_command_to_string (cmd));
  mixgtk_cmd_dispatcher_dispatch (command);
  g_free (command);
}

/* load mix binary */
static void
open_cb_ (const gchar *file)
{
  exec_cmd_ (MIX_CMD_LOAD, file);
}

void
on_file_open_activate (GtkWidget *w, gpointer data)
{
  get_file_ (open_cb_, _("Load MIX program..."), FALSE, NULL);
}

/* edit mixal source */
static void
edit_cb_ (const gchar *file)
{
  exec_cmd_ (MIX_CMD_EDIT, file);
}

void
on_file_edit_activate (GtkWidget *w, gpointer data)
{
  get_file_ (edit_cb_,
             _("Edit MIXAL source file..."),
             TRUE,
             mixgtk_cmd_dispatcher_get_src_path ());
}

/* compile mixal source */
static void
compile_cb_ (const gchar *file)
{
  exec_cmd_ (MIX_CMD_COMPILE, file);
}

void
on_file_compile_activate (GtkWidget *w, gpointer data)
{
  get_file_ (compile_cb_,
             _("Compile MIXAL source file..."),
             TRUE,
             mixgtk_cmd_dispatcher_get_src_path ());
}

void
on_debug_run_activate (GtkWidget *w, gpointer p)
{
  mixgtk_cmd_dispatcher_dispatch (mix_vm_command_to_string (MIX_CMD_RUN));
}

void
on_debug_next_activate (GtkWidget *w, gpointer p)
{
  mixgtk_cmd_dispatcher_dispatch (mix_vm_command_to_string (MIX_CMD_NEXT));
}


void
on_file_exit_activate (GtkWidget *w, gpointer data)
{
  gtk_main_quit ();
}

void
on_clear_breakpoints_activate (GtkWidget *w, gpointer data)
{
  mixgtk_cmd_dispatcher_dispatch (mix_vm_command_to_string (MIX_CMD_CABP));
}

void
on_save_on_exit_toggle (GtkWidget *w, gpointer data)
{
  mixgtk_config_set_autosave (GTK_CHECK_MENU_ITEM (w)->active);
}

void
on_save_activate (GtkWidget *w, gpointer data)
{
  mixgtk_config_save ();
}


