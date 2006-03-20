/*----------------------- mix_ins_t.c -------------------------------
 * Tests for mix_ins.h
 *-------------------------------------------------------------------
** Copyright (C) 1999  Free Software Foundation, Inc.
**  
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**  
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**  
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
**  
*/

#include <stdlib.h>
#include <mix_ins.h>

/* Define VERBOSE_TEST if you want to get prints of the test */
/* #define VERBOSE_TEST */
#include "test.h"


int
main(int argc, const char**argv)
{
  mix_word_t words[6];
  mix_ins_t ins[3];
  guint k;
  
  INIT_TEST;
  
  g_print("\n...ok.\nTesting mix_ins_id_t properties...\n");
  for ( k = 0; k < mix_INVALID_INS; ++k )
    {
      mix_opcode_t c = mix_get_opcode_from_id(k);
      mix_fspec_t f = mix_get_fspec_from_id(k);
      mix_ins_id_t id = mix_get_ins_id(c,f);
      
      g_print("%02d:%s (%1d:%1d),  ",
	      c, mix_get_string_from_id(k), 
	      mix_fspec_left(f), mix_fspec_right(f));
      if ( (k+1)%3 == 0 ) g_print("\n");

      g_assert(id==k);
    }
  
  g_print("\n...ok.\nTesting mix_ins_t properties...\n");
  for ( k = 1; k < mix_INVALID_INS; ++k )
    {
      g_print("%d ",k);
      mix_ins_fill_from_id(ins[0], k);
      g_assert(mix_ins_id_from_ins(ins[0]) == k);
      ins[0].address = 0x0123;
      ins[0].index = mix_I2;
      words[2] = mix_ins_to_word(ins);
      g_assert(ins[0].address == mix_get_ins_address(words[2]));
      g_assert(ins[0].index == mix_get_ins_index(words[2]));
      g_assert(ins[0].fspec == mix_get_ins_fspec(words[2]));
      g_assert(ins[0].opcode == mix_get_ins_opcode(words[2]));
      g_assert(mix_word_to_ins(words[2],ins+1) == k);
      g_assert(ins[0].address == ins[1].address);
      g_assert(ins[0].index == ins[1].index);
      g_assert(ins[0].fspec == ins[1].fspec);
      g_assert(ins[0].opcode == ins[1].opcode);
    }
  
  g_print("\n...ok.\n");
  
  
  
  return EXIT_SUCCESS;
}
