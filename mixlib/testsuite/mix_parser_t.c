/* -*-c-*- -------------- mix_parser_t.c :
 * Test of mix_parser_t
 * ------------------------------------------------------------------
 * Copyright (C) 2000, 2006 Free Software Foundation, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
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


#include <mix_parser.h>

#include <stdlib.h>

/* Define VERBOSE_TEST if you want to get prints of the test */
/* #define VERBOSE_TEST */
#include "test.h"

static const gchar * const FILES_[] = {MIX_TEST_MIXAL_FILES};

static const size_t FILE_NO_ = sizeof(FILES_)/sizeof(FILES_[0]);


static void
test_code_ (const gchar *name)
{
  mix_code_file_t *code;
  mix_parser_err_t err;


  gchar *real_name = g_strdup_printf ("%s/%s", MIX_TEST_SAMPLES_DIR, name);
  gchar *code_name = g_path_get_basename (name);

  mix_parser_t *parser = mix_parser_new (real_name);

  g_assert (parser);

  err = mix_parser_compile (parser);

  if (err != MIX_PERR_OK)
    {
      g_print (mix_parser_err_string (err));
      g_print ("\n");
    }

  g_assert (err == MIX_PERR_OK);

  err = mix_parser_write_code (parser, code_name, FALSE);
  code = mix_code_file_new_read (code_name);
  g_assert (code);

  mix_parser_delete (parser);
  mix_code_file_delete (code);
  g_free (real_name);
  g_free (code_name);
}

int
main(int argc, char **argv)
{
  size_t k;

  INIT_TEST;

  for (k = 0; k < FILE_NO_; ++k)
    test_code_(FILES_[k]);

  return EXIT_SUCCESS;
}

