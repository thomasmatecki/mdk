/* -*-c-*- ---------------- xmixguile_cmd_dispatcher.h :
 * Internal declarations for mixguile_cmd_dispatcher_t
 * ------------------------------------------------------------------
 * Copyright (C) 2001, 2007 Free Software Foundation, Inc.
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


#ifndef XMIXGUILE_CMD_DISPATCHER_H
#define XMIXGUILE_CMD_DISPATCHER_H

#include <stdlib.h>
#include <stdio.h>

#include <libguile.h>
#include "mixguile_cmd_dispatcher.h"

/* the cmd dispatcher type */
struct mixguile_cmd_dispatcher_t
{
  mix_vm_cmd_dispatcher_t *dispatcher;
};

/* scm commands types */
/* prototype of a function implementing a new scm function */
typedef SCM (*scm_func_t) ();

/* record for a new scm command */
typedef struct scm_command_t
{
  gchar *name;			/* name of the scheme command */
  scm_func_t func;		/* implementation of the command */
  int argno;			/* no. of arguments */
  int opt_argno;		/* no. of optional arguments */
  int restp;			/* if 1, receive a list of remaining args */
} scm_command_t;

/* NULL-terminated list of available scm commands */
extern const scm_command_t DEFAULT_SCM_COMMANDS_[];

/* register a NULL-terminated list of scm commands */
extern void
register_scm_commands_ (const scm_command_t *commands);

/* register the mixvm cmd dispatcher to use with commands */
extern void
register_cmd_dispatcher_ (mixguile_cmd_dispatcher_t *dis);


#endif /* XMIXGUILE_CMD_DISPATCHER_H */

