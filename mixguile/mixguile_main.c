/* -*-c-*- -------------- mixguile_main.c :
 * Main function for mixguile, the MIX Guile shell
 * ------------------------------------------------------------------
 * Copyright (C) 2001, 2006, 2007 Free Software Foundation, Inc.
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

#include <stdlib.h>
#include <stdio.h>
#include "mixguile.h"


#ifdef HAVE_GETOPT_LONG
#  include <getopt.h>
#else
#  include <lib/getopt.h>
#endif /* HAVE_GETOPT_LONG */

enum {
  VER_OPT = 'v',
  NOINIT_OPT = 'q',
};

static const char *options_ = "vq";

static struct option long_options_[] =
{
  {"version", no_argument, 0, VER_OPT},
  {0, 0, 0, 0}
};

int
main (int argc, char *argv[])
{
  const gchar *CONFIG_FILE = "mixvm.config";
  mix_config_t *config;
  mix_vm_cmd_dispatcher_t *dis;
  int c;

  gboolean initfile = TRUE;

  setlocale (LC_ALL, "");
  bindtextdomain (PACKAGE, LOCALEDIR);
  textdomain (PACKAGE);

  /* prevent getopt printing a message for unknown options (stored in optopt) */
  opterr = 0;

  while (1)
    {
      c = getopt_long (argc, argv, options_, long_options_, (int*)0);

      /* Detect the end of the options. */
      if (c == -1)
	break;

      switch (c)
	{
     	case VER_OPT:
	  mix_print_license ("mixguile, Scheme MIX Virtual Machine");
	  return EXIT_SUCCESS;
	case NOINIT_OPT:
	  initfile = FALSE;
	  break;
	default:
	  /* let guile try to understand the option */
	  break;
	}
    }

  mix_init_lib ();

  config = mix_config_new (NULL, CONFIG_FILE);
  dis = mix_vm_cmd_dispatcher_new_with_config (stdout, stderr, config);
  mixguile_init (argc, argv, initfile, mixguile_enter_repl, dis);

  return EXIT_SUCCESS; /* never reached */
}
