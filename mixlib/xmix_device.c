/* -*-c-*- -------------- xmix_device.c :
 * Implementation of the functions declared in xmix_device.h
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


#include "xmix_device.h"
#include "mix_types.h"

gchar *DEV_DIR_ = NULL;

const char *DEV_EXT_ = ".dev";

const char *DEF_NAMES_[] = {
  "tape0", "tape1", "tape2", "tape3", "tape4", "tape5", "tape6", "tape7",
  "disk0", "disk1", "disk2", "disk3", "disk4", "disk5", "disk6", "disk7",
  "cardrd", "cardwr", "printer", "console", "paper"
};

const size_t SIZES_[] = {
  100, 100, 100, 100, 100, 100, 100, 100,
  100, 100, 100, 100, 100, 100, 100, 100,
  16, 16, 24, 14, 14
};

const mix_device_mode_t MODES_[] = {
  mix_dev_BIN, mix_dev_BIN, mix_dev_BIN, mix_dev_BIN, mix_dev_BIN,
  mix_dev_BIN, mix_dev_BIN, mix_dev_BIN, mix_dev_BIN, mix_dev_BIN,
  mix_dev_BIN, mix_dev_BIN, mix_dev_BIN, mix_dev_BIN, mix_dev_BIN,
  mix_dev_BIN, mix_dev_CHAR, mix_dev_CHAR, mix_dev_CHAR, mix_dev_CHAR,
  mix_dev_CHAR
};

const mix_fmode_t FMODES_[] = {
  mix_io_RDWRT,  mix_io_RDWRT,  mix_io_RDWRT,  mix_io_RDWRT,
  mix_io_RDWRT,  mix_io_RDWRT,  mix_io_RDWRT,  mix_io_RDWRT,
  mix_io_RDWRT,  mix_io_RDWRT,  mix_io_RDWRT,  mix_io_RDWRT,
  mix_io_RDWRT,  mix_io_RDWRT,  mix_io_RDWRT,  mix_io_RDWRT,
  mix_io_READ, mix_io_WRITE, mix_io_WRITE, mix_io_RDWRT, mix_io_READ
};

/* constructors */
void
construct_device_ (mix_device_t *result, mix_device_type_t type)
{
  gchar *name;
  name = DEV_DIR_ ? g_strdup_printf ("%s/%s", DEV_DIR_, DEF_NAMES_[type])
    : g_strdup (DEF_NAMES_[type]);
  construct_device_with_name_ (result, type, name);
  g_free (name);
}


void
construct_device_with_name_ (mix_device_t *result,
			     mix_device_type_t type, const gchar *name)
{
  result->type = type;
  if (type != mix_dev_CONSOLE)
    {
      result->file = MIX_IOCHANNEL(mix_file_new_with_def_ext (name,
							      FMODES_[type],
							      DEV_EXT_));
      result->vtable = DEF_DEV_VTABLE_;
    }
  else
    {
      result->file = mix_io_new (stdout);
      result->vtable = CONSOLE_DEV_VTABLE_;
    }
}


void
construct_device_with_file_ (mix_device_t *result,
			     mix_device_type_t type, FILE *file)
{
  result->type = type;
  result->file = mix_io_new (file);
  result->vtable = DEF_DEV_VTABLE_;
}


/*
  Write a block to the device.
*/
static gboolean
write_ (mix_device_t *dev, const mix_word_t *block)
{
  gboolean result;

  if (FMODES_[dev->type] == mix_io_READ) return FALSE;
  if (MODES_[dev->type] == mix_dev_CHAR)
    result = mix_io_write_word_array_as_char (GET_CHANNEL_ (dev),
					      block, SIZES_[dev->type]);
  else
    result = mix_io_write_word_array (GET_CHANNEL_ (dev),
				      block, SIZES_[dev->type]);
  fflush (mix_io_to_FILE (GET_CHANNEL_ (dev)));

  return result;
}

static gboolean
read_cons_ (mix_device_t *dev, mix_word_t *block)
{
  return mix_io_read_word_array_as_char (mix_io_new (stdin), block,
                                         SIZES_[mix_dev_CONSOLE]);
}

static gboolean
read_ (mix_device_t *dev, mix_word_t *block)
{
  gboolean result;

  if (FMODES_[dev->type] == mix_io_WRITE) return FALSE;
  if (MODES_[dev->type] == mix_dev_CHAR)
    {
      result = mix_io_read_word_array_as_char (GET_CHANNEL_ (dev),
                                               block, SIZES_[dev->type]);
    }
  else
    result = mix_io_read_word_array (GET_CHANNEL_ (dev),
                                     block, SIZES_[dev->type]);

  return result;
}

static gboolean
ioc_ (mix_device_t *dev, mix_short_t arg)
{
  int m;
  FILE *file;

  m = mix_short_magnitude(arg);
  if (mix_short_is_negative(arg)) m = -m;
  m *= sizeof (mix_word_t) * SIZES_[dev->type];
  file = mix_io_to_FILE (GET_CHANNEL_(dev));

  if (dev->type >= mix_dev_TAPE_0 && dev->type <= mix_dev_TAPE_7)
    {
      if (m == 0) rewind (file);
      else fseek (file, m, SEEK_CUR);
    }
  if (dev->type >= mix_dev_DISK_0 && dev->type <= mix_dev_DISK_7)
    {
      if (m == 0) return FALSE;
      // position disk
    }
  if (dev->type == mix_dev_PAPER_TAPE)
    {
      if (m == 0) return FALSE;
      rewind (file);
    }
  return TRUE;
}

static gboolean
busy_ (const mix_device_t *dev)
{
  return (!mix_io_is_ready (GET_CHANNEL_(dev)));
}

static void
destroy_ (mix_device_t *dev)
{
  if (dev->type != mix_dev_CONSOLE && GET_FILE_(dev) != NULL)
    mix_file_delete (GET_FILE_(dev));
}

static mix_device_vtable_t VTABLE_ = {
  write_, read_, ioc_, busy_, destroy_
};

const mix_device_vtable_t * DEF_DEV_VTABLE_ = &VTABLE_;

static mix_device_vtable_t CVTABLE_ = {
  write_, read_cons_, ioc_, busy_, destroy_
};

const mix_device_vtable_t * CONSOLE_DEV_VTABLE_ = &CVTABLE_;
