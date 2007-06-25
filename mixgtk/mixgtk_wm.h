/* -*-c-*- ---------------- mixgtk_wm.h :
 * Functions for window management
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


#ifndef MIXGTK_WM_H
#define MIXGTK_WM_H

#include "mixgtk.h"

typedef enum {
  MIXGTK_MIXVM_WINDOW,
  MIXGTK_MIXAL_WINDOW,
  MIXGTK_DEVICES_WINDOW
} mixgtk_window_id_t;

extern gboolean
mixgtk_wm_init (void);

extern void
mixgtk_wm_attach_window (mixgtk_window_id_t w);

extern void
mixgtk_wm_detach_window (mixgtk_window_id_t w);

#endif /* MIXGTK_WM_H */

