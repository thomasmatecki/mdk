/* -*-c-*- ---------------- mixgtk.h :
 * Gmixvm initialisation and cleanup functions
 * ------------------------------------------------------------------
 *  Last change: Time-stamp: <2001-04-29 12:06:48 jao>
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


#ifndef MIXGTK_H
#define MIXGTK_H

#include <gtk/gtk.h>
#include <mixlib/mix.h>

/* the app files directory */
#define MIXGTK_FILES_DIR  ".mdk"

/* initialise the app */
extern gboolean
mixgtk_init (int argc, char *argv[]);

/* enter the main mixgtk loop */
extern void
mixgtk_main (void);

/* clean up */
extern void
mixgtk_release (void);



#endif /* MIXGTK_H */

