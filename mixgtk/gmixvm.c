/* -*-c-*- -------------- gmixvm.c :
 * Main function of the mix gtk front-end
 * ------------------------------------------------------------------
 * Copyright (C) 2001, 2002, 2006, 2007, 2009 Free Software Foundation, Inc.
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

#include <stdio.h>
#include <stdlib.h>
#include "mixgtk.h"

static gboolean initfile_ = TRUE;

#ifdef MAKE_GUILE
#  include <mixguile/mixguile.h>
#  include "mixgtk_cmd_dispatcher.h"
static void
inner_main_ (void *closure, int argc, char *argv[])
{
  mixgtk_init (argc, argv);
  mixguile_set_cmd_dispatcher (mixgtk_cmd_dispatcher_get_mix_dispatcher ());
  mixguile_load_bootstrap (initfile_);
  mixgtk_main ();
  mixgtk_release ();
}
#endif

#ifdef HAVE_GETOPT_LONG
#  include <getopt.h>
#else
#  include <lib/getopt.h>
#endif /* HAVE_GETOPT_LONG */

enum {
  VER_OPT = 'v',
  NOINIT_OPT = 'q',
  HELP_OPT = 'h',
  USAGE_OPT = 'u'
};

static const char *options_ = "vqhu";

static struct option long_options_[] =
{
  {"version", no_argument, 0, VER_OPT},
  {"help", no_argument, 0, HELP_OPT},
  {"usage", no_argument, 0, USAGE_OPT},
  {"noinit", no_argument, 0, NOINIT_OPT},
  {0, 0, 0, 0}
};

static void print_usage_ (const gchar *prog)
{
  static const char *usage_ =
    "Usage: %s [-vhuq] [--version] [--help] [--usage] [--noinit]\n";
  fprintf (stderr, usage_, prog);
}

int
main(int argc, char *argv[])
{
  int c;

  const char *prog_name = argv[0];

  setlocale (LC_ALL, "");
  bindtextdomain (PACKAGE, LOCALEDIR);
  textdomain (PACKAGE);

  while (1)
    {
      c = getopt_long (argc, argv, options_, long_options_, (int*)0);

      /* Detect the end of the options. */
      if (c == -1)
	break;

      switch (c)
	{
     	case VER_OPT:
	  mix_print_license (_("gmixvm, GTK MIX virtual machine"));
	  return EXIT_SUCCESS;
	case NOINIT_OPT:
	  initfile_ = FALSE;
	  break;
	case HELP_OPT:
	case USAGE_OPT:
	  print_usage_ (prog_name);
	  return EXIT_SUCCESS;
	case '?':
	  print_usage_ (prog_name);
	  return EXIT_FAILURE;
	default:
	  g_assert_not_reached ();
	  break;
	}
    }

#ifdef MAKE_GUILE
  mixguile_enter (argc, argv, inner_main_);
#else
  if (!mixgtk_init (argc, argv)) return EXIT_FAILURE;
  mixgtk_main ();
  mixgtk_release ();
#endif

  return EXIT_SUCCESS;
}
