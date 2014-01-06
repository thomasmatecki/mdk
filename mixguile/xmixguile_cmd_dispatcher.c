/* -*-c-*- -------------- xmixguile_cmd_dispatcher.c :
 * Implementation of the functions declared in xmixguile_cmd_dispatcher.h
 * ------------------------------------------------------------------
 * Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2009, 2014 Free Software Foundation, Inc.
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

#include <string.h>

#include <mixlib/mix.h>
#include <libguile.h>

#include "xmixguile_cmd_dispatcher.h"

/* cmd dispatcher for use within the scm commands */
static mixguile_cmd_dispatcher_t *dispatcher_;
static mix_vm_cmd_dispatcher_t *vm_dispatcher_;
static mix_vm_t *vm_;
static SCM mutex_;

/* register a NULL-terminated list of scm commands */
void
register_scm_commands_ (const scm_command_t *commands)
{
  int k = 0;
  g_return_if_fail (commands != NULL);
  while (commands[k].name)
    {
      scm_c_define_gsubr (commands[k].name,
                          commands[k].argno,
                          commands[k].opt_argno,
                          commands[k].restp,
                          commands[k].func);
      ++k;
    }
}

/* register the mixvm cmd dispatcher to use with commands */
void
register_cmd_dispatcher_ (mixguile_cmd_dispatcher_t *dis)
{
  g_return_if_fail (dis != NULL);
  dispatcher_ = dis;
  vm_dispatcher_ = mixguile_cmd_dispatcher_get_vm_dispatcher (dis);
  vm_ = (mix_vm_t *) mix_vm_cmd_dispatcher_get_vm (vm_dispatcher_);
  mutex_ = scm_make_mutex ();
}

/* commands */
static SCM
mixvm_cmd_ (SCM cmd, SCM arg)
{
  char *com = NULL, *argu = NULL;

  SCM_ASSERT (scm_is_string (cmd) || scm_is_symbol (cmd),
              cmd, SCM_ARG1, "mixvm-cmd");
  SCM_ASSERT (scm_is_string (arg) || scm_is_symbol (arg),
              arg, SCM_ARG2, "mixvm-cmd");

  scm_lock_mutex (mutex_);
  com =  scm_to_locale_string (cmd);
  argu = scm_to_locale_string (arg);
  (void) mix_vm_cmd_dispatcher_dispatch (vm_dispatcher_,
                                         mix_vm_command_from_string (com),
                                         argu);
  g_free (com);
  g_free (argu);

  scm_unlock_mutex (mutex_);

  return SCM_UNSPECIFIED;
}

static SCM
mixvm_status_ (void)
{
  return scm_from_long (mix_vm_get_run_status (vm_));
}

static SCM
mix_last_result_ (void)
{
  return scm_from_bool (mix_vm_cmd_dispatcher_get_last_result (vm_dispatcher_));
}

static long
word_to_long_ (mix_word_t word)
{
  long result = mix_word_magnitude (word);
  return mix_word_is_negative (word) ? -result : result;
}

static long
short_to_long_ (mix_short_t s)
{
  long result = mix_short_magnitude (s);
  return mix_short_is_negative (s) ? -result : result;
}

static SCM
mix_reg_ (SCM reg)
{
  char *regis;
  long val = MIX_WORD_MAX + 1;

  SCM_ASSERT (scm_is_string (reg) || scm_is_symbol (reg),
              reg, SCM_ARG1, "mix-reg");

  scm_lock_mutex (mutex_);
  if (SCM_SYMBOLP (reg)) reg = scm_symbol_to_string (reg);
  regis = scm_to_locale_string (reg);
  switch (regis[0])
    {
    case 'A':
      val = word_to_long_ (mix_vm_get_rA (vm_)); break;
    case 'X':
      val = word_to_long_ (mix_vm_get_rX (vm_)); break;
    case 'J':
      val = short_to_long_ (mix_vm_get_rJ (vm_)); break;
    case 'I':
      {
	int i = regis[1] - '0';
	if (i > 0 && i < 7) val = short_to_long_ (mix_vm_get_rI (vm_, i));
      }
      break;
    default:
      break;
    }
  g_free (regis);

  scm_unlock_mutex (mutex_);

  SCM_ASSERT (val <= MIX_WORD_MAX, reg, SCM_ARG1, "mix-reg");

  return scm_from_long (val);
}

