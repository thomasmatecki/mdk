/* -*-c-*- -------------- mix_src_file.c :
 * Implementation of the functions declared in mix_src_file.h
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

#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include "mix_src_file.h"

/* the MIXAL source file type */
struct mix_src_file_t
{
  gchar *path;			/* the path to the disk file */
  GPtrArray *lines;		/* an array of the file lines */
  guint lineno;			/* the number of lines */
};

/* format a source line */
static gchar *
format_line_ (gchar *line)
{
  const gchar *label, *op, *rest;
  gint k = 0;

  if (!line) return line;
  if (line[0] == '*' || strlen(line) == 0)
    return g_strdup (line);

  if (isspace (line[0]))
    {
      label = " ";
      while (line[k] && isspace (line[k])) ++k;
    }
  else
    {
      label = line;
      while (line[k] && !isspace (line[k])) ++k;
      while (line[k] && isspace (line[k])) ++k;
    }

  if (line[k])
    {
      line[k - 1] = 0;
      op = line + k;
      while (line[k] && !isspace (line[k])) ++k;
      while (line[k] && isspace (line[k])) ++k;
      line[k - 1] = 0;
      rest = (line[k]) ? line + k: "";
    }
  else
    {
      op = rest = "";
    }


  return g_strdup_printf ("%-11s %-5s %s", label, op, rest);
}

/* load the source file lines into memory */
static gboolean
load_file_ (mix_src_file_t *file)
{
  mix_file_t *mf = mix_file_new_with_def_ext (file->path,
					      mix_io_READ,
					      MIX_SRC_DEFEXT);
  if (mf != NULL)
    {
      enum {BUFFER_SIZE = 256};
      static gchar BUFFER[BUFFER_SIZE];

      FILE  *f = mix_file_to_FILE (mf);
      file->lines = g_ptr_array_new ();
      file->lineno = 0;

      while (fgets (BUFFER, BUFFER_SIZE, f) == BUFFER)
	{
	  g_ptr_array_add (file->lines, (gpointer) format_line_ (BUFFER));
	  file->lineno++;
	}

      mix_file_delete (mf);
      return TRUE;
    }
  return FALSE;
}

/* create a new src file from an existing disk file */
mix_src_file_t *
mix_src_file_new_for_read (const gchar *path)
{
  mix_src_file_t *result = g_new (mix_src_file_t, 1);
  result->lines = NULL;
  result->path = g_strdup (path);
  result->lineno = 0;
  return result;
}

/* destroy a src file object */
void
mix_src_file_delete (mix_src_file_t *src)
{
  g_return_if_fail (src != NULL);
  if (src->lines) g_ptr_array_free (src->lines, TRUE);
  g_free (src->path);
  g_free (src);
}

/* get the source file path */
const gchar *
mix_src_file_get_path (const mix_src_file_t *src)
{
  g_return_val_if_fail (src != NULL, NULL);
  return src->path;
}

/* get a given line of the source file */
const gchar *
mix_src_file_get_line (const mix_src_file_t *src, guint lineno)
{
  g_return_val_if_fail (src != NULL, NULL);
  if (src->lines == NULL && !load_file_ ((mix_src_file_t*)src))
    return NULL;
  if (lineno > src->lineno || lineno == 0)
    return NULL;
  return (gchar *)g_ptr_array_index (src->lines, lineno - 1);
}

/* get the total no. of lines in the file */
guint
mix_src_file_get_line_no (const mix_src_file_t *src)
{
  g_return_val_if_fail (src != NULL, 0);
  if (src->lines == NULL && !load_file_ ((mix_src_file_t*)src))
    return 0;
  return src->lineno;
}

