/* -*-c-*- -------------- mix_file.c :
 * Implementation of the functions declared in mix_file.h
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

#include <string.h>

#include "xmix_io.h"
#include "mix_file.h"

const gchar *MIX_SRC_DEFEXT = ".mixal",
  *MIX_LIST_DEFEXT = ".mls", *MIX_CODE_DEFEXT = ".mix";

/* file names completions */
#define needs_completion_(name,defext) \
  ( strcmp(name + strlen(name) - strlen(defext), defext) != 0 )

#define add_completion_(name, defext) \
  g_strconcat(name, defext, NULL)

/* The actual definition of mix_file_t */
struct mix_file_t
{
  mix_iochannel_t parent;
  gchar *base_name;
  gchar *ext;
};

/* Creation/destruction of files */
static mix_file_t *
open_file_(const gchar *name, mix_fmode_t mode)
{
  mix_file_t *result;
  FILE *file;
  const gchar *fmode = fmode_to_type_ (mode);

  /* if the read/write file already exists, open in r+ mode */
  if (mode == mix_io_RDWRT && (file = fopen (name, "r")))
    {
      fmode = "r+";
      fclose (file);
    }

  result = g_new(mix_file_t, 1);
  file = fopen(name, fmode);
  if ( file == NULL ) {
    g_free (result);
    return NULL;
  }
  io_init_from_file_(MIX_IOCHANNEL(result), file);
  return result;
}

mix_file_t *
mix_file_new(const gchar *name, mix_fmode_t mode)
{
  mix_file_t *result;
  gchar *bname;

  if ( name == NULL ) return NULL;
  bname = g_strdup(name);
  if ( bname == NULL ) return NULL;
  result = open_file_(name, mode);
  if ( result == NULL )
    {
      g_free(bname);
      return NULL;
    }
  result->base_name = bname;
  result->ext = NULL;
  return result;
}

/* creates a file adding to its name the defext if missing */
mix_file_t *
mix_file_new_with_def_ext(const gchar *name, mix_fmode_t mode,
			  const gchar *defext)
{
  const gchar *real_name;
  mix_file_t *result;

  if ( name == NULL ) return NULL;
  if ( defext == NULL ) return mix_file_new(name, mode);
  real_name =  needs_completion_(name, defext) ?
    add_completion_(name, defext) : name;
  result = open_file_(real_name, mode);
  if ( real_name != name ) g_free((void *)real_name);
  if ( result  == NULL ) return NULL;
  result->ext = g_strdup(defext);
  if ( needs_completion_(name, defext) )
    result->base_name = g_strdup(name);
  else
    result->base_name = g_strndup(name, strlen(name) - strlen(defext));
  if ( result->ext == NULL || result->base_name == NULL )
    {
      mix_file_delete(result);
      return NULL;
    }
  return result;
}

void
mix_file_delete(mix_file_t *file)
{
  g_return_if_fail(file != NULL);
  io_close_(MIX_IOCHANNEL(file));
  if (file->base_name) g_free(file->base_name);
  if (file->ext) g_free(file->ext);
  g_free(file);
}

/* convert to a standard FILE */
extern FILE *
mix_file_to_FILE(const mix_file_t *file)
{
  if ( file == NULL ) return NULL;
  return io_get_FILE_(file);
}

/* complete a name with an extension, if needed */
gchar *
mix_file_complete_name (const gchar *name, const gchar *extension)
{
  if (!name) return NULL;
  if (!extension || !needs_completion_ (name, extension))
    return g_strdup (name);
  return add_completion_ (name, extension);
}

/* Get the base name and extension of file */
const gchar *
mix_file_base_name(const mix_file_t *file)
{
  g_return_val_if_fail(file != NULL, NULL);
  return file->base_name;
}

const gchar *
mix_file_extension(const mix_file_t *file)
{
  g_return_val_if_fail(file != NULL, NULL);
  return file->ext;
}
