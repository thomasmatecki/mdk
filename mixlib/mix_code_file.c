/* -*-c-*- -------------- mix_code_file.c :
 * Implementation of the functions declared in mix_code_file.h
 * ------------------------------------------------------------------
 * Copyright (C) 2000, 2002, 2003, 2006, 2007 Free Software Foundation, Inc.
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

#include "mix.h"
#include "xmix_io.h"
#include "mix_code_file.h"

/* mix_code_file_t type */
static const gint32 SIGNATURE_ = 0xDEADBEEF;	/* release files */
static const gint32 SIGNATURE_D_ = 0xBEEFDEAD;	/* debug files */

#define IS_DEBUG_(file) ((file)->header.signature == SIGNATURE_D_)
#define IS_RELEASE_(file) ((file)->header.signature == SIGNATURE_)


typedef struct mix_cfheader_t mix_cfheader_t;
struct mix_cfheader_t
{
  gint32 signature;
  gint mj_ver;
  gint mn_ver;
  gint16 start;
  size_t path_len;
};

struct mix_code_file_t
{
  mix_file_t *file;			/* the underlying disk file */
  mix_address_t address;		/* current address while reading */
  mix_cfheader_t header;		/* file header */
  gchar *source_path;			/* variable length part of the header*/
  mix_symbol_table_t *symbol_table;	/* code files with debug info fill it*/
};

#define to_io_(cf)  MIX_IOCHANNEL (cf->file)

/* Code files are lists of tagged words. A MIX_INS_TAG_ denotes
 * that the word is an instruction, whose address is the next to
 * the previous instruction's one, while a MIX_ADDR_TAG_ indicates
 * that the word is a new address origin for the next instructions.
 * The tags are stored as bit patterns in bit 31 of the mix word.
 */
#define MIX_ADDR_TAG_ (MIX_WORD_SIGN_BIT<<1)

#define is_ins_(word) (((word)&MIX_ADDR_TAG_) == MIX_WORD_ZERO)
#define is_addr_(word) (((word)&MIX_ADDR_TAG_) == MIX_ADDR_TAG_)
#define tag_ins_(word) (word)
#define tag_addr_(word) ((word)|MIX_ADDR_TAG_)
#define extract_ins_(tagged) (tagged)
#define extract_addr_(tagged) ((tagged)&MIX_SHORT_MAX)

/* mix code files have a default extension (.mix) which is customizable */
static const gchar *DEFEXT_ = NULL;

const gchar *
mix_code_file_get_defext (void)
{
  return DEFEXT_;
}

gboolean
mix_code_file_set_defext (const gchar *ext)
{
  if (DEFEXT_ != NULL)  g_free ((void *)DEFEXT_);
  DEFEXT_ =  (ext != NULL)? g_strdup (ext):NULL;
  return (DEFEXT_ != NULL || ext == NULL);
}

/* create/destroy code files for read or write */
static mix_code_file_t *
mix_code_file_new_ (const gchar *name, mix_fmode_t mode)
{
  mix_code_file_t *result = g_new (mix_code_file_t, 1);
  if ( result != NULL )
    {
      result->file = mix_file_new_with_def_ext (name, mode, DEFEXT_);
      result->address = MIX_SHORT_ZERO;
      result->source_path = NULL;
      result->symbol_table = NULL;
    }
  if ( result != NULL && result->file == NULL )
    {
      g_free (result);
      result = NULL;
    }
  return result;
}

mix_code_file_t *
mix_code_file_new_read (const gchar *name)
{
  mix_code_file_t *result = mix_code_file_new_ (name, mix_io_READ);
  mix_cfheader_t *header;
  FILE *file;
  gboolean check;

  if ( result == NULL )
    return NULL;

  file = mix_file_to_FILE (result->file);
  header = &(result->header);
  check = fread (header, sizeof (mix_cfheader_t), 1, file)
    && (IS_RELEASE_ (result) || IS_DEBUG_ (result));

  if ( check )
    {
      gint major, minor;
      sscanf (VERSION, "%d.%d", &major, &minor);
      check = header->mj_ver == major && header->mn_ver <= minor;
    }

  if ( check )
    {/* get source path */
      result->source_path = g_strnfill (1 + header->path_len, '\0');
      check =  result->source_path != NULL
	&& fgets (result->source_path, 1 + header->path_len, file) != NULL;
    }

  if ( check && IS_DEBUG_ (result) )
    {/* read symbol table */
      result->symbol_table = mix_symbol_table_new_from_file (file);
      check = result->symbol_table != NULL;
    }

  if ( !check )
    {
      mix_code_file_delete (result);
      return NULL;
    }

  return result;
}