static SCM
mix_set_reg_ (SCM reg, SCM value)
{
  char *regis;
  long val;
  gboolean result = TRUE;

  SCM_ASSERT (scm_is_string (reg) || scm_is_symbol (reg),
              reg, SCM_ARG1, "mix-set-reg!");
  SCM_ASSERT (scm_is_number (value), value, SCM_ARG2, "mix-set-reg!");

  scm_lock_mutex (mutex_);
  if (SCM_SYMBOLP (reg)) reg = scm_symbol_to_string (reg);
  regis = scm_to_locale_string (reg);
  val = scm_to_long (value);
  switch (regis[0])
    {
    case 'A':
      mix_vm_set_rA (vm_, mix_word_new (val)); break;
    case 'X':
      mix_vm_set_rX (vm_, mix_word_new (val)); break;
    case 'J':
      mix_vm_set_rJ (vm_, mix_short_new (val)); break;
    case 'I':
      {
	int i = regis[1] - '0';
	if (i > 0 && i < 7) mix_vm_set_rI (vm_, i, mix_short_new (val));
	else result = FALSE;
      }
      break;
    default:
      result = FALSE; break;
    }
  g_free (regis);

  scm_unlock_mutex (mutex_);

  SCM_ASSERT (result, reg, SCM_ARG1, "mix-set-reg!");

  return SCM_BOOL_T;
}

static SCM
mix_cell_ (SCM no)
{
  int cell;
  long result;

  SCM_ASSERT (SCM_NUMBERP (no), no, SCM_ARG1, "mix-cell");
  cell = scm_to_int (no);
  SCM_ASSERT (cell < MIX_VM_CELL_NO, no, SCM_ARG1, "mix-cell");
  result = word_to_long_ (mix_vm_get_addr_contents (vm_, cell));
  return scm_from_long (result);
}

static SCM
mix_set_cell_ (SCM no, SCM val)
{
  int cell;
  long result;

  SCM_ASSERT (SCM_NUMBERP (no), no, SCM_ARG1, "mix-set-cell!");
  SCM_ASSERT (SCM_NUMBERP (val), no, SCM_ARG2, "mix-set-cell!");
  cell = scm_to_int (no);
  SCM_ASSERT (cell < MIX_VM_CELL_NO, no, SCM_ARG1, "mix-set-cell!");
  result = scm_to_long (val);
  mix_vm_set_addr_contents (vm_, cell, mix_word_new (result));
  return SCM_BOOL_T;
}

static SCM
mix_over_ (void)
{
  return scm_from_bool (mix_vm_get_overflow (vm_));
}

static SCM
mix_set_over_ (SCM over)
{
  mix_vm_set_overflow (vm_, scm_to_bool (over));
  return SCM_BOOL_T;
}

static SCM
mix_loc_ (void)
{
  return scm_from_long (mix_vm_get_prog_count (vm_));
}

static SCM
mix_cmp_ (void)
{
  gchar *result = NULL;
  switch (mix_vm_get_cmpflag (vm_))
    {
    case mix_LESS: result = "L"; break;
    case mix_EQ: result = "E"; break;
    case mix_GREAT: result = "G"; break;
    default: g_assert_not_reached ();
    }
  return scm_from_locale_symbol (result);
}

static SCM
mix_set_cmp_ (SCM value)
{
  gchar *val = NULL;
  mix_cmpflag_t result = -1;

  SCM_ASSERT (scm_is_string (value) || scm_is_symbol (value),
              value, SCM_ARG1, "mix-set-cmp!");

  scm_lock_mutex (mutex_);
  val = scm_to_locale_string (value);
  if (strlen (val) == 1)
    {
      switch (val[0])
	{
	case 'L': result = mix_LESS; break;
	case 'E': result = mix_EQ; break;
	case 'G': result = mix_GREAT; break;
	default: break;
	}
    }
  g_free (val);
  scm_unlock_mutex (mutex_);
  SCM_ASSERT (result != -1, value, SCM_ARG1, "mix-set-cmp!");
  mix_vm_set_cmpflag (vm_, result);
  return SCM_BOOL_T;
}

static SCM
mix_src_name_ (void)
{
  const gchar *path = mix_vm_cmd_dispatcher_get_src_file_path (vm_dispatcher_);
  return scm_from_locale_string (path? g_path_get_basename (path) : "");
}

static SCM
mix_src_path_ (void)
{
  const gchar *path = mix_vm_cmd_dispatcher_get_src_file_path (vm_dispatcher_);
  return scm_from_locale_string (path? (char *)path : "");
}

static SCM
mix_prog_name_ (void)
{
  const gchar *path = mix_vm_cmd_dispatcher_get_program_path (vm_dispatcher_);
  return scm_from_locale_string (path? g_path_get_basename (path) : "");
}

