/* -*-c-*- -------------- mix_vm_command.c :
 * Implementation of the functions declared in mix_vm_command.h
 * ------------------------------------------------------------------
 * Copyright (C) 2001, 2002, 2004, 2006, 2007, 2014 Free Software Foundation, Inc.
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


#include "xmix_vm_command.h"
#include "xmix_vm_handlers.h"
#include "completion.h"

#ifdef HAVE_LIBHISTORY
#  include <readline/history.h>
#endif

#ifdef HAVE_LIBREADLINE
#  include <readline/readline.h>
#endif

/* hook execution */
static void
exec_hook_list_ (GSList *list, mix_vm_cmd_dispatcher_t *dis, const gchar *arg)
{
  while (list)
    {
      hook_ *hook = (hook_ *)list->data;
      if (hook)
	(hook->func)(dis, arg, hook->data);
      list = list->next;
    }
}

static void
exec_global_hook_list_ (GSList *list,
			mix_vm_cmd_dispatcher_t *dis,
			mix_vm_command_t cmd, const gchar *arg)
{
  while (list)
    {
      global_hook_ *hook = (global_hook_ *)list->data;
      if (hook)
	(hook->func)(dis, cmd, arg, hook->data);
      list = list->next;
    }
}

/* conversion from/to commands to strings */
const gchar *
mix_vm_command_to_string (mix_vm_command_t cmd)
{
  if (cmd < MIX_CMD_INVALID) return commands_[cmd].name;
  else return NULL;
}

mix_vm_command_t
mix_vm_command_from_string (const gchar *name)
{
  gint cmd = 0;
  while (cmd < MIX_CMD_INVALID && strcmp (name, commands_[cmd].name))
    ++cmd;
  return cmd;
}

/* get help string about a command */
const gchar *
mix_vm_command_help (mix_vm_command_t cmd)
{
  if (cmd < MIX_CMD_INVALID) return commands_[cmd].doc;
  else return NULL;
}

const gchar *
mix_vm_command_usage (mix_vm_command_t cmd)
{
  if (cmd < MIX_CMD_INVALID) return commands_[cmd].usage;
  else return NULL;
}

/* create a new command dispatcher */
static Completion *
make_completions_ (void)
{
  GList *cmds = NULL;
  gint k;

  Completion *completions = completion_new (NULL);
  for (k = 0; k < MIX_CMD_INVALID; ++k)
    cmds = g_list_append (cmds, (gpointer) mix_vm_command_to_string (k));
  completion_add_items (completions, cmds);
  return completions;
}

mix_vm_cmd_dispatcher_t *
mix_vm_cmd_dispatcher_new (FILE *out_fd, /* output messages file */
			   FILE *err_fd /* error messages file */)
{
  mix_vm_cmd_dispatcher_t *result = NULL;
  int k;

  /* g_return_val_if_fail (out_fd && err_fd, NULL); */

  result = g_new (mix_vm_cmd_dispatcher_t, 1);
  result->result = TRUE;
  result->out = out_fd;
  result->err = err_fd;
  result->log_msg = TRUE;
  result->uptime = result->laptime = result->progtime = 0;
  result->printtime = TRUE;
  result->trace = FALSE;
  result->program = NULL;
  result->editor = NULL;
  result->assembler = NULL;
  result->eval = mix_eval_new ();
  result->dump = mix_dump_context_new (out_fd,
				       MIX_SHORT_ZERO, MIX_SHORT_ZERO,
				       MIX_DUMP_ALL);
  result->vm = mix_vm_new ();
  result->global_pre = result->global_post = NULL;

  for (k =0; k < MIX_CMD_INVALID; ++k)
    result->pre_hooks[k] = result->post_hooks[k] = NULL;

  result->config = NULL;

  for (k = 0; k < PRNO_; ++k)
    result->preds[k] = mix_predicate_new (k);

  result->mem_preds = g_hash_table_new (NULL, NULL);
  result->commands = g_hash_table_new (g_str_hash, g_str_equal);
  result->completions = make_completions_ ();

  return result;
}

