/* -*-c-*- ---------------- mix_device.h :
 * Declaration of mix_device_t and associated methods.
 * ------------------------------------------------------------------
 * Copyright (C) 2000, 2001, 2004, 2007 Free Software Foundation, Inc.
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


#ifndef MIX_DEVICE_H
#define MIX_DEVICE_H

#include <stddef.h>
#include <stdio.h>
#include "mix.h"
#include "mix_types.h"

/*
  A mix device, which derives from mix_io_channel_t
*/
typedef struct mix_device_t mix_device_t;

/*
  The device type
*/
typedef enum {
  mix_dev_TAPE_0,
  mix_dev_TAPE_1,
  mix_dev_TAPE_2,
  mix_dev_TAPE_3,
  mix_dev_TAPE_4,
  mix_dev_TAPE_5,
  mix_dev_TAPE_6,
  mix_dev_TAPE_7,
  mix_dev_DISK_0,
  mix_dev_DISK_1,
  mix_dev_DISK_2,
  mix_dev_DISK_3,
  mix_dev_DISK_4,
  mix_dev_DISK_5,
  mix_dev_DISK_6,
  mix_dev_DISK_7,
  mix_dev_CARD_RD,
  mix_dev_CARD_WR,
  mix_dev_PRINTER,
  mix_dev_CONSOLE,
  mix_dev_PAPER_TAPE,
  mix_dev_INVALID
} mix_device_type_t;

/*
  The device io mode
*/
typedef enum {
  mix_dev_BIN,
  mix_dev_CHAR
} mix_device_mode_t;

/*
  Set the directory for mix device files (by default, it's ".")
  If the dir does not exist, it is created.
*/
extern gboolean
mix_device_set_dir (const gchar *dirname);

extern const gchar *
mix_device_get_dir (void);

/*
  Create a new device with default name and given type.
*/
extern mix_device_t *
mix_device_new (mix_device_type_t type);

/*
  Create a new device with a given type and name.
*/
extern mix_device_t *
mix_device_new_with_name (mix_device_type_t type, const gchar *name);

/*
  Create a new device with a given type and stream
*/
extern mix_device_t *
mix_device_new_with_file (mix_device_type_t type, FILE *file);

/*
  Delete a device.
*/
extern void
mix_device_delete (mix_device_t *dev);

/*
  Get a device type
*/
extern mix_device_type_t
mix_device_type (const mix_device_t *dev);


/*
  Get a device name
*/
extern const char *
mix_device_get_name (const mix_device_t *dev);

/*
  Get the device block size
*/
extern size_t
mix_device_block_size (const mix_device_t *dev);

/*
  Get the device io mode
*/
extern mix_device_mode_t
mix_device_mode (const mix_device_t *dev);

/*
  Write a block to the device.
*/
extern gboolean
mix_device_write (mix_device_t *dev, const mix_word_t *block);

/*
  Read a block from the device.
*/
extern gboolean
mix_device_read (mix_device_t *dev, mix_word_t *block);

/*
  Perform an io control operation on the device.
  The parameter _arg_ is the operation's argument:
   0- rewind to beginning
   <0 - rewind the given number of blocks
   >0 - skip forward the given number of blocks
*/
extern gboolean
mix_device_ioc (mix_device_t *dev, mix_short_t arg);

/*
  Check if a device is busy
*/
extern gboolean
mix_device_busy (const mix_device_t *dev);

#endif /* MIX_DEVICE_H */

