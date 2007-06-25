/* -*-c-*- -------------- mixasm_comp.c :
 * Implementation of the functions declared in mixasm_comp.h
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

#include <mixlib/mix.h>
#include <stdlib.h>
#include <stdio.h>
#include <mixlib/mix_parser.h>
#include "mixasm_comp.h"

int
mix_asm_compile(const gchar *src, const gchar *out, gboolean use_list,
		const gchar *list, gboolean debug)
{
  int result = EXIT_SUCCESS;
  mix_parser_t *parser;
  mix_parser_err_t error;

  if ( (parser = mix_parser_new(src)) == NULL )
    {
      fprintf(stderr, _("*** Unable to open source file %s\n"), src);
      return EXIT_FAILURE;
    }
  if ( mix_parser_compile(parser) == MIX_PERR_OK )
    {
      guint k;
      if ( ( k = mix_parser_warning_count(parser) ) != 0 )
	fprintf(stderr, _("(%d warning(s))\n"), k);
      if ( (error = mix_parser_write_code(parser, out, debug)) != MIX_PERR_OK )
	{
	  fprintf(stderr, _("*** Error writing output code file: %s\n"),
		  mix_parser_err_string(error));
	  result = EXIT_FAILURE;
	}
      else if ( use_list
		&& (error = mix_parser_write_listing(parser, list)) !=
		MIX_PERR_OK)
	{
	  fprintf(stderr, _("*** Error writing listing file: %s\n"),
		  mix_parser_err_string(error));
	  result = EXIT_FAILURE;
	}
    }
  else
    {
      fprintf(stderr, _("(%d warning(s), %d error(s))\n"),
	      mix_parser_warning_count(parser), mix_parser_err_count(parser));
      result = EXIT_FAILURE;
    }

  mix_parser_delete(parser);
  return result;
}


