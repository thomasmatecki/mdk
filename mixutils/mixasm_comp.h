/* -*-c-*- ---------------- mixasm_comp.h :
 * Declarations of functions used to compile mix source files.
 * ------------------------------------------------------------------
 * Copyright (C) 2000, 2007 Free Software Foundation, Inc.
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


#ifndef MIXASM_COMP_H
#define MIXASM_COMP_H

#include <glib.h>

extern int
mix_asm_compile(const gchar *src, const gchar *out, gboolean use_list,
		const gchar *list, gboolean debug);



#endif /* MIXASM_COMP_H */

