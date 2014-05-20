/* -*-c-*- -------------- mixvm_command.c :
 * Implementation of the functions declared in mixvm_command.h
 * ------------------------------------------------------------------
 * Copyright (C) 2000, 2001, 2002, 2004, 2006, 2007, 2014 Free Software Foundation, Inc.
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

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include <mixlib/mix.h>

#ifdef HAVE_LIBREADLINE
#  include <readline/readline.h>
#  include <readline/history.h>
#  ifndef HAVE_RL_COMPLETION_MATCHES /* old versions of rl don't use rl_ */
#    define rl_completion_matches completion_matches
#  endif
#endif /* HAVE_LIBREADLINE */

#include <mixlib/mix_vm.h>
#include <mixlib/mix_vm_dump.h>
#include <mixlib/mix_eval.h>
#include <mixlib/mix_src_file.h>
#include <mixlib/mix_vm_command.h>

#ifdef MAKE_GUILE
#  include <mixguile/mixguile.h>
static gboolean
try_guile_ (char *line)
{
  if (line[0] == '(')
    {
      if (line[strlen (line) -1] != ')') return FALSE;
      mixguile_interpret_command (line);
      return TRUE;
    }
  return FALSE;
}
#else  /* !MAKE_GUILE */
#  define try_guile_(ignored) FALSE
#endif /* MAKE_GUILE */

#include "mixvm_loop.h"
#include "mixvm_command.h"

/* mixvm dispatcher */
static mix_vm_cmd_dispatcher_t *dis_ = NULL;
static mix_config_t *config_ = NULL;

/* The names of functions that actually do the manipulation. */
#define DEC_FUN(name) \
static gboolean cmd_##name (mix_vm_cmd_dispatcher_t *dis, const char *arg)

DEC_FUN (shell_);
DEC_FUN (quit_);
DEC_FUN (prompt_);

mix_vm_command_info_t commands[] = {
  { "prompt", cmd_prompt_, N_("Set command prompt"), "prompt PROMPT" },
  { "shell", cmd_shell_, N_("Execute shell command"), "shell COMMAND" },
  { "quit", cmd_quit_, N_("Quit the program"), "quit" },
  { (char *)NULL, NULL, (char *)NULL }
};


#ifdef HAVE_LIBREADLINE
/* readline functions */
static char *
mixvm_cmd_generator_ (const char *text, int state);


/* Attempt to complete on the contents of TEXT.  START and END bound the
   region of rl_line_buffer that contains the word to complete.  TEXT is
   the word to complete.  We can use the entire contents of rl_line_buffer
   in case we want to do some simple parsing.  Return the array of matches,
   or NULL if there aren't any. */
static char **
mixvm_cmd_completion_ (char *text, int start, int end)
{
  char **matches;

  matches = (char **)NULL;

  /* If this word is at the start of the line, then it is a command
     to complete.  Otherwise it is the name of a file in the current
     directory. */
  if (start == 0)
    matches = rl_completion_matches (text, mixvm_cmd_generator_);

  return (matches);
}

/* Generator function for command completion.  STATE lets us know whether
   to start from scratch; without any state (i.e. STATE == 0), then we
   start at the top of the list. */
static char *
mixvm_cmd_generator_ (const char *text, int state)
{
  static const GList *comp = NULL;
  char *prefix = NULL;
  char *name = NULL;

  /* If this is a new word to complete, initialize now. */
  if (!state)
    {
      if (prefix) g_free (prefix);
      comp = mix_vm_cmd_dispatcher_complete (dis_, text, &prefix);
    }

  /* Return the next name which partially matches from the command list. */
  if (comp)
    {
      name = g_strdup ((const gchar *)comp->data);
      comp = comp->next;
    }

  return name;
}
#endif /* HAVE_LIBREADLINE */


