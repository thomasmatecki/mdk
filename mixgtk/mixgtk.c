/* -*-c-*- -------------- mixgtk.c :
 * Main functions of the mix gtk front-end
 * ------------------------------------------------------------------
 * Copyright (C) 2001, 2002, 2004, 2006, 2007 Free Software Foundation, Inc.
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
#include <string.h>

#include "mixgtk_widgets.h"

#include "mixgtk_cmd_dispatcher.h"
#include "mixgtk_input.h"
#include "mixgtk_config.h"
#include "mixgtk_wm.h"
#include "mixgtk.h"

/* initialise the app */
gboolean
mixgtk_init (int argc, char *argv[])
{
  mix_init_lib ();

  gtk_init (&argc, &argv);

  if (!mixgtk_config_load ())
    {
      g_error (_("Unable to load gmixvm configuration"));
    }

  if (!mixgtk_widget_factory_init ())
    {
      g_error (_("Unable to initialise application: missing glade file"));
      return FALSE;
    }

  if (!mixgtk_wm_init ())
    {
      g_error (_("Unable to initialise application\n"));
      return FALSE;
    }

  mixgtk_input_init ();

  return TRUE;
}

/* main loop */
void
mixgtk_main (void)
{
  gtk_main ();
}

/* clean up */
void
mixgtk_release (void)
{
  if (mixgtk_config_is_autosave ()) mixgtk_config_save ();
  mix_vm_cmd_dispatcher_delete (mixgtk_cmd_dispatcher_get_mix_dispatcher ());
  mix_release_lib ();
}
