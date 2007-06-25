/* -*-c-*- ---------------- mixguile_cmd_dispatcher.h :
 * Command dispatcher with guile support
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


#ifndef MIXGUILE_CMD_DISPATCHER_H
#define MIXGUILE_CMD_DISPATCHER_H

#include <mixlib/mix.h>
#include <mixlib/mix_vm_command.h>

/* the guile command dispatcher type */
typedef struct mixguile_cmd_dispatcher_t mixguile_cmd_dispatcher_t;

/* create/destroy cmd dispatcher */
extern mixguile_cmd_dispatcher_t *
mixguile_cmd_dispatcher_new (mix_vm_cmd_dispatcher_t *dis);

extern void
mixguile_cmd_dispatcher_delete (mixguile_cmd_dispatcher_t *dis);

/* get the underlying vm dispatcher */
extern mix_vm_cmd_dispatcher_t *
mixguile_cmd_dispatcher_get_vm_dispatcher (const
					   mixguile_cmd_dispatcher_t *disp);

/* interpret commands from file or string */
extern void
mixguile_cmd_dispatcher_interpret_file (mixguile_cmd_dispatcher_t *dis,
					const gchar *path);

extern void
mixguile_cmd_dispatcher_interpret_command (mixguile_cmd_dispatcher_t *dis,
					   const gchar *command);


#endif /* MIXGUILE_CMD_DISPATCHER_H */

