/* -*-c-*- -------------- mixgtk_cmd_dispatcher.c :
 * Implementation of the functions declared in mixgtk_cmd_dispatcher.h
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
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include <mixlib/mix.h>

#ifdef HAVE_LIBHISTORY
#  include <readline/history.h>
#endif

#ifdef MAKE_GUILE
#  include <mixguile/mixguile.h>
#endif

#include <gdk/gdkkeysyms.h>
#include <mixlib/mix_vm_command.h>
#include "mixgtk_mixvm.h"
#include "mixgtk_mixal.h"
#include "mixgtk_fontsel.h"
#include "mixgtk_config.h"
#include "mixgtk_external.h"
#include "mixgtk_cmd_dispatcher.h"


/* a mix vm command dispatcher */
typedef struct mixgtk_dispatch_
{
  mix_vm_cmd_dispatcher_t *dispatcher; /* the underlying cmd dispatcher */
  FILE *out;			/* the dispatcher's output file */
  int fildes[2];		/* pipe for communication with the dispatcher */
  GtkWidget *prompt;		/* the command prompt widget */
  GtkWidget *log;		/* the dispatcher's messages echo area */
  GtkWidget *status;		/* the status bar widget */
  guint context;		/* context of the status bar messages */
  gchar *last_file;
} mixgtk_dispatch_data_t;

static struct mixgtk_dispatch_ dis_data_ = {NULL};

static const gchar *TITLE_FORMAT_ = "gmixvm - %s";

static void
log_command_ (mixgtk_dispatch_data_t *dis, const gchar *cmd)
{
  GtkTextBuffer *buf = gtk_text_view_get_buffer (GTK_TEXT_VIEW (dis->log));
  GtkTextIter end;

  gtk_text_buffer_get_end_iter (buf, &end);
  gtk_text_buffer_place_cursor (buf, &end);
  gtk_text_buffer_insert_at_cursor (buf, "MIX> ", -1);
  gtk_text_buffer_insert_at_cursor (buf, cmd, -1);
  gtk_text_buffer_insert_at_cursor (buf, "\n", -1);

#ifdef HAVE_LIBHISTORY
  add_history ((char *)cmd);
/*  history_search ((char *)cmd, 0); */
  history_set_pos (history_base + history_length - 1);
#endif
}

static void
flush_log_ (mixgtk_dispatch_data_t *dis)
{
  enum {BLKSIZE = 100};
  static gchar BUFFER[BLKSIZE];

  GtkTextBuffer *buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (dis->log));
  GtkTextMark *mark;
  GtkTextIter end;
  ssize_t k;

  gtk_text_buffer_get_end_iter (buffer, &end);
  gtk_text_buffer_place_cursor (buffer, &end);

  fflush (dis->out);
  while ((k = read (dis->fildes[0], BUFFER, BLKSIZE)) != 0)
    {
      if (k == -1 && errno != EINTR) break;
      if (k != -1)
	gtk_text_buffer_insert_at_cursor (buffer, BUFFER, k);
    }

  mark = gtk_text_buffer_get_insert (buffer);
  gtk_text_view_scroll_to_mark (GTK_TEXT_VIEW (dis->log), mark, 0,
                                TRUE, 0, 0);
}

/* hooks */
static void
global_post_hook_ (mix_vm_cmd_dispatcher_t *dis,
		   mix_vm_command_t cmd, const gchar *arg, gpointer data)
{
  flush_log_ ((mixgtk_dispatch_data_t *)data);
  mixgtk_mixvm_update_vm_widgets ();
}

