/* -*-c-*- ---------------- mix_file.h :
 * Declarations for the mix_file_t type.
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


#ifndef MIX_FILE_H
#define MIX_FILE_H

#include "mix_io.h"

/* The mix_file_t type, deriving from mix_iochannel_t and
   representing a disk file
*/
typedef struct mix_file_t mix_file_t;

/* Creation/destruction of files */
extern mix_file_t *
mix_file_new(const gchar *name, mix_fmode_t mode);

/* creates a file adding to its name the defext if missing */
extern mix_file_t *
mix_file_new_with_def_ext(const gchar *name, mix_fmode_t mode,
			  const gchar *defext);

extern void
mix_file_delete(mix_file_t *file);

/* convert to a standard FILE */
extern FILE *
mix_file_to_FILE(const mix_file_t *file);

/* standard default extensions */
extern const gchar *MIX_SRC_DEFEXT, *MIX_LIST_DEFEXT, *MIX_CODE_DEFEXT;

/* complete a name with an extension, if needed */
extern gchar *
mix_file_complete_name (const gchar *name, const gchar *extension);

/* Get the base name and extension of file */
extern const gchar *
mix_file_base_name(const mix_file_t *file);

extern const gchar *
mix_file_extension(const mix_file_t *file);


#endif /* MIX_FILE_H */