static SCM
mix_prog_path_ (void)
{
  const gchar *path = mix_vm_cmd_dispatcher_get_program_path (vm_dispatcher_);
  return scm_from_locale_string (path? (char *)path : "");
}

static SCM
mix_ddir_ (void)
{
  return scm_from_locale_string ((char *)mix_device_get_dir ());
}

static SCM
mix_uptime_ (void)
{
  return scm_from_long (mix_vm_cmd_dispatcher_get_uptime (vm_dispatcher_));
}

static SCM
mix_progtime_ (void)
{
  return scm_from_long (mix_vm_cmd_dispatcher_get_progtime (vm_dispatcher_));
}

static SCM
mix_laptime_ (void)
{
  return scm_from_long (mix_vm_cmd_dispatcher_get_laptime (vm_dispatcher_));
}

static SCM
mix_src_line_ (SCM opt)
{
  gulong no = 0;
  const gchar *line = "";
  if (opt != SCM_UNDEFINED)
    {
      SCM_ASSERT (SCM_NUMBERP (opt), opt, SCM_ARG1, "mix-src-line");
      no = scm_to_ulong (opt);
    }
  else
    no = mix_vm_cmd_dispatcher_get_src_file_lineno (vm_dispatcher_);

  SCM_ASSERT (line >= 0, opt, SCM_ARG1, "mix-src-line");

  if (no > 0)
    line = mix_vm_cmd_dispatcher_get_src_file_line (vm_dispatcher_, no, FALSE);

  return scm_from_locale_string ((char *)line);
}

static SCM
mix_src_line_no_ (void)
{
  return
    scm_from_long (mix_vm_cmd_dispatcher_get_src_file_lineno (vm_dispatcher_));
}

/* ----- hook functions ---- */

/* auxiliar arg list maker */
static SCM
make_arg_list_ (const gchar *arg)
{
  gchar **arglist = g_strsplit (arg, " ", -1);
  SCM argument = scm_list_n (SCM_UNDEFINED, SCM_EOL);
  if (arglist && arglist[0])
    {
      int k = 0;
      while (arglist[k])
	argument = scm_cons (scm_from_locale_string (arglist[k++]), argument);
      argument = scm_reverse (argument);
    }
  g_strfreev (arglist);
  return argument;
}

/* command hook auxiliar functions and types */
/*
static SCM
hook_error_handler_ (void *data, SCM tag, SCM args){}
*/
typedef struct
{
  SCM function;
  SCM args;
} hook_data_t;

static SCM
hook_catch_body_ (void *data)
{
  hook_data_t *h = (hook_data_t *)data;
  return scm_call_1 (h->function, h->args);
}

static void
scm_hook_ (mix_vm_cmd_dispatcher_t *dis, const gchar *arg, gpointer data)
{
  hook_data_t h;
  h.function = (SCM) data;

  g_assert (scm_is_true (scm_procedure_p (h.function)));

  h.args = make_arg_list_ (arg);
  g_assert (scm_is_true (scm_list_p (h.args)));

  scm_internal_catch (SCM_BOOL_T, hook_catch_body_, &h,
                      scm_handle_by_message_noexit, dis);
}

/* global hook auxiliar functions and types */
typedef struct
{
  SCM function;
  SCM cmd;
  SCM args;
} global_hook_data_t;

static SCM
global_hook_catch_body_ (void *data)
{
  global_hook_data_t *h = (global_hook_data_t *)data;
  return scm_call_2 (h->function, h->cmd, h->args);
}

static void
scm_global_hook_ (mix_vm_cmd_dispatcher_t *dis, mix_vm_command_t cmd,
		  const gchar *arg, gpointer data)
{
  global_hook_data_t h;
  h.function = (SCM) data;
  h.cmd = scm_from_locale_string ((char *)mix_vm_command_to_string (cmd));
  h.args = make_arg_list_ (arg);
  scm_internal_catch (SCM_BOOL_T, global_hook_catch_body_, &h,
                      scm_handle_by_message_noexit, NULL);
}

static SCM
define_hook_procedure_ (SCM function)
{
  enum {BUFF_SIZE = 128};
  static gchar BUFFER[BUFF_SIZE];
  static const gchar *PATTERN = "____mix__hook__%d____";
  static int K = 0;
  g_snprintf (BUFFER, BUFF_SIZE, PATTERN, K++);
  /* scm_c__define (name, val) returns a pair: (symbol . symbol-value) */
  return scm_cdr (scm_c_define ((char *)BUFFER, function));
}

