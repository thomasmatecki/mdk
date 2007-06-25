/* -*-c-*- ---------------- mixgtk_config.h :
 * Configuration functions declarations.
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


#ifndef MIXGTK_CONFIG_H
#define MIXGTK_CONFIG_H

#include <mixlib/mix.h>
#include <mixlib/mix_config.h>

/* load configuration */
extern gboolean
mixgtk_config_load (void);

/* get mix config */
extern mix_config_t *
mixgtk_config_get_mix_config (void);

/* autosave state */
extern gboolean
mixgtk_config_is_autosave (void);

extern void
mixgtk_config_set_autosave (gboolean autosave);

/* toolbar style */
extern guint
mixgtk_config_tb_style (void);

extern void
mixgtk_config_set_tb_style (guint style);

/* update config item */
extern void
mixgtk_config_update (const gchar *key, const gchar *value);

/* get config item */
extern const gchar *
mixgtk_config_get (const gchar *key);

/* remove config item */
extern void
mixgtk_config_remove (const gchar *key);

/* save configuration */
extern void
mixgtk_config_save (void);

/* shared config params */
extern gboolean
mixgtk_config_show_toolbars (void);

extern void
mixgtk_config_set_show_toolbars (gboolean show);

#endif /* MIXGTK_CONFIG_H */

