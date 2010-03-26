/* -*-c-*- -------------- mixvm_loop.c :
 * Implementation of mix vm command loop.
 * ------------------------------------------------------------------
 * Copyright (C) 2000, 2001, 2002, 2004, 2006, 2007, 2009, 2010 Free
 * Software Foundation, Inc.
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


#include <stdio.h>
#include <string.h>

#include <mixlib/mix.h>
#include <mixlib/mix_config.h>
#include <mixlib/mix_vm.h>
#include <mixlib/mix_device.h>
#include <mixlib/mix_vm_dump.h>

#ifdef MAKE_GUILE
#include <mixguile/mixguile.h>
#endif

#include "mixvm_command.h"
#include "mixvm_loop.h"

#ifdef HAVE_LIBHISTORY
#  include <readline/history.h>
#else
#  define add_history(x) ((void)0)
#endif

#ifdef HAVE_LIBREADLINE
#  include <readline/readline.h>
#else /* !HAVE_LIBREADLINE */
static char *
readline (char *prompt)
{
  enum {LINE_LEN = 256};
  char *line = g_new (char, LINE_LEN);
  printf ("%s", prompt);
  return fgets (line, LINE_LEN, stdin);
}
#endif /* HAVE_LIBREADLINE */

/* A static variable for holding the line. */
static char *line_read = (char *)NULL;

#define PROMPT_LEN 128
static char PROMPT[PROMPT_LEN + 1] = {'M', 'I', 'X', '>', ' '};
static const char *CONFIG_FILE_ = "mixvm.config";
static const char *PROMPT_KEY_ = "Prompt";

/* Read a string, and return a pointer to it.  Returns NULL on EOF. */
static char *
rl_gets ()
{
  /* If the buffer has already been allocated, return the memory
     to the free pool. */
  if (line_read)
    {
      g_free (line_read);
      line_read = (char *)NULL;
    }

  /* Get a line from the user. */
  line_read = readline ((char *)PROMPT);

  /* If the line has any text in it, save it on the history. */
  if (line_read && *line_read)
    add_history (line_read);

  return (line_read);
}


/* The main command loop of the virtual machine  */
static mix_config_t *config_ = NULL;

static mix_vm_cmd_dispatcher_t *
init_mixvm_ (const gchar *file, gboolean use_emacs)
{
  static const gchar *HISTORY_FILE = "mixvm.history";
  static gint HISTORY_SIZE = 100;
  config_ = mix_config_new (NULL, CONFIG_FILE_);

  mix_config_set_autosave (config_, TRUE);
  if (!mix_config_get_history_file (config_))
    mix_config_set_history_file (config_, HISTORY_FILE);
  if (mix_config_get_history_size (config_) == 0)
    mix_config_set_history_size (config_, HISTORY_SIZE);

  mix_vmloop_set_prompt (mix_config_get (config_, PROMPT_KEY_));

  return mixvm_cmd_init (config_, (char *)file, use_emacs);
}

void
mix_vmloop_set_prompt (const gchar *prompt)
{
  if (prompt)
    {
      g_snprintf (PROMPT, PROMPT_LEN, "%s ", prompt);
      mix_config_update (config_, PROMPT_KEY_, prompt);
    }
}

static void
loop_ (void *closure, int argc, char *argv[])
{
  while ( mixvm_cmd_exec (rl_gets ()) )
    ;
  mix_config_delete (config_);
}

void
mix_vmloop (int argc, char *argv[], gboolean initfile,
	    const gchar *file, gboolean use_emacs)
{
#ifdef MAKE_GUILE
  mix_vm_cmd_dispatcher_t *dis =  init_mixvm_ (file, use_emacs);
  mixguile_init (argc, argv, initfile, loop_, dis);
#else
  (void) init_mixvm_ (file, use_emacs);
  loop_ (NULL, argc, argv);
#endif
}

/* run a program and exit */
void
mix_vmrun (const gchar *code_file, gboolean dump, gboolean ptime)
{
  gchar *time_cmd = ptime? g_strdup ("stime on") : g_strdup ("stime off");
  gchar *run_cmd = g_strdup ("run");
  gchar *dump_cmd = dump? g_strdup ("pall") : NULL;
  gboolean result;

  init_mixvm_ (code_file, FALSE);
  result = mixvm_cmd_exec (time_cmd) && mixvm_cmd_exec (run_cmd);
  if (result && dump) mixvm_cmd_exec (dump_cmd);
  mix_config_set_autosave (config_, FALSE);
  mix_config_delete (config_);
  g_free(time_cmd);
  g_free(run_cmd);
  if (dump_cmd) g_free(dump_cmd);
}