/* emacs interface */
static void
emacs_output_ (mix_vm_cmd_dispatcher_t *dis, const gchar *arg, gpointer data)
{
  /* pek: probably bad that we snag the src w/every emacs_output_,
     however when multiple files are supported then this will
     have to be done each time (but the info will be snagged
     from elsewhere...) */
  const mix_vm_t *vm = mix_vm_cmd_dispatcher_get_vm (dis);
  const mix_src_file_t *src = mix_vm_get_src_file (vm);
  const gchar *path = mix_src_file_get_path (src);

  mix_address_t loc = mix_vm_get_prog_count (vm);
  guint lineno = mix_vm_get_address_lineno (vm, loc);

  printf ("\032\032mixvm:%s%s:%d\n", path, MIX_SRC_DEFEXT, lineno);
  return;
}

static int
cmd_quit_ (mix_vm_cmd_dispatcher_t *dis, const char *arg)
{
  puts (_("Quitting ..."));
  if (dis_) mix_vm_cmd_dispatcher_delete (dis_);
  if (config_) mix_config_delete (config_);
  exit (0);

  /* pek: anything needed here to make the marker disappear??? */
  return FALSE;
}

static int
cmd_shell_ (mix_vm_cmd_dispatcher_t *dis, const char *arg)
{
  system (arg);
  return TRUE;
}

static int
cmd_prompt_ (mix_vm_cmd_dispatcher_t *dis, const char *arg)
{
  if (arg && strlen (arg)) mix_vmloop_set_prompt (arg);
  return TRUE;
}


/* external interface */
static void
init_dis_ (mix_vm_cmd_dispatcher_t *dis)
{
  static const gchar * envars[] = { "MDK_EDITOR", "X_EDITOR", "EDITOR",
				    "VISUAL" };

  static const guint s = sizeof (envars) / sizeof (envars[0]);
  static const gchar *editor = NULL;
  gchar *edit = NULL;

  if (!editor)
    {
      int k;
      for (k = 0; k < s; k++)
	if ( (editor = getenv (envars[k])) != NULL ) break;
    }
  if (!editor) editor = "vi";
  edit = g_strconcat (editor, " %s", NULL);
  mix_vm_cmd_dispatcher_set_editor (dis, edit);
  g_free (edit);
  mix_vm_cmd_dispatcher_set_assembler (dis, "mixasm %s");
}

mix_vm_cmd_dispatcher_t *
mixvm_cmd_init (mix_config_t *config, char *arg, gboolean use_emacs)
{
  int k;

#ifdef HAVE_LIBREADLINE
  /* Tell the completer that we want a crack first. */
  rl_attempted_completion_function =
    (rl_completion_func_t *)mixvm_cmd_completion_;
#endif /* HAVE_LIBREADLINE */

  /* initialise the dispatcher */
  config_ = config;
  dis_ = mix_vm_cmd_dispatcher_new_with_config (stdout, stderr, config_);

  if ( dis_ == NULL)
    g_error (_("Failed initialisation (no memory resources)"));

  init_dis_ (dis_);

  /* add local commands */
  k = 0;
  while (commands[k].name)
    {
      mix_vm_cmd_dispatcher_register_new (dis_, commands + k);
      ++k;
    }

  /* install post hook for emacs interaction */
  if (use_emacs)
    {
      mix_vm_cmd_dispatcher_post_hook (dis_, MIX_CMD_LOAD, emacs_output_, NULL);
      mix_vm_cmd_dispatcher_post_hook (dis_, MIX_CMD_RUN, emacs_output_, NULL);
      mix_vm_cmd_dispatcher_post_hook (dis_, MIX_CMD_NEXT, emacs_output_, NULL);
    }

  if (arg)
    mix_vm_cmd_dispatcher_dispatch (dis_, MIX_CMD_LOAD, arg);

  return dis_;
}

gboolean
mixvm_cmd_exec (char *line)
{
  if (!line) return cmd_quit_(dis_, NULL);

  /* strip white  space */
  line = g_strstrip(line);

  if (strlen (line) == 0) return TRUE;

  if (try_guile_ (line)) return TRUE;

  (void)mix_vm_cmd_dispatcher_dispatch_text (dis_, line);

  return TRUE;
}
