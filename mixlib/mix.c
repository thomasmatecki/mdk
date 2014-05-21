/* -*-c-*- -------------- mix.c :
 * Implementation of the functions declared in mix.h
 * ------------------------------------------------------------------
 * Copyright (C) 2000, 2001, 2002, 2005, 2006, 2007, 2008, 2009, 2014 Free Software Foundation, Inc.
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

#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>


#include "mix_types.h"
#include "mix_ins.h"
#include "mix_code_file.h"
#include "mix.h"


/* This function must be called before using the library */
void
mix_init_lib(void)
{
  mix_init_types ();
  mix_init_ins ();
  mix_code_file_set_defext (MIX_CODE_DEFEXT);
}


/* This function must be called for deallocating the lib resources
   when it is no longer in use
*/
void
mix_release_lib (void)
{
  /* clean the user defined code file extension (if any) */
  mix_code_file_set_defext (NULL);
  mix_release_ins ();
}

const char *MIX_GPL_LICENSE =
"Copyright (C) 2000-2014 Free Software Foundation, Inc.\n"
"There is NO warranty.  You may redistribute this software\n"
"under the terms of the GNU General Public License.\n"
"For more information about these matters, see the files named COPYING.\n";

void
mix_print_license (const gchar *program)
{
  fprintf (stderr, _("%s (GNU MDK %s)\n\n"), program, VERSION);
  fprintf (stderr, "%s", MIX_GPL_LICENSE);
}

/* check dir, and create it if it doesn't exist */
gboolean
mix_stat_dir (const gchar *dirname, const gchar *alias)
{
  struct stat statbuf;

  g_return_val_if_fail (dirname != NULL, FALSE);
  if (alias == NULL) alias = "";

  if (stat (dirname, &statbuf) == -1)
    {
      if (errno != ENOENT || mkdir (dirname, S_IRWXU | S_IRWXG | S_IRWXO))
	{
	  g_warning (_("Error creating %s dir %s: %s"),
		     alias, dirname, strerror (errno));
	  return FALSE;
	}
      stat (dirname, &statbuf);
    }

  if (!(statbuf.st_mode & S_IFDIR))
    {
      g_warning (_("Error setting %s dir: %s is not a directory"),
		 alias, dirname);
      return FALSE;
    }

  return TRUE;
}
