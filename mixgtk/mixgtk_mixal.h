/* -*-c-*- ---------------- mixgtk_mixal.h :
 * Declarations for functions displaying the mixal source file
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


#ifndef MIXGTK_MIXAL_H
#define MIXGTK_MIXAL_H

#include <gtk/gtk.h>
#include <mixlib/mix_vm.h>
#include "mixgtk_widgets.h"


/* initialise the mixal widget */
extern GtkWidget *
mixgtk_mixal_init (mix_vm_t *vm);

extern void
mixgtk_mixal_reparent (GtkStatusbar *stat);

/* load the corresponding mixal file */
extern void
mixgtk_mixal_load_file (void);

/* update the widget */
extern void
mixgtk_mixal_update (void);

extern void
mixgtk_mixal_update_fonts (void);

extern void
mixgtk_mixal_pop_status (void);

/* breakpoints */
extern void
mixgtk_mixal_update_bp_at_address (guint addr);

extern void
mixgtk_mixal_update_bp_at_line (guint line);

extern void
mixgtk_mixal_update_bp_all (void);


#endif /* MIXGTK_MIXAL_H */

