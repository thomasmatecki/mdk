/* -*-c-*- ---------------- mixvm_loop.h :
 * Declarations for functions controlling the mixvm loop.
 * ------------------------------------------------------------------
 * Copyright (C) 2000, 2001, 2006, 2007 Free Software Foundation, Inc.
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


#ifndef MIXVM_LOOP_H
#define MIXVM_LOOP_H

extern void
mix_vmloop (int argc, char *argv[], gboolean initfile,
	    const gchar *code_file, gboolean use_emacs);

extern void
mix_vmrun (const gchar *code_file, gboolean dump, gboolean ptime);

extern void
mix_vmloop_set_prompt (const gchar *prompt);

#endif  /* MIXVM_LOOP_H */