mix_vm_cmd_dispatcher_t *
mix_vm_cmd_dispatcher_new_with_config (FILE *out, FILE *err,
				       mix_config_t *config)
{
  mix_vm_cmd_dispatcher_t *result = mix_vm_cmd_dispatcher_new (out, err);
  if (result != NULL && (result->config = config) != NULL)
    {
#ifdef HAVE_LIBHISTORY
      gint hsize = 0;
#endif
      const gchar *val = mix_config_get (result->config, TRACING_KEY_);
      if (val) cmd_strace_ (result, val);
      val = mix_config_get (result->config, EDITOR_KEY_);
      if (val) mix_vm_cmd_dispatcher_set_editor (result, val);
      val = mix_config_get (result->config, ASM_KEY_);
      if (val) mix_vm_cmd_dispatcher_set_assembler (result, val);
      val = mix_config_get (result->config, TIMING_KEY_);
      if (val) cmd_stime_ (result, val);
      val = mix_config_get_devices_dir (result->config);
      if (!val || !mix_stat_dir (val, "devices"))
	{
	  gchar *dirname =
            g_path_get_dirname (mix_config_get_filename (config));
	  cmd_sddir_ (result, dirname);
	  g_free (dirname);
	}
      else
	mix_device_set_dir (val);
      val = mix_config_get (result->config, LOGGING_KEY_);
      if (val) cmd_slog_ (result, val);
#ifdef HAVE_LIBHISTORY
      val = mix_config_get_history_file (result->config);
      hsize = mix_config_get_history_size (result->config);
      using_history ();
      stifle_history (hsize);
      if (val)
	{
	  read_history ((char *)val);
	  history_set_pos (history_base + history_length - 1);
	}
#endif
    }
  return result;
}


/* delete (does not close the fds in the constructor) */
static gboolean
del_pred_ (gpointer key, gpointer val, gpointer data)
{
  if (val) mix_predicate_delete ((mix_predicate_t *)val);
  return TRUE;
}

static void
del_hook_ (gpointer data, gpointer ignored)
{
  if (data) g_free (data);
}

static void
del_hook_list_ (GSList *s)
{
  if (s)
    {
      g_slist_foreach (s, del_hook_, NULL);
      g_slist_free (s);
    }
}

void
mix_vm_cmd_dispatcher_delete (mix_vm_cmd_dispatcher_t *dis)
{
#ifdef HAVE_LIBHISTORY
  const gchar *hfile = NULL;
#endif
  gint k;

  g_return_if_fail (dis != NULL);
  mix_eval_delete (dis->eval);
  mix_dump_context_delete (dis->dump);
  mix_vm_delete (dis->vm);
  if (dis->editor) g_free (dis->editor);
  if (dis->editor) g_free (dis->assembler);
#ifdef HAVE_LIBHISTORY
  if (dis->config && (hfile = mix_config_get_history_file
		      (dis->config)))
    write_history ((char *)hfile);
#endif
  for (k = 0; k < PRNO_; ++k) mix_predicate_delete (dis->preds[k]);
  g_hash_table_foreach_remove (dis->mem_preds, del_pred_, NULL);
  g_hash_table_destroy (dis->mem_preds);
  g_hash_table_destroy (dis->commands);
  completion_free (dis->completions);
  for (k = 0; k < MIX_CMD_INVALID; ++k)
    {
      del_hook_list_ (dis->pre_hooks[k]);
      del_hook_list_ (dis->post_hooks[k]);
    }
  del_hook_list_ (dis->global_pre);
  del_hook_list_ (dis->global_post);
  g_free (dis);
}

/* register new commands for a dispatcher */
void
mix_vm_cmd_dispatcher_register_new (mix_vm_cmd_dispatcher_t *dis,
				    mix_vm_command_info_t *cmd)
{
  GList *list = NULL;
  g_return_if_fail (dis != NULL);
  g_return_if_fail (cmd != NULL);
  g_hash_table_insert (dis->commands, (gpointer)cmd->name, (gpointer)cmd);
  list = g_list_append (list, (gpointer)cmd->name);
  completion_add_items (dis->completions, list);
}