static void
load_post_hook_ (mix_vm_cmd_dispatcher_t *dis, const gchar *arg,
		 gpointer data)
{
  static glong id = -1;

  if (mix_vm_cmd_dispatcher_get_last_result (dis))
    {
      GtkWindow *mainw =
	GTK_WINDOW (mixgtk_widget_factory_get_dialog (MIXGTK_MAIN));

      if (dis_data_.last_file) g_free (dis_data_.last_file);
      dis_data_.last_file = g_strdup_printf (TITLE_FORMAT_, arg);
      gtk_window_set_title (mainw, dis_data_.last_file);

      mixgtk_mixal_load_file ();
      mixgtk_mixal_update ();
      mixgtk_mixal_update_bp_all ();

      if (id != -1)
	gtk_statusbar_remove (GTK_STATUSBAR (dis_data_.status),
			      dis_data_.context, (guint)id);
      id =
	gtk_statusbar_push (GTK_STATUSBAR (dis_data_.status), dis_data_.context,
			    dis_data_.last_file);
    }
}

static void
run_post_hook_ (mix_vm_cmd_dispatcher_t *dis, const gchar *arg,
		 gpointer data)
{
  mixgtk_mixal_update ();
}

static void
next_post_hook_ (mix_vm_cmd_dispatcher_t *dis, const gchar *arg,
		 gpointer data)
{
  mixgtk_mixal_update ();
}

static void
linebp_post_hook_ (mix_vm_cmd_dispatcher_t *dis, const gchar *arg,
		gpointer data)
{
  if (arg && strlen (arg)) mixgtk_mixal_update_bp_at_line (atoi (arg));
}

static void
addrbp_post_hook_ (mix_vm_cmd_dispatcher_t *dis, const gchar *arg,
		gpointer data)
{
  if (arg && strlen (arg)) mixgtk_mixal_update_bp_at_address (atoi (arg));
}

static void
allbp_post_hook_ (mix_vm_cmd_dispatcher_t *dis, const gchar *arg,
		  gpointer data)
{
  mixgtk_mixal_update_bp_all ();
}


static void
install_hooks_ (void)
{
  mix_vm_cmd_dispatcher_global_post_hook (dis_data_.dispatcher,
					  global_post_hook_,
					  (gpointer)(&dis_data_));
  mix_vm_cmd_dispatcher_post_hook (dis_data_.dispatcher,
				   MIX_CMD_LOAD, load_post_hook_,
				   NULL);
  mix_vm_cmd_dispatcher_post_hook (dis_data_.dispatcher,
				   MIX_CMD_RUN, run_post_hook_,
				   NULL);
  mix_vm_cmd_dispatcher_post_hook (dis_data_.dispatcher,
				   MIX_CMD_NEXT, next_post_hook_,
				   NULL);
  mix_vm_cmd_dispatcher_post_hook (dis_data_.dispatcher,
				   MIX_CMD_SBP, linebp_post_hook_,
				   NULL);
  mix_vm_cmd_dispatcher_post_hook (dis_data_.dispatcher,
				   MIX_CMD_CBP, linebp_post_hook_,
				   NULL);
  mix_vm_cmd_dispatcher_post_hook (dis_data_.dispatcher,
				   MIX_CMD_SBPA, addrbp_post_hook_,
				   NULL);
  mix_vm_cmd_dispatcher_post_hook (dis_data_.dispatcher,
				   MIX_CMD_CBPA, addrbp_post_hook_,
				   NULL);
  mix_vm_cmd_dispatcher_post_hook (dis_data_.dispatcher,
				   MIX_CMD_CABP, allbp_post_hook_,
				   NULL);
}


