/* -*-c-*- ---------------- mix_src_file.h :
 * Declaration of mix_src_file_t, a type representing a MIXAL source
 * file.
 * ------------------------------------------------------------------
 * Copyright (C) 2000, 2007 Free Software Foundation, Inc.
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


#ifndef MIX_SRC_FILE_H
#define MIX_SRC_FILE_H

#include "mix_file.h"

/* the MIXAL source file type */
typedef struct mix_src_file_t mix_src_file_t;

/* create a new src file from an existing disk file */
extern mix_src_file_t *
mix_src_file_new_for_read (const gchar *path);

/* destroy a src file object */
extern void
mix_src_file_delete (mix_src_file_t *src);

/* get the source file path */
extern const gchar *
mix_src_file_get_path (const mix_src_file_t *src);

/* get a given line of the source file */
extern const gchar *
mix_src_file_get_line (const mix_src_file_t *src, guint lineno);

/* get the total no. of lines in the file */
extern guint
mix_src_file_get_line_no (const mix_src_file_t *src);



#endif /* MIX_SRC_FILE_H */

