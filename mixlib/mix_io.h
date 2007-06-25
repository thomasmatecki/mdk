/* -*-c-*- ------------------ mix_io.h :
 * Declarations for mix_iochannel_t and mix_file_t
 * ------------------------------------------------------------------
 * Copyright (C) 2000, 2004, 2007 Free Software Foundation, Inc.
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


#ifndef MIX_IOCHANNEL_H
#define MIX_IOCHANNEL_H

#include <stdio.h>
#include "mix_types.h"

/* mix_iochannel_t: an object for input/output of mix types */
typedef struct mix_iochannel_t mix_iochannel_t;

/* Cast to mix_iochannel_t */
#define MIX_IOCHANNEL(file)  (mix_iochannel_t *)(file)

/* I/O channels can be created in different modes: */
typedef enum {
  mix_io_READ,			/* read existing file */
  mix_io_WRITE,			/* write new file */
  mix_io_RDWRT,			/* read/write existing from beginning */
  mix_io_APPEND,		/* append to existing or new file */
  mix_io_RAPPEND		/* read from beginning, append to end */
} mix_fmode_t;

/* Create from a file handle */
extern mix_iochannel_t *
mix_io_new (FILE *file);

/* Delete */
extern void
mix_io_delete (mix_iochannel_t *ch);

/* Convert to a FILE * */
extern FILE *
mix_io_to_FILE (mix_iochannel_t *ioc);

/* Read/write from/to an iochannel */
extern gboolean
mix_io_eof (mix_iochannel_t *ioc);

extern gboolean
mix_io_is_ready (mix_iochannel_t *ioc);

extern gboolean
mix_io_write_byte (mix_iochannel_t *ioc, mix_byte_t b);

extern gboolean
mix_io_write_byte_array (mix_iochannel_t *ioc, const mix_byte_t *b, size_t s);


extern mix_byte_t
mix_io_read_byte (mix_iochannel_t *ioc);

extern gboolean
mix_io_read_byte_array (mix_iochannel_t *ioc, mix_byte_t *b, size_t s);


extern gboolean
mix_io_write_word (mix_iochannel_t *ioc, mix_word_t w);


extern gboolean
mix_io_write_word_array (mix_iochannel_t *ioc, const mix_word_t *w, size_t s);

extern mix_word_t
mix_io_read_word (mix_iochannel_t *ioc);

extern gboolean
mix_io_read_word_array (mix_iochannel_t *ioc, mix_word_t *w, size_t s);

extern gboolean
mix_io_write_short (mix_iochannel_t *ioc, mix_short_t w);


extern gboolean
mix_io_write_short_array (mix_iochannel_t *ioc, const mix_short_t *w, size_t s);

extern mix_short_t
mix_io_read_short (mix_iochannel_t *ioc);

extern gboolean
mix_io_read_short_array (mix_iochannel_t *ioc, mix_short_t *w, size_t s);

extern gboolean
mix_io_write_char (mix_iochannel_t *ioc, mix_char_t c);

extern mix_char_t
mix_io_read_char (mix_iochannel_t *ioc);

extern gboolean
mix_io_write_word_array_as_char (mix_iochannel_t *ioc,
                                 const mix_word_t *w, size_t s);

extern gboolean
mix_io_read_word_array_as_char (mix_iochannel_t *ioc,
				mix_word_t *w, size_t s);


#endif /* MIX_IOCHANNEL_H */

