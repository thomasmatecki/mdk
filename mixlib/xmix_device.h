/* -*-c-*- ---------------- xmix_device.h :
 * Protected declarations for mix_device_t
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


#ifndef XMIX_DEVICE_H
#define XMIX_DEVICE_H

#include "mix_file.h"
#include "mix_device.h"

/* device file directory */
extern gchar *DEV_DIR_;

/* table of overridable device operations */
typedef gboolean (*mix_dev_write_func_t) (mix_device_t *, const mix_word_t *);
typedef gboolean (*mix_dev_read_func_t) (mix_device_t *, mix_word_t *);
typedef gboolean (*mix_dev_ioc_func_t) (mix_device_t *, mix_short_t);
typedef gboolean (*mix_dev_busy_func_t) (const mix_device_t *);
typedef void (*mix_dev_destroy_t) (mix_device_t *);

typedef struct mix_device_vtable_t
{
  mix_dev_write_func_t write;
  mix_dev_read_func_t  read;
  mix_dev_ioc_func_t   ioc;
  mix_dev_busy_func_t  busy;
  mix_dev_destroy_t destroy;
} mix_device_vtable_t;

/* default vtables */
extern const mix_device_vtable_t *DEF_DEV_VTABLE_;
extern const mix_device_vtable_t *CONSOLE_DEV_VTABLE_;

/*
  Actual definition of a mix device, which can be cast to
  a mix file.
*/
struct mix_device_t
{
  mix_iochannel_t *file;
  mix_device_type_t type;
  const mix_device_vtable_t *vtable;
};

/* constructors */
extern void
construct_device_ (mix_device_t *dev, mix_device_type_t type);

extern void
construct_device_with_name_ (mix_device_t *dev,
			     mix_device_type_t type, const gchar *name);

extern void
construct_device_with_file_ (mix_device_t *dev,
			     mix_device_type_t type, FILE *file);


#define GET_CHANNEL_(dev) (dev->file)
#define GET_FILE_(dev) ((mix_file_t *)(dev->file))

/* default extension for device files */
extern const char *DEV_EXT_;
/* default names for device files */
extern const char *DEF_NAMES_[];
/* block sizes for devices */
extern const size_t SIZES_[];
/* io modes for devices */
extern const mix_device_mode_t MODES_[];
/* files modes for devices */
extern const mix_fmode_t FMODES_[];




#endif /* XMIX_DEVICE_H */

