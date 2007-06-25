/* -*-c-*- -------------- mix_device.c :
 * Implementation of the functions declared in mix_device.h
 * ------------------------------------------------------------------
 * Copyright (C) 2000, 2001, 2007 Free Software Foundation, Inc.
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

#include "mix_file.h"
#include "xmix_device.h"

/*
  Set the directory for mix device files (by default, it's ".")
  If the dir does not exist, it is created.
*/
gboolean
mix_device_set_dir (const gchar *dirname)
{
  if (mix_stat_dir (dirname, "devices"))
    {
      if (DEV_DIR_) g_free (DEV_DIR_);
      DEV_DIR_ = g_strdup (dirname);
      return TRUE;
    }
  else
    return FALSE;
}

const gchar *
mix_device_get_dir (void)
{
  return DEV_DIR_;
}

mix_device_t *
mix_device_new (mix_device_type_t type)
{
  mix_device_t *result = NULL;
  g_return_val_if_fail (type < mix_dev_INVALID, NULL);
  result = g_new (mix_device_t, 1);
  construct_device_ (result, type);
  return result;
}



mix_device_t *
mix_device_new_with_name (mix_device_type_t type, const gchar *name)
{
  mix_device_t *result = NULL;
  g_return_val_if_fail (name != NULL, NULL);
  g_return_val_if_fail (type < mix_dev_INVALID, NULL);
  result = g_new (mix_device_t, 1);
  construct_device_with_name_ (result, type, name);
  return result;
}

/*
  Create a new device with a given type and stream
*/
mix_device_t *
mix_device_new_with_file (mix_device_type_t type, FILE *file)
{
  mix_device_t *result = NULL;
  g_return_val_if_fail (file != NULL, NULL);
  g_return_val_if_fail (type < mix_dev_INVALID, NULL);
  result = g_new (mix_device_t, 1);
  construct_device_with_file_ (result, type, file);
  return result;
}

void
mix_device_delete (mix_device_t *dev)
{
  if (dev != NULL)
    {
      (dev->vtable->destroy) (dev);
      g_free (dev);
    }
}

mix_device_type_t
mix_device_type (const mix_device_t *dev)
{
  g_return_val_if_fail (dev != NULL, mix_dev_INVALID);
  return dev->type;
}

const char *
mix_device_get_name (const mix_device_t *dev)
{
  g_return_val_if_fail (dev != NULL, NULL);
  return mix_file_base_name(GET_FILE_(dev));
}

/*
  Get the device block size
*/
size_t
mix_device_block_size (const mix_device_t *dev)
{
  g_return_val_if_fail (dev != NULL, 0);
  return SIZES_[dev->type];
}

/*
  Get the device io mode
*/
mix_device_mode_t
mix_device_mode (const mix_device_t *dev)
{
  g_return_val_if_fail (dev != NULL, 0);
  return MODES_[dev->type];
}

/*
  Write a block to the device.
*/
gboolean
mix_device_write (mix_device_t *dev, const mix_word_t *block)
{
  g_return_val_if_fail (dev != NULL, FALSE);
  g_return_val_if_fail (block != NULL, FALSE);
  g_assert (dev->vtable != NULL);
  return (dev->vtable->write) (dev, block);
}

gboolean
mix_device_read (mix_device_t *dev, mix_word_t *block)
{
  g_return_val_if_fail (dev != NULL, FALSE);
  g_return_val_if_fail (block != NULL, FALSE);
  g_assert (dev->vtable != NULL);
  return (dev->vtable->read) (dev, block);
}

gboolean
mix_device_ioc (mix_device_t *dev, mix_short_t arg)
{
  g_return_val_if_fail (dev != NULL, FALSE);
  g_assert (dev->vtable != NULL);
  return (dev->vtable->ioc) (dev, arg);
}

gboolean
mix_device_busy (const mix_device_t *dev)
{
  g_return_val_if_fail (dev != NULL, FALSE);
  return (dev->vtable->busy) (dev);

}
