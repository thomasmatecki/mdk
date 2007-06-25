/* -*-c-*- ---------------- mix_code_file.h :
 * Declaration of mix_code_file_t, a file containing compiled mix
 * instructions.
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


#ifndef MIX_CODE_FILE_H
#define MIX_CODE_FILE_H

#include "mix_file.h"
#include "mix_ins.h"
#include "mix_symbol_table.h"

/* mix_code_file_t type */
typedef struct mix_code_file_t mix_code_file_t;

/* instructions are loaded at specific memory addresses and related to
   source line numbers
*/
typedef struct mix_ins_desc_t mix_ins_desc_t;
struct mix_ins_desc_t
{
  mix_word_t ins;		/* a mix instruction coded into a word */
  mix_address_t address;	/* the address of this instruction */
  guint lineno;			/* source file line no. */
};

/* mix code files have a default extension (.mix) which is customizable */
extern const gchar *
mix_code_file_get_defext(void);

extern gboolean
mix_code_file_set_defext(const gchar *ext);

/* create/destroy code files for read or write */
/* if -name- does not end with defext, it is automatically appended */
extern mix_code_file_t *
mix_code_file_new_read(const gchar *name);

/* open a code file for write with/out debug information */
extern mix_code_file_t *
mix_code_file_new_write(const gchar *name, mix_address_t start,
			const gchar *source_path, gboolean debug,
			const mix_symbol_table_t *table);

extern void
mix_code_file_delete(mix_code_file_t *file);

/* get general parameters from a code file */
extern gboolean
mix_code_file_is_debug(const mix_code_file_t *file);

extern gint
mix_code_file_major_version(const mix_code_file_t *file);

extern gint
mix_code_file_minor_version(const mix_code_file_t *file);

extern mix_address_t
mix_code_file_get_start_addr(const mix_code_file_t *file);

extern mix_symbol_table_t *
mix_code_file_get_symbol_table(mix_code_file_t *file);

/* read instructions from a code file */
extern gboolean
mix_code_file_is_eof(mix_code_file_t *file);

extern gboolean
mix_code_file_get_ins(mix_code_file_t *file, mix_ins_desc_t *desc);

/* write instructions to a code file */
extern gboolean
mix_code_file_write_ins(mix_code_file_t *file, const mix_ins_desc_t *desc);

extern gboolean
mix_code_file_write_next_ins(mix_code_file_t *file, mix_word_t ins,
			     guint lineno);

extern gboolean
mix_code_file_set_address(mix_code_file_t *file, mix_address_t address);

/* get details about the source file */
extern const gchar *
mix_code_file_get_source_path (const mix_code_file_t *file);


#endif /* MIX_CODE_FILE_H */

