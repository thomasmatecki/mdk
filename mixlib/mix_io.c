/* -*-c-*- --------------- mix_io.c :
 * Implementation of the functions declared in mix_io.h
 * ------------------------------------------------------------------
 * Copyright (C) 2000, 2004, 2006, 2007 Free Software Foundation, Inc.
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


#include "xmix_io.h"

mix_iochannel_t *
mix_io_new (FILE *file)
{
  mix_iochannel_t *result;
  g_return_val_if_fail (file != NULL, NULL);
  result = g_new (mix_iochannel_t, 1);
  result->file = file;
  return result;
}

void
mix_io_delete (mix_iochannel_t *ch)
{
  if (ch != NULL)
    {
      fclose (ch->file);
      g_free (ch);
    }
}

FILE *
mix_io_to_FILE (mix_iochannel_t *ioc)
{
  if (ioc == NULL) return NULL;
  return ioc->file;
}

gboolean
mix_io_eof (mix_iochannel_t *ioc)
{
  if (ioc == NULL) return TRUE;
  return is_eof_ (ioc);
}

gboolean
mix_io_is_ready (mix_iochannel_t *ioc)
{
  if (ioc == NULL) return FALSE;
  return is_ready_ (ioc);
}

gboolean
mix_io_write_byte (mix_iochannel_t *ioc, mix_byte_t b)
{
  if (ioc == NULL) return FALSE;
  return write_data_ (ioc, &b, 1);
}

gboolean
mix_io_write_byte_array (mix_iochannel_t *ioc, const mix_byte_t *b,
			 size_t s)
{
  if (ioc == NULL || b == NULL) return FALSE;
  return write_data_ (ioc, b, s);
}

mix_byte_t
mix_io_read_byte (mix_iochannel_t *ioc)
{
  mix_byte_t result = MIX_BYTE_ZERO;
  if (ioc != NULL) (void)read_data_ (ioc, &result, 1);
  return result;
}

gboolean
mix_io_read_byte_array (mix_iochannel_t *ioc, mix_byte_t *b, size_t s)
{
  return (ioc != NULL) && (b != NULL) && read_data_ (ioc, b, s);
}

gboolean
mix_io_write_word (mix_iochannel_t *ioc, mix_word_t w)
{
  return (ioc != NULL) && write_data_ (ioc, &w, 1);
}

gboolean
mix_io_write_word_array (mix_iochannel_t *ioc, const mix_word_t *w,
			 size_t s)
{
  return (ioc != NULL) && (w != NULL) && write_data_ (ioc, w, s);
}

mix_word_t
mix_io_read_word (mix_iochannel_t *ioc)
{
  mix_word_t result = MIX_WORD_ZERO;
  if (ioc != NULL) (void)read_data_ (ioc, &result, 1);
  return result;
}

gboolean
mix_io_read_word_array (mix_iochannel_t *ioc, mix_word_t *w, size_t s)
{
  return (ioc != NULL) && (w != NULL) && read_data_ (ioc, w, s);
}

gboolean
mix_io_write_short (mix_iochannel_t *ioc, mix_short_t w)
{
  return (ioc != NULL) && write_data_ (ioc, &w, 1);
}

gboolean
mix_io_write_short_array (mix_iochannel_t *ioc, const mix_short_t *w,
			  size_t s)
{
  return (ioc != NULL) && (w != NULL) && write_data_ (ioc, w, s);
}

mix_short_t
mix_io_read_short (mix_iochannel_t *ioc)
{
  mix_short_t result = MIX_SHORT_ZERO;
  if (ioc != NULL) (void)read_data_ (ioc, &result, 1);
  return result;
}

gboolean
mix_io_read_short_array (mix_iochannel_t *ioc, mix_short_t *w, size_t s)
{
  return (ioc != NULL) && (w != NULL) && read_data_ (ioc, w, s);
}

gboolean
mix_io_write_char (mix_iochannel_t *ioc, mix_char_t c)
{
  guchar value = mix_char_to_ascii (c);
  return (ioc != NULL) && write_data_ (ioc, &value, 1);
}

mix_char_t
mix_io_read_char (mix_iochannel_t *ioc)
{
  guchar value = MIX_CHAR_MAX;
  if (ioc != NULL) (void)read_data_ (ioc, &value, 1);
  return mix_ascii_to_char (value);
}

gboolean
mix_io_write_word_array_as_char  (mix_iochannel_t *ioc,
				  const mix_word_t *w, size_t s)
{
  guint k, j;
  guchar value;

  if ((ioc == NULL) || (w == NULL)) return FALSE;

  for (k = 0; k < s; k++)
    for (j = 1; j < 6; j++)
      {
	mix_char_t ch = mix_byte_to_char (mix_word_get_byte (w[k], j));
	value = mix_char_to_ascii (ch);
	if (!write_data_ (ioc, &value, 1)) return FALSE;
      }
  value = '\n';
  return write_data_ (ioc, &value, 1);
}

gboolean
mix_io_read_word_array_as_char (mix_iochannel_t *ioc,
				mix_word_t *w, size_t s)
{
  guint k, j;
  guchar value;
  gboolean eol = FALSE;
  mix_char_t spc = mix_ascii_to_char (' ');

  if ((ioc == NULL) || (w == NULL)) return FALSE;

  for (k = 0; k < s && !eol; k++)
    for (j = 1; j < 6; j++)
      {
	if (!eol && !read_data_ (ioc, &value, 1)) return FALSE;
        eol = eol || (value == '\n') || is_eof_ (ioc);
	mix_word_set_byte (&w[k], j, eol? spc : mix_ascii_to_char (value));
      }
  for (; k < s; ++k) w[k] = MIX_WORD_ZERO;

  while (!eol && !is_eof_ (ioc) && value != '\n')
    if (!read_data_ (ioc, &value, 1)) return FALSE;

  return TRUE;
}
