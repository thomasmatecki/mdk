/* -*-c-*- -------------- mix_device_t.c :
 * Implementation of the functions declared in mix_device_t.h
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
#include <mix_device.h>

/* Define VERBOSE_TEST if you want to get prints of the test */
/* #define VERBOSE_TEST */
#include "test.h"

static int S_ = MIX_CHAR_MAX +1;

int 
main (int argc, char **argv) 
{
  mix_device_t *console;
  size_t s;
  mix_word_t **block;
  mix_char_t mchars[S_];
  gchar chars[S_];
  int i, j;
  int bno;
  
  
  INIT_TEST;
  
  console = mix_device_new (mix_dev_CONSOLE);
  s = mix_device_block_size (console);
  bno = S_/(s*5);
  if (bno == 0) bno = 1;
  
  block = g_new (mix_word_t *, bno);
  for (i = 0; i < bno; ++i)
    block[i] = g_new (mix_word_t, s);
  
  for (i = 0; i < S_; ++i) {
    chars[i] = mix_char_to_ascii (i);
    mchars[i] = mix_ascii_to_char (chars[i]);
    g_assert (mchars[i] == i);
  }
  
  for (i = 0; i < bno; ++i) {
    for (j = 0; j < s; ++j) {
      int n = i*s + 5*j;
      if (n < S_) 
	block[i][j] = mix_bytes_to_word (mchars + n, 5);
      else
	block[i][j] = 0;
    }
  }
   
  for (i = 0; i < bno; ++i) {
    mix_device_write (console, block[i]);
  }
  
  
  return EXIT_SUCCESS;
}
