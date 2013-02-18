/* -*-c-*- -------------- mixguile_cmd_dispatcher.c :
 * Implementation of the functions declared in mixguile_cmd_dispatcher.h
 * ------------------------------------------------------------------
 * Copyright (C) 2001, 2007, 2009 Free Software Foundation, Inc.
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

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include <libguile.h>
#include "mixguile.h"
#include "xmixguile_cmd_dispatcher.h"

#define SCM_CMD  "scm"
#define SCMF_CMD "scmf"

/*local commands */

static SCM eval_ (void *code)
{
  scm_c_eval_string ((char *)code);
  return SCM_BOOL_T;
}

static gboolean
cmd_scm_ (mix_vm_cmd_dispatcher_t *dis, const gchar *arg)
{
  scm_c_catch (SCM_BOOL_T, eval_, (void*) arg,
               scm_handle_by_message_noexit, NULL, NULL, NULL);
  return TRUE;
}

static SCM load_ (void *path)
{
  scm_c_primitive_load ((char *)path);
  return SCM_BOOL_T;
}

static gboolean
cmd_scmf_ (mix_vm_cmd_dispatcher_t *dis, const gchar *arg)
{
  scm_c_catch (SCM_BOOL_T, load_, (void*) arg,
               scm_handle_by_message_noexit, NULL, NULL, NULL);
  return TRUE;
}

static mix_vm_command_info_t commands_[] = {
  { SCM_CMD, cmd_scm_, N_("Eval Scheme command using Guile"), "scm COMMAND"},
  { SCMF_CMD, cmd_scmf_, N_("Eval Scheme file using Guile"), "scmf PATH"},
  {NULL}
};

/* create/destroy cmd dispatcher */
mixguile_cmd_dispatcher_t *
mixguile_cmd_dispatcher_new (mix_vm_cmd_dispatcher_t *dis)
{
  static gboolean REGISTERED = FALSE;
  mixguile_cmd_dispatcher_t *result = NULL;
  int k = 0;

  g_return_val_if_fail (dis != NULL, NULL);

  if (!REGISTERED)
    {
      register_scm_commands_ (DEFAULT_SCM_COMMANDS_);
      REGISTERED = TRUE;
    }

  result = g_new (mixguile_cmd_dispatcher_t, 1);
  result->dispatcher = dis;

  while (commands_[k].name)
    {
      mix_vm_cmd_dispatcher_register_new (dis, commands_ + k);
      ++k;
    }

  register_cmd_dispatcher_ (result);

  return result;
}

void
mixguile_cmd_dispatcher_delete (mixguile_cmd_dispatcher_t *dis)
{
  g_return_if_fail (dis != NULL);
  mix_vm_cmd_dispatcher_delete (dis->dispatcher);
}

/* get the underlying vm dispatcher */
mix_vm_cmd_dispatcher_t *
mixguile_cmd_dispatcher_get_vm_dispatcher (const mixguile_cmd_dispatcher_t *dis)
{
  g_return_val_if_fail (dis != NULL, NULL);
  return dis->dispatcher;
}

void
mixguile_cmd_dispatcher_interpret_file (mixguile_cmd_dispatcher_t *dis,
					const gchar *path)
{
  g_return_if_fail (dis != NULL);
  g_return_if_fail (path != NULL);
  mix_vm_cmd_dispatcher_dispatch_split_text (dis->dispatcher,
					     SCMF_CMD, path);
}

void
mixguile_cmd_dispatcher_interpret_command (mixguile_cmd_dispatcher_t *dis,
					   const gchar *command)
{
  g_return_if_fail (dis != NULL);
  g_return_if_fail (command != NULL);
  mix_vm_cmd_dispatcher_dispatch_split_text (dis->dispatcher,
					     SCM_CMD, command);
}