const GList *
mix_vm_cmd_dispatcher_complete (const mix_vm_cmd_dispatcher_t *dis,
				const gchar *cmd, gchar **prefix)
{
  char *cp;
  GList *result;

  g_return_val_if_fail (dis != NULL, NULL);
  g_return_val_if_fail (cmd != NULL, NULL);

  cp = g_strdup (cmd);
  result = completion_complete (dis->completions, cp, prefix);
  g_free (cp);
  return result;
}

/* set/get out/error streams */
FILE *
mix_vm_cmd_dispatcher_get_out_stream (const mix_vm_cmd_dispatcher_t *dis)
{
  g_return_val_if_fail (dis, NULL);
  return dis->out;
}

FILE *
mix_vm_cmd_dispatcher_get_err_stream (const mix_vm_cmd_dispatcher_t *dis)
{
  g_return_val_if_fail (dis, NULL);
  return dis->err;
}

FILE * /* old output stream */
mix_vm_cmd_dispatcher_set_out_stream (mix_vm_cmd_dispatcher_t *dis, FILE *out)
{
  FILE *old = NULL;
  g_return_val_if_fail (dis != NULL, old);
  old = dis->out;
  dis->out = out;
  dis->dump->channel = out;
  return old;
}

FILE * /* old error stream */
mix_vm_cmd_dispatcher_set_error_stream (mix_vm_cmd_dispatcher_t *dis,
					FILE *err)
{
  FILE *old = NULL;
  g_return_val_if_fail (dis != NULL, old);
  old = dis->err;
  dis->err = err;
  return old;
}


/* set editor and compiler templates */
void
mix_vm_cmd_dispatcher_set_editor (mix_vm_cmd_dispatcher_t *dis,
				  const gchar *edit_tplt)
{
  g_return_if_fail (dis != NULL);
  if (dis->editor) g_free (dis->editor);
  dis->editor = (edit_tplt) ? g_strdup (edit_tplt) : NULL;
  if (dis->config && dis->editor)
    mix_config_update (dis->config, EDITOR_KEY_, dis->editor);
}

void
mix_vm_cmd_dispatcher_set_assembler (mix_vm_cmd_dispatcher_t *dis,
				     const gchar *asm_tplt)
{
  g_return_if_fail (dis != NULL);
  if (dis->assembler) g_free (dis->assembler);
  dis->assembler = (asm_tplt) ? g_strdup (asm_tplt) : NULL;
  if (dis->config && dis->assembler)
    mix_config_update (dis->config, ASM_KEY_, dis->assembler);
}

const gchar *
mix_vm_cmd_dispatcher_get_editor (const mix_vm_cmd_dispatcher_t *dis)
{
  g_return_val_if_fail (dis != NULL, NULL);
  return dis->editor;
}

const gchar *
mix_vm_cmd_dispatcher_get_assembler (const mix_vm_cmd_dispatcher_t *dis)
{
  g_return_val_if_fail (dis != NULL, NULL);
  return dis->assembler;
}

const gchar *
mix_vm_cmd_dispatcher_get_src_file_path (const mix_vm_cmd_dispatcher_t *dis)
{
  static gchar *PATH = NULL;

  const mix_vm_t *vm = mix_vm_cmd_dispatcher_get_vm (dis);
  const mix_src_file_t *f  = mix_vm_get_src_file (vm);

  if (PATH)
    {
      g_free (PATH);
      PATH = NULL;
    }

  if (f)
    PATH = mix_file_complete_name (mix_src_file_get_path (f), MIX_SRC_DEFEXT);

  return PATH;
}

const gchar *
mix_vm_cmd_dispatcher_get_program_path (const mix_vm_cmd_dispatcher_t *dis)
{
  g_return_val_if_fail (dis != NULL, NULL);
  return dis->program;
}

