/* -*-c-*- ---------------- mixgtk_device.h :
 * Block devices used by mixgtk
 * ------------------------------------------------------------------
 * Copyright (C) 2001, 2004, 2007 Free Software Foundation, Inc.
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


#ifndef MIXGTK_DEVICE_H
#define MIXGTK_DEVICE_H

#include <gtk/gtk.h>
#include <mixlib/mix_vm.h>
#include <mixlib/mix_device.h>

/* init default devices */
extern GtkWidget *
mixgtk_device_init (mix_vm_t *vm);

extern void
mixgtk_device_update_fonts (void);

/* change the output format of a device */
extern void
mixgtk_device_set_format (mix_device_type_t dev, gboolean dec);


#endif /* MIXGTK_DEVICE_H */

