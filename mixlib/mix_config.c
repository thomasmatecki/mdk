/* -*-c-*- -------------- mix_config.c :
 * Implementation of the functions declared in mix_config.h
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


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include "mix_config.h"

const gchar *MIX_CONFIG_DIR = ".mdk";

static const gchar COMMENT_PREFIX_ = '#';
static const gchar *AUTOSAVE_KEY_ = "Autosave";
static const gchar *AUTOSAVE_YES_ = "True";
static const gchar *AUTOSAVE_NO_ = "False";
static const gchar *DEVICES_KEY_ = "Devices.dir";
static const gchar *HISTORY_KEY_ = "History.file";
static const gchar *HISTORY_SIZE_KEY_ = "History.size";

/* the config type */
struct mix_config_t
{
  gchar *filename;		/* full path to configuration file */
  gboolean autosave;		/* whether save on destroy */
  GHashTable *items;		/* configuration items */
};

/* create a new config handler, giving the dir and name of the config file */
mix_config_t *
mix_config_new (const gchar *dirname, const gchar *filename)
{
  static const gchar *DEF_DIRNAME_ = NULL;
  static const gchar *DEF_FILENAME_ = "config";

  const gchar *autosave;
  FILE *f;
  mix_config_t *result = NULL;

  if (DEF_DIRNAME_ == NULL)
    DEF_DIRNAME_ = g_strconcat (g_get_home_dir (), G_DIR_SEPARATOR_S,
				MIX_CONFIG_DIR, NULL);

  if (dirname == NULL) dirname = DEF_DIRNAME_;
  if (filename == NULL) filename = DEF_FILENAME_;

  if (!mix_stat_dir (dirname, "configuration")) return NULL;

  result = g_new (mix_config_t, 1);
  result->filename = g_strdup_printf ("%s/%s", dirname, filename);
  result->items = g_hash_table_new (g_str_hash, g_str_equal);

  f = fopen (result->filename, "r");
  if (f != NULL)
    {
      enum {LEN = 256};
      gchar buffer[LEN];
      gchar *line = buffer;
      while (!feof (f))
	{
	  line = fgets (line, LEN, f);
	  if (line) line = g_strstrip (line);
	  if (line && line[0] != COMMENT_PREFIX_)
	    {
	      gchar **vals = g_strsplit (line, "=", 2);
	      g_hash_table_insert (result->items,
                                   (gpointer) g_strstrip (vals[0]),
                                   (gpointer) g_strstrip (vals[1]));
	    }
	}
      fclose (f);
    }

  autosave = mix_config_get (result, AUTOSAVE_KEY_);
  result->autosave = autosave && !g_ascii_strcasecmp (autosave, AUTOSAVE_YES_);

  return result;
}

/* delete a config handler, saving the configuration if needed */
void
mix_config_delete (mix_config_t *config)
{
  g_return_if_fail (config != NULL);
  if (mix_config_is_autosave (config)) mix_config_save (config);
  g_free (config->filename);
  g_hash_table_destroy (config->items);
  g_free (config);
}

/* get the config filename */
const gchar *
mix_config_get_filename (const mix_config_t *config)
{
  g_return_val_if_fail (config != NULL, NULL);
  return config->filename;
}

/* get a config item's value from its key */
const gchar *
mix_config_get (const mix_config_t *config, const gchar *key)
{
  g_return_val_if_fail (config != NULL, NULL);
  g_return_val_if_fail (key != NULL, NULL);

  return (const gchar*)g_hash_table_lookup (config->items, key);
}

gint
mix_config_get_integer (const mix_config_t *config, const gchar *key)
{
  const gchar *val;
  g_return_val_if_fail (config != NULL, 0);
  g_return_val_if_fail (key != NULL, 0);
  val = mix_config_get (config, key);
  if (!val) return 0;
  return atoi (val);
}