/* install hooks */
void
mix_vm_cmd_dispatcher_pre_hook (mix_vm_cmd_dispatcher_t *dis,
				mix_vm_command_t cmd,
				mix_vm_cmd_hook_t hook, gpointer data)
{
  hook_ *phook;

  g_return_if_fail (dis != NULL);
  g_return_if_fail (cmd < MIX_CMD_INVALID);

  phook = g_new (hook_, 1);
  phook->func = hook;
  phook->data = data;

  dis->pre_hooks[cmd] = g_slist_append (dis->pre_hooks[cmd], phook);
}

void
mix_vm_cmd_dispatcher_post_hook (mix_vm_cmd_dispatcher_t *dis,
				 mix_vm_command_t cmd,
				 mix_vm_cmd_hook_t hook, gpointer data)
{
  hook_ *phook;

  g_return_if_fail (dis != NULL);
  g_return_if_fail (cmd < MIX_CMD_INVALID);

  phook = g_new (hook_, 1);
  phook->func = hook;
  phook->data = data;

  dis->post_hooks[cmd] = g_slist_append (dis->post_hooks[cmd], phook);
}

void
mix_vm_cmd_dispatcher_global_pre_hook (mix_vm_cmd_dispatcher_t *dis,
				       mix_vm_cmd_global_hook_t hook,
				       gpointer data)
{
  global_hook_ *phook;

  g_return_if_fail (dis != NULL);

  phook = g_new (global_hook_, 1);
  phook->func = hook;
  phook->data = data;

  dis->global_pre = g_slist_append (dis->global_pre, phook);
}

void
mix_vm_cmd_dispatcher_global_post_hook (mix_vm_cmd_dispatcher_t *dis,
					mix_vm_cmd_global_hook_t hook,
					gpointer data)
{
  global_hook_ *phook;

  g_return_if_fail (dis != NULL);

  phook = g_new (global_hook_, 1);
  phook->func = hook;
  phook->data = data;

  dis->global_post = g_slist_append (dis->global_post, phook);
}

/* dispatch a command */
gboolean /* TRUE if success, FALSE otherwise */
mix_vm_cmd_dispatcher_dispatch (mix_vm_cmd_dispatcher_t *dis,
				mix_vm_command_t cmd, const gchar *arg)
{
  g_return_val_if_fail (dis != NULL, FALSE);

  if (dis->global_pre)
    exec_global_hook_list_ (dis->global_pre, dis, cmd, arg);

  if (cmd < MIX_CMD_INVALID)
    {
      if (dis->pre_hooks[cmd])
	exec_hook_list_ (dis->pre_hooks[cmd], dis, arg);

      fflush (dis->out);
      fflush (dis->err);

      dis->result = (commands_[cmd].func)(dis, arg);

      fflush (dis->out);
      fflush (dis->err);

      if (dis->post_hooks[cmd])
	exec_hook_list_ (dis->post_hooks[cmd], dis, arg);

      fflush (dis->out);
      fflush (dis->err);
    }
  else
    {
      fprintf (dis->err, _("Unknown command. Try: help\n"));
    }

  if (dis->global_post)
    exec_global_hook_list_ (dis->global_post, dis, cmd, arg);

  fflush (dis->out);
  fflush (dis->err);
  return dis->result;
}

/* dispatch a command in text format */
gboolean
mix_vm_cmd_dispatcher_dispatch_text (mix_vm_cmd_dispatcher_t *dis,
				     const gchar *text)
{
  gchar *cp, *arg = "";
  int k = 0;

  g_return_val_if_fail (dis != NULL, FALSE);
  g_return_val_if_fail (text != NULL, FALSE);

  cp = g_strdup (text);
  while (cp[k] && !isspace (cp[k])) ++k;
  if (cp[k])
    {
      cp[k] = '\0'; ++k;
      while (cp[k] && isspace (cp[k])) ++k;
      arg = cp + k;
    }

  (void) mix_vm_cmd_dispatcher_dispatch_split_text (dis, cp, arg);

  g_free (cp);

  return dis->result;
}