static SCM
mix_add_hook_ (SCM cmd, SCM function, gboolean pre)
{
  gchar *cmdstr = NULL;
  mix_vm_command_t command;
  const gchar *fun = pre? "mix-add-pre-hook" : "mix-add-post-hook";

  SCM_ASSERT (scm_is_string (cmd) || scm_is_symbol (cmd), cmd, SCM_ARG1, fun);
  SCM_ASSERT (scm_is_true (scm_procedure_p (function)), function, SCM_ARG2, fun);
  scm_lock_mutex (mutex_);
  cmdstr = scm_to_locale_string (cmd);
  command = mix_vm_command_from_string (cmdstr);
  g_free (cmdstr);
  scm_unlock_mutex (mutex_);
  SCM_ASSERT (command != MIX_CMD_INVALID, cmd, SCM_ARG1, fun);
  scm_lock_mutex (mutex_);
  if (pre)
    mix_vm_cmd_dispatcher_pre_hook (vm_dispatcher_, command, scm_hook_,
				    (gpointer) define_hook_procedure_ (function));
  else
    mix_vm_cmd_dispatcher_post_hook (vm_dispatcher_, command, scm_hook_,
				     (gpointer) define_hook_procedure_ (function));
  scm_unlock_mutex (mutex_);
  return SCM_BOOL_T;
}

static SCM
mix_add_global_hook_ (SCM function, gboolean pre)
{
  const gchar *fun =
    pre? "mix-add-global-pre-hook" : "mix-add-global-post-hook";

  SCM_ASSERT (scm_is_true (scm_procedure_p (function)), function, SCM_ARG1, fun);
  scm_lock_mutex (mutex_);
  if (pre)
    mix_vm_cmd_dispatcher_global_pre_hook (vm_dispatcher_, scm_global_hook_,
					   (gpointer) define_hook_procedure_ (function));
  else
    mix_vm_cmd_dispatcher_global_post_hook (vm_dispatcher_, scm_global_hook_,
					    (gpointer) define_hook_procedure_ (function));
  scm_unlock_mutex (mutex_);
  return SCM_BOOL_T;
}

static SCM
mix_add_pre_hook_ (SCM cmd, SCM function)
{
  return mix_add_hook_ (cmd, function, TRUE);
}

static SCM
mix_add_post_hook_ (SCM cmd, SCM function)
{
  return mix_add_hook_ (cmd, function, FALSE);
}

static SCM
mix_add_global_pre_hook_ (SCM function)
{
  return mix_add_global_hook_ (function, TRUE);
}

static SCM
mix_add_global_post_hook_ (SCM function)
{
  return mix_add_global_hook_ (function, FALSE);
}

/* NULL-terminated list of available scm commands */
const scm_command_t DEFAULT_SCM_COMMANDS_[] = {
  {"mixvm-cmd", mixvm_cmd_, 2, 0, 0},
  {"mixvm-status", mixvm_status_, 0, 0, 0},
  {"mix-last-result", mix_last_result_, 0, 0, 0},
  {"mix-reg", mix_reg_, 1, 0, 0},
  {"mix-set-reg!", mix_set_reg_, 2, 0, 0},
  {"mix-cell", mix_cell_, 1, 0, 0},
  {"mix-set-cell!", mix_set_cell_, 2, 0, 0},
  {"mix-over", mix_over_, 0, 0, 0},
  {"mix-loc", mix_loc_, 0, 0, 0},
  {"mix-set-over!", mix_set_over_, 1, 0, 0},
  {"mix-cmp", mix_cmp_, 0, 0, 0},
  {"mix-up-time", mix_uptime_, 0, 0, 0},
  {"mix-lap-time", mix_laptime_, 0, 0, 0},
  {"mix-prog-time", mix_progtime_, 0, 0, 0},
  {"mix-prog-name", mix_prog_name_, 0, 0, 0},
  {"mix-prog-path", mix_prog_path_, 0, 0, 0},
  {"mix-src-name", mix_src_name_, 0, 0, 0},
  {"mix-src-path", mix_src_path_, 0, 0, 0},
  {"mix-src-line-no", mix_src_line_no_, 0, 0, 0},
  {"mix-src-line", mix_src_line_, 0, 1, 0},
  {"mix-ddir", mix_ddir_, 0, 0, 0},
  {"mix-set-cmp!", mix_set_cmp_, 1, 0, 0},
  {"mix-add-pre-hook", mix_add_pre_hook_, 2, 0, 0},
  {"mix-add-post-hook", mix_add_post_hook_, 2, 0, 0},
  {"mix-add-global-pre-hook", mix_add_global_pre_hook_, 1, 0, 0},
  {"mix-add-global-post-hook", mix_add_global_post_hook_, 1, 0, 0},
  {NULL}
};
