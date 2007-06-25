/* -*-c-*- ---------------- mix.h :
 * Initialisation of the mix library
 * ------------------------------------------------------------------
 * Copyright (C) 2000, 2001, 2006, 2007 Free Software Foundation, Inc.
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


#ifndef MIX_H
#define MIX_H

#ifdef HAVE_CONFIG_H
#	include <config.h>
#else
#	warning "config.h not found, package misconfigured."
#	define PACKAGE "mdk"
#	define VERSION "0.0"
#endif

#ifdef HAVE_LOCALE_H
#	include <locale.h>
#endif

/*
#ifdef HAVE_GETTEXT
#	include <libintl.h>
#	define gettext_noop(String)	(String)
#else
#	include <intl/libgettext.h>
#endif
*/
#include "gettext.h"

#define _(String) gettext (String)
#define N_(String) gettext_noop (String)

#include <glib.h>

/* This function must be called before using the library */
extern void
mix_init_lib (void);

/* This function must be called for deallocating the lib resources
   when it is no longer in use
*/
extern void
mix_release_lib (void);


extern const char *MIX_GPL_LICENSE;

extern void
mix_print_license (const gchar *program);


/* check dir, and create it if it doesn't exist */
extern gboolean
mix_stat_dir (const gchar *dirname, const gchar *alias);


#endif /* MIX_H */