/* dispatch a command in text format, with command and arg split */
gboolean
mix_vm_cmd_dispatcher_dispatch_split_text (mix_vm_cmd_dispatcher_t *dis,
					   const gchar *command,
					   const gchar *arg)
{
  mix_vm_command_info_t *info;

  g_return_val_if_fail (dis, FALSE);

  if (!command) return FALSE;
  if (!arg) arg = "";

  info = (mix_vm_command_info_t *)g_hash_table_lookup (dis->commands, command);

  if (info)
    {
      if (dis->global_pre)
	exec_global_hook_list_ (dis->global_pre, dis, MIX_CMD_LOCAL, arg);

      fflush (dis->out);
      fflush (dis->err);

      dis->result = info->func (dis, arg);

      fflush (dis->out);
      fflush (dis->err);

      if (dis->global_post)
	exec_global_hook_list_ (dis->global_post, dis, MIX_CMD_LOCAL, arg);

      fflush (dis->out);
      fflush (dis->err);
    }
  else
    dis->result =
      mix_vm_cmd_dispatcher_dispatch (dis,
				      mix_vm_command_from_string (command),
				      arg);
  return dis->result;
}


/* get the last dispatch's result */
gboolean
mix_vm_cmd_dispatcher_get_last_result (const mix_vm_cmd_dispatcher_t *dis)
{
  g_return_val_if_fail (dis != NULL, FALSE);
  return dis->result;
}

/* get total uptime */
mix_time_t
mix_vm_cmd_dispatcher_get_uptime (const mix_vm_cmd_dispatcher_t *dis)
{
  g_return_val_if_fail (dis != NULL, 0);
  return dis->uptime;
}

/* get program total time */
mix_time_t
mix_vm_cmd_dispatcher_get_progtime (const mix_vm_cmd_dispatcher_t *dis)
{
  g_return_val_if_fail (dis != NULL, 0);
  return dis->progtime;
}

/* get time lapse */
mix_time_t
mix_vm_cmd_dispatcher_get_laptime (const mix_vm_cmd_dispatcher_t *dis)
{
  g_return_val_if_fail (dis != NULL, 0);
  return dis->laptime;
}

/* toggle time printing */
void
mix_vm_cmd_dispatcher_print_time (mix_vm_cmd_dispatcher_t * dis, gboolean print)
{
  g_return_if_fail (dis != NULL);
  dis->printtime = print;
}

/* get the mix vm */
const mix_vm_t *
mix_vm_cmd_dispatcher_get_vm (const mix_vm_cmd_dispatcher_t *dis)
{
  g_return_val_if_fail (dis != NULL, NULL);
  return dis->vm;
}

/* src file info */
gulong
mix_vm_cmd_dispatcher_get_src_file_lineno (const mix_vm_cmd_dispatcher_t *dis)
{
  g_return_val_if_fail (dis != NULL, 0);
  return mix_vm_get_break_lineno (dis->vm);
}

const gchar *
mix_vm_cmd_dispatcher_get_src_file_line (const mix_vm_cmd_dispatcher_t *dis,
					 gulong line, gboolean cr)
{
  const mix_src_file_t *file;
  g_return_val_if_fail (dis != NULL, NULL);

  file = mix_vm_get_src_file (dis->vm);

  if (line == 0 || file == NULL) return cr? "" : "\n";

  if (cr)
    return mix_src_file_get_line (file, line);
  else
    {
      enum {BUFF_SIZE = 256};
      static gchar BUFFER[BUFF_SIZE];
      int len = g_snprintf (BUFFER, BUFF_SIZE,
                            "%s",
                            mix_src_file_get_line (file, line));
      if (len > 0 && BUFFER[len - 1] == '\n') BUFFER[len - 1] = '\0';
      return BUFFER;
    }
}
