/*
** Copyright (C) 1999 Free Software Foundation, Inc.
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

/* Common definitions for test programs */

#ifndef TEST_H
#define TEST_H

#include <glib.h>
#include <mix.h>

#define PRINT_BYTE(byte) g_print("%s = %02d",#byte,byte)

#ifdef VERBOSE_TEST /* get printed information  */
#define INIT_TEST \
 do { g_set_print_handler(NULL); mix_init_lib(); } while(FALSE);
#else /* no printed information */
static void
dummy_print_f_(const gchar *m)
{
  /* no output */
}
#define INIT_TEST \
do { g_set_print_handler(dummy_print_f_); mix_init_lib(); } while(FALSE);
#endif /* VERBOSE_TEST */


#endif /* TEST_H */