/* initialise the command dispatcher */
gboolean
mixgtk_cmd_dispatcher_init (mixgtk_dialog_id_t top)
{
  static gboolean restart = FALSE;
  gchar *text = NULL;

  dis_data_.prompt =
    mixgtk_widget_factory_get (top, MIXGTK_WIDGET_PROMPT);
  g_return_val_if_fail (dis_data_.prompt != NULL, FALSE);

  if (dis_data_.log)
    text = gtk_editable_get_chars (GTK_EDITABLE (dis_data_.log), 0, -1);

  dis_data_.log =
    mixgtk_widget_factory_get (top, MIXGTK_WIDGET_LOG);

  g_return_val_if_fail (dis_data_.log != NULL, FALSE);

  if (text)
    {
      gtk_text_buffer_insert_at_cursor
        (gtk_text_view_get_buffer (GTK_TEXT_VIEW (dis_data_.log)), text, -1);
      g_free (text);
    }

  if (!dis_data_.dispatcher)
    {
      static const gchar *HISTORY_FILE = "gmixvm.history";
      static gint HISTORY_SIZE = 100;
      mix_config_t *config = mixgtk_config_get_mix_config ();

      int r = pipe (dis_data_.fildes);
      g_return_val_if_fail (r == 0, FALSE);
      /* connect stdout/stderr to the pipe's write end */
      if (dup2 (dis_data_.fildes[1], STDOUT_FILENO) == -1
	  || dup2 (dis_data_.fildes[1], STDOUT_FILENO) == -1)
	return FALSE;
      dis_data_.out = fdopen (dis_data_.fildes[1], "w");
      g_return_val_if_fail (dis_data_.out != NULL, FALSE);
      r = fcntl (dis_data_.fildes[0], F_GETFL, 0);
      g_return_val_if_fail (r != -1, FALSE);
      r |= O_NONBLOCK;
      r = fcntl(dis_data_.fildes[0], F_SETFL, r);
      g_return_val_if_fail (r != -1, FALSE);

      if (!mix_config_get_history_file (config))
	mix_config_set_history_file (config, HISTORY_FILE);
      if (mix_config_get_history_size (config) == 0)
	mix_config_set_history_size (config, HISTORY_SIZE);

      dis_data_.dispatcher =
	mix_vm_cmd_dispatcher_new_with_config (dis_data_.out,
					       dis_data_.out,
					       config);
      mix_vm_cmd_dispatcher_print_time (dis_data_.dispatcher, FALSE);

      install_hooks_ ();
    }

  dis_data_.status =
    mixgtk_widget_factory_get (MIXGTK_MAIN, MIXGTK_WIDGET_STATUSBAR);
  g_return_val_if_fail (dis_data_.status != NULL, FALSE);
  dis_data_.context = gtk_statusbar_get_context_id (GTK_STATUSBAR
						    (dis_data_.status),
						    "cmd_dis_context");

  if (!restart) mixgtk_external_init (dis_data_.dispatcher);

  if (dis_data_.last_file)
    gtk_window_set_title
      (GTK_WINDOW (mixgtk_widget_factory_get_dialog (MIXGTK_MAIN)),
       dis_data_.last_file);

  mixgtk_fontsel_set_font (MIX_FONT_LOG, dis_data_.log);
  mixgtk_fontsel_set_font (MIX_FONT_PROMPT, dis_data_.prompt);

  restart = TRUE;
  return TRUE;
}

/* dispatch an externally provided command */
#ifdef MAKE_GUILE
static gboolean
try_guile_ (const gchar *command)
{
  if (command && command[0] == '(' && command[strlen (command) - 1] == ')')
    {
      mixguile_interpret_command (command);
      return TRUE;
    }
  return FALSE;
}
#else
#  define try_guile_(ignored)  FALSE
#endif

void
mixgtk_cmd_dispatcher_dispatch (const gchar *command)
{
  GtkWidget *entry = dis_data_.prompt;
  g_return_if_fail (command != NULL);
  g_assert (entry != NULL);
  gtk_entry_set_text (GTK_ENTRY (entry), command);
  log_command_ (&dis_data_, command);
  if (!try_guile_ (command))
    mix_vm_cmd_dispatcher_dispatch_text (dis_data_.dispatcher, command);
  gtk_entry_set_text (GTK_ENTRY (entry), "");
}

/* get times */
void
mixgtk_cmd_dispatcher_get_times (gint *uptime, gint *progtime, gint *laptime)
{
  if (uptime != NULL)
    *uptime = mix_vm_cmd_dispatcher_get_uptime (dis_data_.dispatcher);
  if (progtime != NULL)
    *progtime = mix_vm_cmd_dispatcher_get_progtime (dis_data_.dispatcher);
  if (laptime != NULL)
    *laptime = mix_vm_cmd_dispatcher_get_laptime (dis_data_.dispatcher);
}

