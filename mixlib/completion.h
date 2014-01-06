/* Adapted from GLIB - Library of useful routines for C programming
 * Copyright (C) 1995-1997  Peter Mattis, Spencer Kimball and Josh MacDonald
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/*
 * Modified by the GLib Team and others 1997-2000.  See the AUTHORS
 * file for a list of people on the GLib Team.  See the ChangeLog
 * files for a list of changes.  These files are distributed with
 * GLib at ftp://ftp.gtk.org/pub/gtk/.
 */

/*
 * Modified by Jose Antonio Ortega Ruiz 2014, for inclusion in MDK due
 * to deprecation in new versions of Glib.  Only changes are renaming
 * GCompletion to Completion and g_completion to completion.
 */

#ifndef __COMPLETION_H__
#define __COMPLETION_H__

#include <glib.h>

typedef struct _Completion     Completion;

typedef gchar*          (*CompletionFunc)      (gpointer);

/* Completion
 */

typedef gint (*CompletionStrncmpFunc) (const gchar *s1,
                                       const gchar *s2,
                                       gsize        n);

struct _Completion
{
  GList* items;
  CompletionFunc func;

  gchar* prefix;
  GList* cache;
  CompletionStrncmpFunc strncmp_func;
};

Completion* completion_new (CompletionFunc func);
void completion_add_items (Completion* cmp, GList* items);
void completion_remove_items (Completion* cmp, GList* items);
void completion_clear_items (Completion* cmp);
GList* completion_complete (Completion* cmp,
                            const gchar* prefix,
                            gchar** new_prefix);
GList* completion_complete_utf8 (Completion *cmp,
                                 const gchar* prefix,
                                 gchar** new_prefix);
void completion_set_compare (Completion *cmp,
                             CompletionStrncmpFunc strncmp_func);
void completion_free (Completion* cmp);

#endif /* __COMPLETION_H__ */
