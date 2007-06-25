/* -*-c-*- ---------------- mix_config.h :
 * Basic config storage utility.
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


#ifndef MIX_CONFIG_H
#define MIX_CONFIG_H

#include "mix.h"

/* default config dir */
extern const gchar *MIX_CONFIG_DIR;

/* the config type */
typedef struct mix_config_t mix_config_t;

/* create a new config handler, giving the dir and name of the config file */
extern mix_config_t *
mix_config_new (const gchar *dirname, const gchar *filename);

/* delete a config handler, saving the configuration if needed */
extern void
mix_config_delete (mix_config_t *config);

/* get the config filename (fully qualified) */
extern const gchar *
mix_config_get_filename (const mix_config_t *config);

/* get a config item's value from its key */
extern const gchar *
mix_config_get (const mix_config_t *config, const gchar *key);

extern gint
mix_config_get_integer (const mix_config_t *config, const gchar *key);

/* update (or create if it does not exist) a new config item */
extern void
mix_config_update (mix_config_t *config, const gchar *key, const gchar *value);

extern void
mix_config_update_integer (mix_config_t *config, const gchar *key, gint value);

extern void
mix_config_remove (mix_config_t *config, const gchar *key);

/* save the current configuration */
extern void
mix_config_save (const mix_config_t *config);

/** shared config params **/

/* set autosave on delete flag */
extern void
mix_config_set_autosave (mix_config_t *config, gboolean autosave);

extern gboolean
mix_config_is_autosave (const mix_config_t *config);

/* devices dir*/
extern void
mix_config_set_devices_dir (mix_config_t *config, const gchar *dirname);

extern const gchar *
mix_config_get_devices_dir (const mix_config_t *config);

/* history file. if relative path, config dir taken as root */
extern void
mix_config_set_history_file (mix_config_t *config, const gchar *path);

extern const gchar *
mix_config_get_history_file (const mix_config_t *config);

extern void
mix_config_set_history_size (mix_config_t *config, gint s);

extern gint
mix_config_get_history_size (const mix_config_t *config);


#endif /* MIX_CONFIG_H */

