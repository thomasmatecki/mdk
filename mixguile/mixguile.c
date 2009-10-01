/* -*-c-*- -------------- mixguile.c :
 * Implementation of the functions declared in mixguile.h
 * ------------------------------------------------------------------
 * Copyright (C) 2001, 2002, 2006, 2007, 2009 Free Software Foundation, Inc.
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

#include <mixlib/mix_config.h>
#include "mixguile_cmd_dispatcher.h"
#include "mixguile.h"

static mixguile_cmd_dispatcher_t *dispatcher_ = NULL;
static mix_vm_cmd_dispatcher_t *vm_dispatcher_ = NULL;
static main_func_t main_fun_;
static gboolean init_file_;

/* do local initialisation and enter the user provided main */

static void
real_main_ (void *closure, int argc, char *argv[])
{
  if (vm_dispatcher_)
    {
      mixguile_set_cmd_dispatcher (vm_dispatcher_);
      mixguile_load_bootstrap (init_file_);
    }
  (*main_fun_)(NULL, argc, argv);
}

/*
  initialise the guile command dispatcher and enter the provided
  main function.
*/
void
mixguile_init (int argc, char *argv[], gboolean initfile,
	       main_func_t main_fun,
	       mix_vm_cmd_dispatcher_t *dis)
{
  main_fun_ = main_fun;
  vm_dispatcher_ = dis;
  init_file_ = initfile;
  scm_boot_guile (argc, argv, real_main_, 0);
}

/* load bootstrap file */
void
mixguile_load_bootstrap (gboolean loadlocal)
{
  const gchar *scmfile = SCM_FILE;
  gchar *lscmfile = g_strconcat (g_get_home_dir (), G_DIR_SEPARATOR_S,
				 MIX_CONFIG_DIR, G_DIR_SEPARATOR_S,
				 LOCAL_SCM_FILE, NULL);

  if (access (scmfile, R_OK) && access ((scmfile = LOCAL_SCM_FILE), R_OK))
    {
      g_warning (_("mixguile bootstrap file %s not found\n"), SCM_FILE);
      scmfile = NULL;
    }
  else
    mixguile_interpret_file (scmfile);

  if (loadlocal && !access (lscmfile, R_OK))
    {
      mixguile_interpret_file (lscmfile);
    }

  g_free (lscmfile);
}

/* enter the guile repl */
void
mixguile_enter_repl (void *closure, int argc, char *argv[])
{
  scm_shell (argc, argv);
}

/* set the command dispatcher */
void
mixguile_set_cmd_dispatcher (mix_vm_cmd_dispatcher_t *dis)
{
  g_return_if_fail (dis != NULL);
  if (dispatcher_) mixguile_cmd_dispatcher_delete (dispatcher_);
  vm_dispatcher_ = dis;
  dispatcher_ = mixguile_cmd_dispatcher_new (dis);
  g_assert (dispatcher_);
}

/* access the mixguile comand dispatcher */
mix_vm_cmd_dispatcher_t *
mixguile_get_cmd_dispatcher (void)
{
  return mixguile_cmd_dispatcher_get_vm_dispatcher (dispatcher_);
}

/* execute a string or file using the guile interpreter */
void
mixguile_interpret_file (const gchar *path)
{
  mixguile_cmd_dispatcher_interpret_file (dispatcher_, path);
}

void
mixguile_interpret_command (const gchar *command)
{
  mixguile_cmd_dispatcher_interpret_command (dispatcher_, command);
}
