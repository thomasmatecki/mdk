/* -*-c-*- ---------------- mix_parser.h :
 * Declarations for mix_parser_t, which compiles a source file into
 * a mix code file.
 * ------------------------------------------------------------------
 * Copyright (C) 2000, 2003, 2004, 2007 Free Software Foundation, Inc.
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


#ifndef MIX_PARSER_H
#define MIX_PARSER_H

#include <glib.h>
#include "mix_vm.h"

/* The parser type */
typedef struct mix_parser_t mix_parser_t;

/* Create/destroy a mix_parser */
extern mix_parser_t *
mix_parser_new (const gchar *in_file);

extern void
mix_parser_delete (mix_parser_t *parser);

/* Access source file name */
extern const gchar *
mix_parser_src_file_base_name (const mix_parser_t *parser);

extern const gchar *
mix_parser_src_file_extension (const mix_parser_t *parser);

/* Compile a mix source file */
/* compilation errors */
typedef enum {
  MIX_PERR_OK,		/* no error */
  MIX_PERR_NOCOMP,	/* file not yet compiled */
  MIX_PERR_INTERNAL,	/* internal error */
  MIX_PERR_NOIN,	/* unable to open input file */
  MIX_PERR_NOOUT,	/* unable to open output file */
  MIX_PERR_UNEX_EOF,	/* unexpected end of file */
  MIX_PERR_INV_LOC,	/* invalid loc field */
  MIX_PERR_DUP_SYMBOL,	/* duplicated symbol */
  MIX_PERR_LONG_SYMBOL,	/* symbol name too long */
  MIX_PERR_NOOP,	/* missing op field */
  MIX_PERR_UNEX_LOC,	/* unexpected location symbol */
  MIX_PERR_INV_ADDRESS, /* invalid address field */
  MIX_PERR_INV_IDX,	/* invalid index field */
  MIX_PERR_INV_FSPEC,	/* invalid fspec */
  MIX_PERR_INV_OP,	/* invalid operation */
  MIX_PERR_INV_EXPR,	/* invalid expression */
  MIX_PERR_UNDEF_SYM,	/* undefined symbol */
  MIX_PERR_MIS_PAREN,	/* mismatched parenthesis */
  MIX_PERR_UNEX_FSPEC,	/* unexpected f-spec */
  MIX_PERR_MIS_SYM,	/* missing symbol name */
  MIX_PERR_SYM_INS,	/* symbol has the same name as instruction */
  MIX_PERR_NOWRITE,	/* failed code write */
  MIX_PERR_SHORT_ALF,	/* short ALF operand */
  MIX_PERR_LONG_ALF,	/* too long ALF operand */
  MIX_PERR_UNQUOTED_ALF /* unquoted ALF operand */
} mix_parser_err_t;

extern const gchar *
mix_parser_err_string (mix_parser_err_t error);

extern mix_parser_err_t
mix_parser_compile (mix_parser_t *parser);

extern guint
mix_parser_warning_count (const mix_parser_t *parser);

extern guint
mix_parser_err_count (const mix_parser_t *parser);

/* Write the compilation result to a code file with the given name.
   code_file is completed, if required, with the requisite extension;
   if code_file == NULL [source_file_name].[extension] is used.
   If debug == TRUE, debug information is written.
*/
extern mix_parser_err_t
mix_parser_write_code (mix_parser_t *parser, const gchar *code_file,
                       gboolean debug);

/* Write a "canonical" listing of a compiled source, i.e. a source
   file with all symbols substituted by their actual values after
   compilation.
*/
extern mix_parser_err_t
mix_parser_write_listing (mix_parser_t *parser, const gchar *list_file);

/* load a virtual machine's memory with the contents of a compiled file */
extern mix_parser_err_t
mix_parser_load_vm (const mix_parser_t *parser, mix_vm_t *vm);


#endif /* MIX_PARSER_H */

