/* -*-c-*- -------------- mixvm.c :
 * Main function for mixvm, the mix vm simulator
 * ------------------------------------------------------------------
 * Copyright (C) 2000, 2001, 2002, 2004, 2006, 2007 Free Software Foundation, Inc.
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
#include <mixlib/mix_vm.h>
#include <mixlib/mix_vm_dump.h>

#include <stdlib.h>
#include <stdio.h>

#ifdef HAVE_GETOPT_LONG
#  include <getopt.h>
#else
#  include <lib/getopt.h>
#endif /* HAVE_GETOPT_LONG */

#include "mixvm_loop.h"

enum {
  VER_OPT = 'v',
  HELP_OPT = 'h',
  USAGE_OPT = 'u',
  RUN_OPT = 'r',
  DUMP_OPT = 'd',
  TIME_OPT = 't',
  EMACS_OPT = 'e',	/* used by mixvm-gud only */
  NOINIT_OPT = 'q'
};

static const char *options_ = "vhurdt"; /* no short opt for --emacs */

static struct option long_options_[] =
{
  {"version", no_argument, 0, VER_OPT},
  {"help", no_argument, 0, HELP_OPT},
  {"usage", no_argument, 0, USAGE_OPT},
  {"run", required_argument, 0, RUN_OPT},
  {"dump", no_argument, 0, DUMP_OPT},
  {"time", no_argument, 0, TIME_OPT},
  /* pek: yo! */
  {"emacs", no_argument, 0, EMACS_OPT},
  {"noinit", no_argument, 0, NOINIT_OPT},
  {0, 0, 0, 0}
};

static const gchar *USAGE_ =
N_("Usage: %s [-vhurdqt] [--version] [--help] [--noinit] [--usage]"
   "\n\t[--run] [--dump] [--time] [MIX_FILE]\n");

int
main (int argc, char **argv)
{
  int c;
  const char *prog_name = argv[0];
  const char *in = NULL;
  gboolean run = FALSE;
  gboolean dump = FALSE;
  gboolean emacs = FALSE;
  gboolean initfile = TRUE;
  gboolean ptime = FALSE;

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
	case HELP_OPT: case USAGE_OPT:
	  fprintf (stderr, _(USAGE_), prog_name);
	  return EXIT_SUCCESS;
     	case VER_OPT:
	  mix_print_license ("mixvm, MIX virtual machine");
	  return EXIT_SUCCESS;
	case RUN_OPT:
	  in = optarg;
	  run = TRUE;
	  break;
	case DUMP_OPT:
	  dump = TRUE;
	  break;
	case TIME_OPT:
	  ptime = TRUE;
	  break;
     	case '?':
	  /* getopt already handles the output of a warning message */
	  fprintf (stderr, _("(Try: %s -h)\n"), prog_name);
	  return EXIT_FAILURE;
	case EMACS_OPT:
	  emacs = TRUE;
	  break;
	case NOINIT_OPT:
	  initfile = FALSE;
	  break;
	default:
	  g_assert_not_reached ();
	}
    }

  if ( optind < argc-1 )
    {
      fprintf (stderr, _("*** Error: Too many input files.\n"));
      return EXIT_FAILURE;
    }

  if (!in) in = argv[optind];

  mix_init_lib ();

  if (run) mix_vmrun (in, dump, ptime);
  else mix_vmloop (argc, argv, initfile, in, emacs);

  mix_release_lib ();

  return EXIT_SUCCESS;

}
