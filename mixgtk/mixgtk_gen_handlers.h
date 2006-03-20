/* -*-c-*- ---------------- mixgtk_gen_handlers.h :
 * general signal handlers declarations
 * ------------------------------------------------------------------
 *  $Id: mixgtk_gen_handlers.h,v 1.7 2005/09/20 19:43:14 jao Exp $
 * ------------------------------------------------------------------
 * Copyright (C) 2001, 2004 Free Software Foundation, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
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


#ifndef MIXGTK_GEN_HANDLERS_H
#define MIXGTK_GEN_HANDLERS_H

#include <gtk/gtk.h>

extern void
on_main_window_destroy (GtkWidget *w, gpointer data);

extern void
on_file_open_activate (GtkWidget *w, gpointer data);

extern void
on_file_exit_activate (GtkWidget *w, gpointer data);

extern void
on_file_edit_activate (GtkWidget *w, gpointer data);

extern void
on_file_compile_activate (GtkWidget *w, gpointer data);

extern void
on_clear_breakpoints_activate (GtkWidget *w, gpointer data);

extern void
on_save_on_exit_toggle (GtkWidget *w, gpointer data);

extern void
on_save_activate (GtkWidget *w, gpointer data);

typedef void (*file_callback_t)(const gchar *file);

extern void
mixgtk_get_file (file_callback_t callback, const gchar *title,
		 const gchar *pattern, const gchar *def_file);


#endif /* MIXGTK_GEN_HANDLERS_H */