/* update (or create if it does not exist) a new config item */
void
mix_config_update (mix_config_t *config, const gchar *key, const gchar *value)
{
  gpointer okey = NULL;
  gpointer oval = NULL;

  g_return_if_fail (config != NULL);
  g_return_if_fail (key != NULL);
  g_return_if_fail (value != NULL);

  if (g_hash_table_lookup_extended (config->items, key, &okey, &oval))
    {
      if (oval != value)
	{
	  g_free (oval);
	  oval = (gpointer)g_strdup (value);
	}
    }
  else
    {
      okey = (gpointer)g_strdup (key);
      oval = (gpointer)g_strdup (value);
    }

  g_hash_table_insert (config->items, okey, oval);
}

void
mix_config_update_integer (mix_config_t *config, const gchar *key, gint value)
{
  gchar *val;

  g_return_if_fail (config != NULL);
  g_return_if_fail (key != NULL);

  val = g_strdup_printf ("%d", value);
  mix_config_update (config, key, val);
  g_free (val);
}

void
mix_config_remove (mix_config_t *config, const gchar *key)
{
  gchar *val;

  g_return_if_fail (config != NULL);
  g_return_if_fail (key != NULL);

  val = g_hash_table_lookup (config->items, key);
  if (val != NULL)
    {
      g_hash_table_remove (config->items, key);
      g_free (val);
    }
}

/* save the current configuration */
static void
save_ (gpointer key, gpointer value, gpointer file)
{
  fprintf ((FILE *)file, "%s=%s\n", (char *)key, (char *)value);
}

void
mix_config_save (const mix_config_t *config)
{
  FILE *f;

  g_return_if_fail (config != NULL);

  f = fopen (config->filename, "w");
  if (!f)
    {
      g_warning (_("Unable to open config file %s (%s)"),
		 config->filename, g_strerror (errno));
      return;
    }
  g_hash_table_foreach (config->items, save_, (gpointer)f);
  fclose (f);
}

/* set autosave on delete flag */
void
mix_config_set_autosave (mix_config_t *config, gboolean autosave)
{
  mix_config_update (config, AUTOSAVE_KEY_,
		     autosave? AUTOSAVE_YES_ : AUTOSAVE_NO_);
  config->autosave = autosave;
}


gboolean
mix_config_is_autosave (const mix_config_t *config)
{
  g_return_val_if_fail (config != NULL, FALSE);
  return config->autosave;
}

/* devices dir*/
void
mix_config_set_devices_dir (mix_config_t *config, const gchar *dirname)
{
  g_return_if_fail (config != NULL);
  g_return_if_fail (dirname != NULL);
  if (mix_stat_dir (dirname, "devices"))
    mix_config_update (config, DEVICES_KEY_, dirname);
}

extern const gchar *
mix_config_get_devices_dir (const mix_config_t *config)
{
  g_return_val_if_fail (config != NULL, NULL);
  return mix_config_get (config, DEVICES_KEY_);
}

/* history file. if relative path, config dir taken as root */
void
mix_config_set_history_file (mix_config_t *config, const gchar *path)
{
  g_return_if_fail (config != NULL);
  g_return_if_fail (path != NULL);
  if (g_path_is_absolute (path))
    {
      mix_config_update (config, HISTORY_KEY_, path);
    }
  else
    {
      gchar *base = g_path_get_dirname (config->filename);
      gchar *hf = g_strconcat (base, G_DIR_SEPARATOR_S, path, NULL);
      mix_config_update (config, HISTORY_KEY_, hf);
      g_free (hf);
      g_free (base);
    }
}

const gchar *
mix_config_get_history_file (const mix_config_t *config)
{
  g_return_val_if_fail (config != NULL, NULL);
  return mix_config_get (config, HISTORY_KEY_);
}

void
mix_config_set_history_size (mix_config_t *config, gint s)
{
  g_return_if_fail (config != NULL);
  g_return_if_fail (s >= 0);
  mix_config_update_integer (config, HISTORY_SIZE_KEY_, s);
}

gint
mix_config_get_history_size (const mix_config_t *config)
{
  g_return_val_if_fail (config != NULL, 0);
  return mix_config_get_integer (config, HISTORY_SIZE_KEY_);
}
