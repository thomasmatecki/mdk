/* -*-c-*- -------------- mixgtk_config.c :
 * Implementation of the functions declared in mixgtk_config.h
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
#include "mixgtk.h"
#include "mixgtk_config.h"

static const gchar *MIXGTK_CONFIG_FILE_ = "gmixvm.config";
static const gchar *SHOW_TB_KEY_ = "Toolbars";
static const gchar *TB_YES_ = "Yes";
static const gchar *TB_NO_ = "No";
static const gchar *TB_STYLE_KEY_ = "ToolbarStyle";

static mix_config_t *config_ = NULL;

/* load configuration */
gboolean
mixgtk_config_load (void)
{
  if (config_ == NULL)
    {
      gchar *cdir = g_strconcat (g_get_home_dir (), G_DIR_SEPARATOR_S,
				 MIXGTK_FILES_DIR, NULL);
      config_ = mix_config_new (cdir, MIXGTK_CONFIG_FILE_);
      g_free (cdir);
    }

  return (config_ != NULL);
}

/* get mixlib config */
mix_config_t *
mixgtk_config_get_mix_config (void)
{
  return config_;
}

/* autosave state */
gboolean
mixgtk_config_is_autosave (void)
{
  return mix_config_is_autosave (config_);
}

void
mixgtk_config_set_autosave (gboolean autosave)
{
  mix_config_set_autosave (config_, autosave);
}

guint
mixgtk_config_tb_style (void)
{
  return (guint)mix_config_get_integer (config_, TB_STYLE_KEY_);
}

void
mixgtk_config_set_tb_style (guint style)
{
  mix_config_update_integer (config_, TB_STYLE_KEY_, (gint)style);
}

/* update config item */
void
mixgtk_config_update (const gchar *key, const gchar *value)
{
  mix_config_update (config_, key, value);
}

/* get config item */
const gchar *
mixgtk_config_get (const gchar *key)
{
  return mix_config_get (config_, key);
}

void
mixgtk_config_remove (const char *key)
{
  mix_config_remove (config_, key);
}

/* save configuration */
void
mixgtk_config_save (void)
{
  mix_config_save (config_);
}

gboolean
mixgtk_config_show_toolbars (void)
{
  const gchar *show = mixgtk_config_get (SHOW_TB_KEY_);
  return (!show || !strcmp (show, TB_YES_));
}

void
mixgtk_config_set_show_toolbars (gboolean show)
{
  mixgtk_config_update (SHOW_TB_KEY_, show? TB_YES_ : TB_NO_);
}
