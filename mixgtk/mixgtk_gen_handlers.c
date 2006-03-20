/* -*-c-*- -------------- mixgtk_gen_handlers.c :
 * Implementation of the functions declared in mixgtk_gen_handlers.h
 * ------------------------------------------------------------------
 * $Id: mixgtk_gen_handlers.c,v 1.10 2005/09/20 19:43:14 jao Exp $
 * ------------------------------------------------------------------
 * Copyright (C) 2001, 2002, 2004 Free Software Foundation, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
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
void
mixgtk_get_file (file_callback_t callback,
		 const gchar *title,
                 const gchar *pattern,
                 const gchar *def_file)
{
  if (callback != NULL)
    {
      GtkWidget *dialog;

      dialog =
        gtk_file_chooser_dialog_new (title,
                                     NULL,
                                     GTK_FILE_CHOOSER_ACTION_OPEN,
                                     GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                     GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
                                     NULL);

      if (pattern != NULL)
        {
          GtkFileFilter *filter = gtk_file_filter_new ();
          gtk_file_filter_add_pattern (filter, pattern);
          gtk_file_chooser_set_filter (GTK_FILE_CHOOSER (dialog), filter);
        }

      if (def_file != NULL)
        gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (dialog), def_file);

      if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
        {
          char *filename;

          filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
          callback (filename);
          g_free (filename);
        }

      gtk_widget_destroy (dialog);
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
  if (file)
      exec_cmd_ (MIX_CMD_LOAD, file);
}

void
on_file_open_activate (GtkWidget *w, gpointer data)
{
  mixgtk_get_file (open_cb_, _("Load MIX program..."), "*.mix", NULL);
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
  mixgtk_get_file (edit_cb_, _("Edit MIXAL source file..."), "*.mixal",
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
  mixgtk_get_file (compile_cb_, _("Compile MIXAL source file..."), "*.mixal",
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


