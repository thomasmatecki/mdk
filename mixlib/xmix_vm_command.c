/* -*-c-*- -------------- xmix_vm_command.c :
 * Implementation of the functions declared in xmix_vm_command.h
 * ------------------------------------------------------------------
 * Copyright (C) 2001, 2002, 2006, 2007 Free Software Foundation, Inc.
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

#include <stdarg.h>
#include <stdio.h>

#include "xmix_vm_command.h"

/* configuration keys */
const gchar *TRACING_KEY_ = "Tracing";
const gchar *TIMING_KEY_ = "Timing";
const gchar *EDITOR_KEY_ = "Editor";
const gchar *ASM_KEY_ = "Assembler";
const gchar *LOGGING_KEY_ = "Logs";

void
log_message_ (mix_vm_cmd_dispatcher_t *dis, const gchar *fmt, ...)
{
  if (dis && fmt && dis->log_msg && dis->out)
    {
      va_list args;
      va_start (args, fmt);
      vfprintf (dis->out, fmt, args);
      fprintf (dis->out, "\n");
      va_end (args);
    }
}


extern void
log_error_ (mix_vm_cmd_dispatcher_t *dis, const gchar *fmt, ...)
{
  enum {BUFF_SIZE = 256};
  static gchar BUFFER[256];

  if (dis && fmt && dis->err)
    {
      va_list args;
      va_start (args, fmt);
      g_snprintf (BUFFER, BUFF_SIZE, "ERROR: %s\n", fmt);
      vfprintf (dis->err, BUFFER, args);
      va_end (args);
    }
}
