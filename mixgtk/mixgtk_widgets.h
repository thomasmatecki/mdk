/* -*-c-*- ---------------- mixgtk_widgets.h :
 * glade-based factory of mixvm widgets
 * ------------------------------------------------------------------
 * Copyright (C) 2001, 2004, 2006, 2007 Free Software Foundation, Inc.
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


#ifndef MIXGTK_WIDGETS_H
#define MIXGTK_WIDGETS_H

#include <gtk/gtk.h>
#include <mixlib/mix_vm.h>

/* enumeration of gmixvm dialogs */
typedef enum {
  MIXGTK_MAIN,	          /* main app window */
  MIXGTK_MIXVM_DIALOG,    /* mixvm dialog */
  MIXGTK_MIXAL_DIALOG,	  /* mixal dialog */
  MIXGTK_DEVICES_DIALOG,  /* devices dialog */
  MIXGTK_WORD_DIALOG,	  /* input dialog for a mix word */
  MIXGTK_ABOUT_DIALOG,
  MIXGTK_GOTO_DIALOG,
  MIXGTK_FONTSEL_DIALOG,  /* font selection dialog */
  MIXGTK_DEVFORM_DIALOG,  /* device format config dialog */
  MIXGTK_EXTERNPROG_DIALOG,	/* external programs dialog */
  MIXGTK_SYMBOLS_DIALOG,	/* symbol table dialog */
  MIXGTK_INPUT_DIALOG,          /* console input dialog */
  MIXGTK_DEVDIR_DIALOG          /* devices directory chooser */
} mixgtk_dialog_id_t;

/* enumeration of mixvm widget ids */
typedef enum {
  MIXGTK_WIDGET_STATUSBAR,
  MIXGTK_WIDGET_ATTACH_BUTTON,
  MIXGTK_WIDGET_DETACH_BUTTON,
  MIXGTK_WIDGET_NOTEBOOK,       /* the notebook */
  MIXGTK_WIDGET_MIXVM,		/* virtual machine */
  MIXGTK_WIDGET_MIXVM_CONTAINER,
  MIXGTK_WIDGET_PROMPT,		/* command prompt */
  MIXGTK_WIDGET_LOG,		/* command output */
  MIXGTK_WIDGET_DEV_CONTAINER,
  MIXGTK_WIDGET_MIXAL_CONTAINER,
  MIXGTK_WIDGET_MIXAL_STATUSBAR,
  MIXGTK_WIDGET_rA,
  MIXGTK_WIDGET_rX,
  MIXGTK_WIDGET_rJ,
  MIXGTK_WIDGET_rI1,
  MIXGTK_WIDGET_rI2,
  MIXGTK_WIDGET_rI3,
  MIXGTK_WIDGET_rI4,
  MIXGTK_WIDGET_rI5,
  MIXGTK_WIDGET_rI6,
  MIXGTK_WIDGET_CMP_L,
  MIXGTK_WIDGET_CMP_E,
  MIXGTK_WIDGET_CMP_G,
  MIXGTK_WIDGET_OVER,
  MIXGTK_WIDGET_CELLS,
  MIXGTK_WIDGET_LAPTIME,
  MIXGTK_WIDGET_PROGTIME,
  MIXGTK_WIDGET_UPTIME,
  MIXGTK_WIDGET_DEVICES,
  MIXGTK_WIDGET_LOC,
  MIXGTK_WIDGET_GOTO_ENTRY,
  MIXGTK_WIDGET_NONE
} mixgtk_widget_id_t;


/* create a new factory from an xml glade file */
extern gboolean
mixgtk_widget_factory_init (void);

/* get a dialog */
extern GtkWidget *
mixgtk_widget_factory_get_dialog (mixgtk_dialog_id_t dlg);

/* get a widget */
extern GtkWidget *
mixgtk_widget_factory_get (mixgtk_dialog_id_t dlg, mixgtk_widget_id_t widget);

/* Get a widget by name */
extern GtkWidget *
mixgtk_widget_factory_get_child_by_name (mixgtk_dialog_id_t dlg,
					 const gchar *name);

#endif /* MIXGTK_WIDGETS_H */

