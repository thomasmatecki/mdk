/* -*-c-*- ---------------- mixgtk_input.h :
 * Declaration of functions for user input
 * ------------------------------------------------------------------
 *  Last change: Time-stamp: <01/03/12 23:35:14 jose>
 * ------------------------------------------------------------------
 * Copyright (C) 2001, 2007 Free Software Foundation, Inc.
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


#ifndef MIXGTK_INPUT_H
#define MIXGTK_INPUT_H

#include <mixlib/mix_types.h>

/* callback function type */
typedef void (*input_callback_t)(mix_word_t result, gpointer data);

/* init */
extern void
mixgtk_input_init (void);

/* get a word */
extern void
mixgtk_input_word (const gchar *message, mix_word_t def,
		   input_callback_t cb, gpointer data);

/* get a short */
extern void
mixgtk_input_short (const gchar *message, mix_short_t def,
		    input_callback_t cb, gpointer data );


#endif /* MIXGTK_INPUT_H */

