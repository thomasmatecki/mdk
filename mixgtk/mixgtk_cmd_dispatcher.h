/* -*-c-*- ---------------- mixgtk_cmd_dispatcher.h :
 * functions to access the command dispatcher
 * ------------------------------------------------------------------
 * Copyright (C) 2001, 2004, 2006, 2007 Free Software Foundation, Inc.
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


#ifndef MIXGTK_CMD_DISPATCHER_H
#define MIXGTK_CMD_DISPATCHER_H

#include <mixlib/mix_vm.h>
#include <mixlib/mix_vm_command.h>
#include "mixgtk_widgets.h"

/* initialise the command dispatcher */
extern gboolean
mixgtk_cmd_dispatcher_init (mixgtk_dialog_id_t top);

extern void
mixgtk_cmd_dispatcher_update_fonts (void);

/* dispatch an externally provided command */
extern void
mixgtk_cmd_dispatcher_dispatch (const gchar *command);

/* get times */
extern void
mixgtk_cmd_dispatcher_get_times (gint *uptime, gint *progtime, gint *laptime);

/* get the underlying vm */
extern mix_vm_t *
mixgtk_cmd_dispatcher_get_vm (void);

/* get the current source file */
extern const gchar *
mixgtk_cmd_dispatcher_get_src_path (void);

/* get the mix cmd dispatcher */
extern mix_vm_cmd_dispatcher_t *
mixgtk_cmd_dispatcher_get_mix_dispatcher (void);


#endif /* MIXGTK_CMD_DISPATCHER_H */