mix_code_file_t *
mix_code_file_new_write(const gchar *name, mix_address_t addr,
			const gchar *source_path, gboolean debug,
			const mix_symbol_table_t *table)
{
  mix_code_file_t *result;
  FILE *file;
  gboolean check;

  result =  mix_code_file_new_ (name, mix_io_WRITE);
  if ( result == NULL || ( file = mix_file_to_FILE (result->file) ) == NULL )
    return NULL;
  else if ( source_path != NULL )
    {
      result->source_path = g_strdup (source_path/*, MAX_PATH_LEN_*/);
      if ( result->source_path == NULL )
	{
	  mix_code_file_delete (result);
	  return NULL;
	}
    }
  else
    result->source_path = NULL;

  result->header.signature = debug? SIGNATURE_D_:SIGNATURE_;
  sscanf (VERSION, "%d.%d", &result->header.mj_ver, &result->header.mn_ver);
  result->header.start = (gint16) addr;
  result->header.path_len = strlen (result->source_path);
  check =  write_data_ (to_io_ (result), &result->header, 1);
  if ( check && result->source_path != NULL )
    check = fputs (result->source_path, file) != EOF;
  if ( check && debug )
    mix_symbol_table_print (table, MIX_SYM_LINE, file, TRUE);
  if ( !check )
    {
      mix_code_file_delete (result);
      return NULL;
    }
  return result;
}

void
mix_code_file_delete (mix_code_file_t *file)
{
  g_return_if_fail (file != NULL);
  mix_file_delete (file->file);
  if (file->source_path) g_free (file->source_path);
  g_free (file);
}

/* get general parameters from a code file */
gboolean
mix_code_file_is_debug (const mix_code_file_t *file)
{
  return (file != NULL) && IS_DEBUG_ (file);
}

gint
mix_code_file_major_version (const mix_code_file_t *file)
{
  g_return_val_if_fail (file != NULL, 0);
  return file->header.mj_ver;
}

gint
mix_code_file_minor_version (const mix_code_file_t *file)
{
  g_return_val_if_fail (file != NULL, 0);
  return file->header.mn_ver;
}

mix_address_t
mix_code_file_get_start_addr (const mix_code_file_t *file)
{
  g_return_val_if_fail (file != NULL, MIX_SHORT_ZERO);
  return mix_short_new (file->header.start);
}

mix_symbol_table_t *
mix_code_file_get_symbol_table(mix_code_file_t *file)
{
  mix_symbol_table_t *result = NULL;
  g_return_val_if_fail (file != NULL, NULL);
  result = file->symbol_table;
  file->symbol_table = NULL;
  return result;
}

/* read instructions from a code file */
gboolean
mix_code_file_is_eof (mix_code_file_t *file)
{
  return is_eof_ (to_io_ (file));
}

gboolean
mix_code_file_get_ins (mix_code_file_t *file, mix_ins_desc_t *desc)
{
  mix_word_t next;
  g_return_val_if_fail (file != NULL, FALSE);
  g_return_val_if_fail (desc != NULL, FALSE);
  while (TRUE)
    {
      if ( ! mix_io_read_word_array (to_io_ (file), &next, 1) ) return FALSE;
      if ( is_addr_ (next) )
	file->address = extract_addr_ (next);
      else if ( is_ins_ (next) )
	{
	  desc->ins = extract_ins_ (next);
	  desc->address = (file->address)++;
	  if ( IS_DEBUG_ (file) )
	    {
	      mix_short_t lineno;
	      if ( !mix_io_read_short_array (to_io_ (file), &lineno, 1) )
		return FALSE;
	      desc->lineno = mix_short_magnitude (lineno);
	    }
	  else
	    desc->lineno = 0;
	  return TRUE;
	}
      else
	{
	  g_assert_not_reached ();
	  return FALSE;
	}
    };
}

/* Write instructions to a code file */
gboolean
mix_code_file_write_ins (mix_code_file_t *file, const mix_ins_desc_t *desc)
{
  g_return_val_if_fail (desc != NULL, FALSE);
  return ( mix_code_file_set_address (file, desc->address) &&
	   mix_code_file_write_next_ins (file, desc->ins, desc->lineno) );
}

gboolean
mix_code_file_write_next_ins (mix_code_file_t *file, mix_word_t ins,
			      guint lineno)
{
  g_return_val_if_fail (file != NULL, FALSE);
  if ( mix_io_write_word (to_io_ (file), tag_ins_ (ins))
       && ( IS_RELEASE_ (file)
	    || mix_io_write_short (to_io_ (file), mix_short_new (lineno)) )
       )
    {
      ++(file->address);
      return TRUE;
    }
  else
    return FALSE;
}

gboolean
mix_code_file_set_address (mix_code_file_t *file, mix_address_t address)
{
  g_return_val_if_fail(file != NULL, FALSE);
  if ( file->address != address ) {
    if ( !mix_io_write_word (to_io_ (file),
			     tag_addr_ (mix_short_to_word_fast (address))) )
      return FALSE;
    file->address = address;
  }
  return TRUE;
}

/* get details about the source file */
const gchar *
mix_code_file_get_source_path (const mix_code_file_t *file)
{
  g_return_val_if_fail (file != NULL, NULL);
  return file->source_path;
}