/* get the underlying vm */
mix_vm_t *
mixgtk_cmd_dispatcher_get_vm (void)
{
  return (mix_vm_t *) mix_vm_cmd_dispatcher_get_vm (dis_data_.dispatcher);
}

/* get the current source file */
const gchar *
mixgtk_cmd_dispatcher_get_src_path (void)
{
  return mix_vm_cmd_dispatcher_get_src_file_path (dis_data_.dispatcher);
}

/* get the mix cmd dispatcher */
mix_vm_cmd_dispatcher_t *
mixgtk_cmd_dispatcher_get_mix_dispatcher (void)
{
  return dis_data_.dispatcher;
}

/* process commands */
void
complete_command_ (void)
{
  GtkEntry *entry = GTK_ENTRY (dis_data_.prompt);
  gchar *prefix = NULL;
  const gchar *text = gtk_entry_get_text (entry);
  const GList  *cmds =
    mix_vm_cmd_dispatcher_complete (dis_data_.dispatcher, text, &prefix);

  if (prefix != NULL)
    {
      GtkTextBuffer *buf =
        gtk_text_view_get_buffer (GTK_TEXT_VIEW (dis_data_.log));

      gtk_entry_set_text (entry, prefix);
      if (g_list_length ((GList *)cmds) > 1)
	{
          static gchar BUFFER[25];
	  gint k = 0;
	  gtk_text_buffer_insert_at_cursor (buf, "Completions:\n", -1);
	  while (cmds)
	    {
	      g_snprintf (BUFFER, 25, "%-12s", (const char*)cmds->data);
	      ++k;
	      gtk_text_buffer_insert_at_cursor (buf, BUFFER, -1);
	      if (k%5 == 0)
		gtk_text_buffer_insert_at_cursor (buf, "\n", -1);

	      cmds = cmds->next;
	    }
	  if (k%5 != 0)
	    gtk_text_buffer_insert_at_cursor (buf, "\n", -1);
	}
      else
        {
          gint pos = strlen (prefix);
          gtk_editable_insert_text (GTK_EDITABLE (entry), " ", 1, &pos);
        }
      flush_log_ (&dis_data_);
      g_free (prefix);
    }
}

int
on_command_prompt_key_press_event (GtkEntry *w, GdkEventKey *e, gpointer d)
{
  guint key = e->keyval;
  gboolean result = FALSE;

#ifdef HAVE_LIBHISTORY
  HIST_ENTRY *entry = NULL;
  if (key == GDK_Up)
    {
      entry = previous_history ();
      if (entry && entry->line)
	gtk_entry_set_text (w, entry->line);
      result = TRUE;
    }
  if (key == GDK_Down)
    {
      entry = next_history ();
      if (entry && entry->line)
	gtk_entry_set_text (w, entry->line);
      result = TRUE;
    }
#endif

  if (key == GDK_Tab)
    {
      complete_command_ ();
      result = TRUE;
    }

  if (result) gtk_editable_set_position (GTK_EDITABLE (w), -1);

  return result;
}

void
on_command_prompt_activate (GtkEntry *prompt, gpointer data)
{
  gchar *text =
    g_strstrip (gtk_editable_get_chars (GTK_EDITABLE (prompt), 0, -1));
  if (text && *text)
    {
      log_command_ (&dis_data_, text);
      if (!try_guile_ (text))
        mix_vm_cmd_dispatcher_dispatch_text (dis_data_.dispatcher, text);
      gtk_entry_set_text (prompt, "");
    }
  g_free (text);
}

void
on_log_font_activate ()
{
  GtkWidget *w[] = { dis_data_.log };
  mixgtk_fontsel_query_font (MIX_FONT_LOG, w, 1);
}

void
on_prompt_font_activate ()
{
  GtkWidget *w[] = { dis_data_.prompt };
  mixgtk_fontsel_query_font (MIX_FONT_PROMPT, w, 1);
}

void
mixgtk_cmd_dispatcher_update_fonts (void)
{
  mixgtk_fontsel_set_font (MIX_FONT_LOG, dis_data_.log);
  mixgtk_fontsel_set_font (MIX_FONT_PROMPT, dis_data_.prompt);
}
