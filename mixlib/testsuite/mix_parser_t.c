/* -*-c-*- -------------- mix_parser_t.c :
 * Test of mix_parser_t
 * ------------------------------------------------------------------
 * Copyright (C) 2000 Free Software Foundation, Inc.
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



#include <stdlib.h>
#include <mix_parser.h>

/* Define VERBOSE_TEST if you want to get prints of the test */
/* #define VERBOSE_TEST */
#include "test.h"

static const gchar * const FILES_[] = { "test1" , "taocp145"
};

static const size_t FILE_NO_ = sizeof(FILES_)/sizeof(FILES_[0]);

static void
test_code_(const gchar *name)
{
  mix_parser_t *parser = mix_parser_new(name);
  mix_parser_err_t err;
  mix_code_file_t *code;
  mix_ins_desc_t ins;
  
  g_assert(parser);
  err =  mix_parser_compile(parser);
  if ( err != MIX_PERR_OK ) {
    g_print(mix_parser_err_string(err));
    g_print("\n");
  }   
  g_assert(err == MIX_PERR_OK);
  err = mix_parser_write_code(parser, name, FALSE);
  code = mix_code_file_new_read(name);
  g_assert(code);
  g_message("%s: Version: %d.%d", name, mix_code_file_major_version(code),
	    mix_code_file_minor_version(code));
  mix_short_print(mix_code_file_get_start_addr(code), "Start address: ");
  g_print("\n");
  while ( mix_code_file_get_ins(code, &ins) ) {
    mix_ins_t i;
    mix_word_to_ins_uncheck(ins.ins, i);
    mix_short_print(ins.address, "addr: ");
    g_print(" : ");
    mix_ins_print(&i);
    g_print("\n");
  }
  
  mix_parser_delete(parser);
  mix_code_file_delete(code);
}

int 
main(int argc, char **argv)
{
  size_t k;
  
  INIT_TEST;

  for (k = 0; k < FILE_NO_; ++k)
    // test_code_(FILES_[k]);
    ;
    

  return EXIT_SUCCESS;
}

