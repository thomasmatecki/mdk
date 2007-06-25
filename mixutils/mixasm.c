/* -*-c-*- -------------- mixasm.c:
 * Main function of mixasm, the mix assembler
 * ------------------------------------------------------------------
 * Copyright (C) 2000, 2001, 2002, 2004, 2005, 2006, 2007 Free Software Foundation, Inc.
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

#ifdef HAVE_GETOPT_LONG
#  include <getopt.h>
#else
#  include <lib/getopt.h>
#endif /* HAVE_GETOPT_LONG */

#include "mixasm_comp.h"

enum {
  VER_OPT = 'v',
  HELP_OPT = 'h',
  USAGE_OPT = 'u',
  OUT_OPT =  'o',
  LIST_OPT = 'l',
  NDEBUG_OPT = 'O'
};


static struct option long_options_[] =
{
  {"version", no_argument, 0, VER_OPT},
  {"help", no_argument, 0, HELP_OPT},
  {"usage", no_argument, 0, USAGE_OPT},
  {"output", required_argument, 0, OUT_OPT},
  {"list", optional_argument, 0, VER_OPT},
  {"ndebug", no_argument, 0, NDEBUG_OPT},
  {0, 0, 0, 0}
};

static const gchar *USAGE_ =
N_("Usage: %s [-vhulO] [-o OUTPUT_FILE] [--version] [--help]\n"
   "\t[--usage] [--ndebug] [--output=OUTPUT_FILE] [--list[=LIST_FILE]] file\n");


int
main (int argc, char **argv)
{
  int c;
  const char *prog_name = argv[0];
  const char *src = NULL, *out = NULL, *list = NULL;
  gboolean use_list = FALSE, debug = TRUE;

  setlocale (LC_ALL, "");
  bindtextdomain (PACKAGE, LOCALEDIR);
  textdomain (PACKAGE);

  while (1)
    {
      /* -g option is still available, but is no longer used */
      c = getopt_long (argc, argv, "vhuo:lOg", long_options_, (int*)0);

      /* Detect the end of the options. */
      if (c == -1)
	break;

      switch (c)
	{
	case HELP_OPT: case USAGE_OPT:
	  fprintf (stderr, _(USAGE_), prog_name);
	  return EXIT_SUCCESS;
     	case VER_OPT:
	  mix_print_license ("mixasm, MIX assembler");
	  return EXIT_SUCCESS;
	case OUT_OPT:
	  out = optarg;
	  break;
     	case LIST_OPT:
	  use_list = TRUE;
	  list = optarg;
	  break;
	case NDEBUG_OPT:
	  debug = FALSE;
	  break;
        case 'g':
          /* used to be the switch to create debug version, not needed anymore */
          break;
     	case '?':
	  /* getopt already handles the output of a warning message */
	  fprintf (stderr, _("(Try: %s -h)\n"), prog_name);
	  return EXIT_FAILURE;
	default:
	  g_assert_not_reached ();
	}
    }

  if ( optind == argc )
    {
      fprintf (stderr, _("*** Error: Missing source file.\n"));
      return EXIT_FAILURE;
    }
  if ( optind < argc-1 )
    {
      fprintf (stderr, _("*** Error: Too many input files.\n"));
      return EXIT_FAILURE;
    }
  src = argv[optind];


  mix_init_lib ();

  c = mix_asm_compile (src, out, use_list, list, debug);

  mix_release_lib ();

  return c;

}

