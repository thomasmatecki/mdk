/* -*-c-*- ---------------- mixgtk_device.c :
 * actual types for mixgtk devices
 * ------------------------------------------------------------------
 * Copyright (C) 2001, 2002, 2004, 2005, 2006, 2007, 2008 Free Software Foundation, Inc.
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
#include <stdlib.h>
#include <string.h>

#include <mixlib/mix_vm_command.h>
#include <mixlib/xmix_device.h>

#include "mixgtk.h"
#include "mixgtk_gen_handlers.h"
#include "mixgtk_widgets.h"
#include "mixgtk_fontsel.h"
#include "mixgtk_config.h"
#include "mixgtk_cmd_dispatcher.h"
#include "mixgtk_device.h"

#define BIN_DEV_COL_ 5

static const gchar *DEV_FORMAT_KEY_ = "Device.format";

/* device container */
static GtkNotebook *dev_nb_ = NULL;
/* devdir dialog */
static GtkWidget *devdir_dlg_ = NULL;
static GtkEntry *devdir_entry_ = NULL;
/* terminal input dialog */
static GtkWidget *input_dlg_ = NULL;
static GtkEntry *input_dlg_entry_ = NULL;
static GtkListStore *input_list_ = NULL;


/** configuration stuff */
#define LAST_BIN_DEV_   mix_dev_DISK_7
static GtkWidget *devdlg_ = NULL;
static GtkWidget *dtoggle_ = NULL;
static GtkWidget *wtoggle_ = NULL;
static GtkWidget *combo_ = NULL;

/* virtual machine */
static mix_vm_t *vm_ = NULL;
/* dec settings */
static gint32 decs_ = 0;
static gint32 new_decs_ = 0;

/* macros manipulating dec settings */
#define IS_DEC(flags,type) (((flags) >> (type)) & 1)
#define SET_DEC(flags,type) ((flags) |= (1<<(type)))
#define CLEAR_DEC(flags,type) ((flags) &= ~(1<<(type)))

/* a mixgtk device */
struct mixgtk_device_t
{
  mix_device_t device;
  GtkWidget *widget;
  GtkWidget *scroll;
};

struct mixgtk_bin_device_t
{
  struct mixgtk_device_t gtk_device;
  guint last_insert;
  GtkListStore *store;
  gboolean dec;
};

/* callbacks for output devices */
static void
write_char_ (struct mixgtk_device_t *dev, const mix_word_t *block)
{
  enum {MAX_BLOCK = 24, BUFF_SIZE = MAX_BLOCK * 5 + 2};
  static gchar BUFFER[BUFF_SIZE];

  guint k, j;
  GtkTextIter end;
  GtkTextBuffer *buffer;
  GtkTextMark *mark;

  for (k = 0; k < SIZES_[dev->device.type]; k++)
    for (j = 1; j < 6; j++)
      {
	mix_char_t ch = mix_word_get_byte (block[k], j);
	BUFFER[5 * k + j - 1] = mix_char_to_ascii (ch);
      }

  BUFFER[5 * k] = '\n';
  BUFFER[5 * k + 1] = '\0';

  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (dev->widget));
  gtk_text_buffer_get_end_iter (buffer, &end);
  gtk_text_buffer_place_cursor (buffer, &end);
  gtk_text_buffer_insert_at_cursor (buffer, BUFFER, -1);
  mark = gtk_text_buffer_get_insert (buffer);
  gtk_text_view_scroll_to_mark (GTK_TEXT_VIEW (dev->widget), mark, 0,
                                TRUE, 0, 0);
}

static const gchar *
get_word_string_ (mix_word_t w, gboolean dec)
{
  enum {BUFF_SIZE = 17};
  static gchar BUFFER[BUFF_SIZE] = { 0 };
  if (dec)
    g_snprintf (BUFFER, BUFF_SIZE, "%s%011ld",
	      mix_word_is_negative (w)? "-" : "+",
	      mix_word_magnitude (w));
  else
    mix_word_print_to_buffer (w, BUFFER);
  return BUFFER;
}

static void
write_bin_  (struct mixgtk_bin_device_t *dev, const mix_word_t *block)
{
  guint k, col;
  gboolean dec = FALSE;
  size_t len;

  GtkTreeView *view = GTK_TREE_VIEW (dev->gtk_device.widget);
  GtkListStore *store = dev->store;
  GtkTreeIter iter;

  g_assert (view);
  g_assert (store);

  dec = IS_DEC(decs_, dev->gtk_device.device.type);

  for (k = 0, len = SIZES_[dev->gtk_device.device.type];
       k < len;  k += BIN_DEV_COL_)
    {
      gtk_list_store_append (store, &iter);
      for (col = 0; col < BIN_DEV_COL_; ++col)
        gtk_list_store_set (store, &iter,
                            col, get_word_string_ (block[k + col], dec),
                            col + BIN_DEV_COL_, (guint)block[k + col],
                            -1);
      dev->last_insert++;
    }
}

static gboolean
write_ (mix_device_t *dev, const mix_word_t *block)
{
  struct mixgtk_device_t *gtkdev  = (struct mixgtk_device_t *) dev;

  if (dev->type != mix_dev_CONSOLE && !(DEF_DEV_VTABLE_->write)(dev, block))
    return FALSE;

  if (MODES_[dev->type] == mix_dev_CHAR) write_char_ (gtkdev, block);
  else write_bin_ ((struct mixgtk_bin_device_t *)gtkdev, block);

  gtk_notebook_set_current_page (dev_nb_,
                                 gtk_notebook_page_num (dev_nb_,
                                                        gtkdev->scroll));

  return TRUE;
}

static void
init_input_widgets_ (void)
{
  input_dlg_ = mixgtk_widget_factory_get_dialog (MIXGTK_INPUT_DIALOG);
  g_assert (input_dlg_);
  input_dlg_entry_ = GTK_ENTRY (mixgtk_widget_factory_get_child_by_name
                                (MIXGTK_INPUT_DIALOG, "input_entry"));
  g_assert (input_dlg_entry_);
  GtkEntryCompletion *completion = gtk_entry_completion_new ();
  input_list_ = gtk_list_store_new (1, G_TYPE_STRING);
  gtk_entry_completion_set_model (completion, GTK_TREE_MODEL (input_list_));
  gtk_entry_completion_set_popup_completion (completion, TRUE);
#if GTK_CHECK_VERSION(2,8,0)
  gtk_entry_completion_set_popup_single_match (completion, TRUE);
#endif
  gtk_entry_completion_set_inline_completion (completion, FALSE);
  gtk_entry_completion_set_minimum_key_length (completion, 1);
  gtk_entry_completion_set_text_column (completion, 0);
  gtk_entry_set_completion (input_dlg_entry_, completion);
}

static gboolean
find_input_ (const gchar *text)
{
  GtkTreeModel *list_store = GTK_TREE_MODEL (input_list_);
  GtkTreeIter iter;

  gboolean valid = gtk_tree_model_get_iter_first (list_store, &iter);
  gboolean found = FALSE;

  while (valid && !found)
    {
      gchar *str_data;

      gtk_tree_model_get (list_store, &iter, 0, &str_data, -1);

      found = (g_ascii_strcasecmp (str_data, text) == 0);
      valid = gtk_tree_model_iter_next (list_store, &iter);
      g_free (str_data);
    }
  return found;
}

static gboolean
read_cons_ (mix_word_t *block)
{
  gchar *text;
  size_t i, j;

  if (input_dlg_ == NULL)
    {
      init_input_widgets_ ();
    }

  gtk_entry_set_text (input_dlg_entry_, "");
  gint result = gtk_dialog_run (GTK_DIALOG (input_dlg_));
  gtk_widget_hide (input_dlg_);
  if (result == GTK_RESPONSE_OK)
    {
      text = g_strdup_printf ("%-70s", gtk_entry_get_text (input_dlg_entry_));
      for (i = 0; i < 70; ++i)
        for (j = 0; j < 5; ++j)
          mix_word_set_byte (block + i, j + 1,
                             mix_char_to_byte
                             (mix_ascii_to_char (text[5 * i + j])));
      if (!find_input_ (g_strchomp (text)))
        {
          GtkTreeIter iter;
          gtk_list_store_append (input_list_, &iter);
          gtk_list_store_set (input_list_, &iter, 0, text, -1);
        }
      g_free (text);
    }
  return TRUE;
}

static gboolean
read_ (mix_device_t *dev, mix_word_t *block)
{
  struct mixgtk_device_t *gtkdev  = (struct mixgtk_device_t *) dev;

  if (dev->type == mix_dev_CONSOLE && !read_cons_ (block)) return FALSE;

  if (dev->type != mix_dev_CONSOLE && !(DEF_DEV_VTABLE_->read)(dev, block))
    return FALSE;

  if (MODES_[dev->type] == mix_dev_CHAR) write_char_ (gtkdev, block);
  else write_bin_ ((struct mixgtk_bin_device_t *)gtkdev, block);

  gtk_notebook_set_current_page (dev_nb_,
                                 gtk_notebook_page_num (dev_nb_,
                                                        gtkdev->scroll));
  return TRUE;
}

static gboolean
ioc_ (mix_device_t *dev, mix_short_t cmd)
{
  return (DEF_DEV_VTABLE_->ioc)(dev, cmd);
}

static gboolean
busy_ (const mix_device_t *dev)
{
  return (DEF_DEV_VTABLE_->busy)(dev);
}

static void
destroy_ (mix_device_t *dev)
{
  struct mixgtk_device_t *gtkdev = (struct mixgtk_device_t *)dev;
  if (MODES_[dev->type] == mix_dev_BIN) {
    struct mixgtk_bin_device_t * bdev = (struct mixgtk_bin_device_t *)dev;
    gtk_list_store_clear (bdev->store);
    g_object_unref ((gpointer)bdev->store);
  }
  (DEF_DEV_VTABLE_->destroy) (dev);
  gtk_notebook_remove_page (dev_nb_,
                            gtk_notebook_page_num (dev_nb_,
                                                   gtkdev->scroll));
}

static mix_device_vtable_t MIXGTK_VTABLE_ = {
  write_, read_, ioc_, busy_, destroy_
};

/* create the gui part of the device */
static void
mixgtk_device_construct_gui_ (struct mixgtk_device_t *dev)
{
  GtkWidget *label = gtk_label_new (DEF_NAMES_[dev->device.type]);

  g_assert (label);

  dev->scroll = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (dev->scroll),
                                  GTK_POLICY_AUTOMATIC,
                                  GTK_POLICY_AUTOMATIC);

  if (MODES_[dev->device.type] == mix_dev_CHAR)
    {
      dev->widget = gtk_text_view_new ();
      gtk_text_view_set_editable (GTK_TEXT_VIEW (dev->widget), FALSE);
    }
  else
    {
      gint k;
      struct mixgtk_bin_device_t *bindev = (struct mixgtk_bin_device_t *)dev;
      GtkCellRenderer *renderer = gtk_cell_renderer_text_new ();
      GType *types = g_new (GType, BIN_DEV_COL_ * 2);

      for (k = 0; k < BIN_DEV_COL_; ++k)
        {
          types[k] = G_TYPE_STRING;
          types[k + BIN_DEV_COL_] = G_TYPE_UINT;
        }

      bindev->store = gtk_list_store_newv (BIN_DEV_COL_ * 2, types);

      dev->widget =
        gtk_tree_view_new_with_model (GTK_TREE_MODEL (bindev->store));

      gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (dev->widget), FALSE);
      gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (dev->widget), TRUE);

      for (k = 0; k < BIN_DEV_COL_; ++k)
	{
          gtk_tree_view_append_column
            (GTK_TREE_VIEW (dev->widget),
             gtk_tree_view_column_new_with_attributes ("", renderer,
                                                       "text", k, NULL));
	}


      g_free (types);
    }

  g_assert (dev->widget);
  gtk_container_add (GTK_CONTAINER (dev->scroll), dev->widget);
  mixgtk_fontsel_set_font (MIX_FONT_DEVICES, dev->widget);
  gtk_notebook_append_page (dev_nb_, dev->scroll, label);
  gtk_widget_show (label);
  gtk_widget_show (dev->scroll);
  gtk_widget_show (dev->widget);
}

static void
redraw_bin_device_ (struct mixgtk_bin_device_t *dev)
{
  if (dev != NULL)
    {
      GtkTreeModel *store = GTK_TREE_MODEL (dev->store);
      GtkTreeIter iter;
      gboolean valid;

      g_assert (store);

      valid =
        gtk_tree_model_get_iter_first (store, &iter);

      while (valid)
        {
          gint k;
          guint word;

          for (k = 0; k < BIN_DEV_COL_; ++k)
            {
              gtk_tree_model_get (store, &iter, k + BIN_DEV_COL_, &word, -1);
              gtk_list_store_set (GTK_LIST_STORE (store), &iter,
                                  k, get_word_string_ ((mix_word_t)word,
                                                       dev->dec),
                                  -1);
            }
          valid = gtk_tree_model_iter_next (store, &iter);
        }
      gtk_widget_queue_draw (((struct mixgtk_device_t *)dev)->widget);
    }
}


/* create a new mixgtk device */
static mix_device_t *
mixgtk_device_new_ (mix_device_type_t type)
{
  struct mixgtk_device_t *dev = NULL;

  g_return_val_if_fail (type < mix_dev_INVALID, NULL);

  if (MODES_[type] == mix_dev_CHAR)
    {
      dev = g_new (struct mixgtk_device_t, 1);
    }
  else
    {
      dev = (struct mixgtk_device_t *) g_new (struct mixgtk_bin_device_t, 1);
      ((struct mixgtk_bin_device_t *)dev)->last_insert = 0;
    }

  construct_device_ (&dev->device, type);

  dev->device.vtable = &MIXGTK_VTABLE_;

  mixgtk_device_construct_gui_ (dev);

  return (mix_device_t *)dev;
}

/* init default devices */
GtkWidget *
mixgtk_device_init (mix_vm_t *vm)
{
  g_assert (vm != NULL);

  vm_ = vm;
  devdlg_ = NULL;
  devdir_dlg_ = NULL;
  devdir_entry_ = NULL;
  input_dlg_ = NULL;
  dtoggle_ = NULL;
  wtoggle_ = NULL;
  combo_ = NULL;

  if (dev_nb_ == NULL)
    {
      mix_vm_set_device_factory (vm, mixgtk_device_new_);

      /* read format configuration */
      if (mixgtk_config_get (DEV_FORMAT_KEY_))
        decs_ =  atoi (mixgtk_config_get (DEV_FORMAT_KEY_));

      dev_nb_ = GTK_NOTEBOOK (gtk_notebook_new ());
      gtk_notebook_set_tab_pos (dev_nb_, GTK_POS_RIGHT);
      gtk_notebook_set_scrollable (dev_nb_, FALSE);
    }

  mixgtk_fontsel_set_font (MIX_FONT_DEVICES, GTK_WIDGET (dev_nb_));

  return GTK_WIDGET (dev_nb_);
}

void
mixgtk_device_set_format (mix_device_type_t dev, gboolean dec)
{
  gboolean changed;

  g_return_if_fail (dev < mix_dev_INVALID);

  changed = (dec && !IS_DEC (decs_, dev)) || (!dec && IS_DEC (decs_, dev));

  if (changed && (MODES_[dev] == mix_dev_BIN))
    {
      struct mixgtk_bin_device_t *gdev =
        (struct mixgtk_bin_device_t *) mix_vm_get_device (vm_, dev);

      if (gdev != NULL)
        {
          if (dec) SET_DEC (decs_, dev); else CLEAR_DEC (decs_, dev);
          gdev->dec = !(gdev->dec);
          redraw_bin_device_ (gdev);
        }
    }
}


static mix_device_type_t
get_device_idx_ (void)
{
  int k;
  const gchar *name = gtk_entry_get_text (GTK_ENTRY (GTK_BIN (combo_)->child));
  for (k = 0; k <= LAST_BIN_DEV_; ++k)
    if (!strcmp (name, DEF_NAMES_[k])) break;
  return k;
}

static void
init_devform_ (void)
{
  devdlg_ = mixgtk_widget_factory_get_dialog (MIXGTK_DEVFORM_DIALOG);
  g_assert (devdlg_);
  dtoggle_ = mixgtk_widget_factory_get_child_by_name
    (MIXGTK_DEVFORM_DIALOG, "decradio");
  g_assert (dtoggle_);
  wtoggle_ = mixgtk_widget_factory_get_child_by_name
    (MIXGTK_DEVFORM_DIALOG, "wordradio");
  g_assert (wtoggle_);
  combo_ = mixgtk_widget_factory_get_child_by_name
    (MIXGTK_DEVFORM_DIALOG, "dev_combo");
  g_assert (combo_);
  gtk_editable_set_editable (GTK_EDITABLE (GTK_BIN (combo_)->child), FALSE);
}

void
on_dev_combo_changed ()
{
  mix_device_type_t dev = get_device_idx_ ();
  if (dev <= LAST_BIN_DEV_)
    {
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dtoggle_),
                                    IS_DEC (new_decs_, dev));
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (wtoggle_),
                                    !IS_DEC (new_decs_, dev));
    }
}

void
on_devform_activate ()
{
  gint result = GTK_RESPONSE_APPLY;

  if (!devdlg_) init_devform_ ();
  new_decs_ = decs_;

  while (result == GTK_RESPONSE_APPLY)
    {
      //on_dev_combo_changed ();
      result = gtk_dialog_run (GTK_DIALOG (devdlg_));
      if (result != GTK_RESPONSE_CANCEL)
        {
          int k;
          gchar value[20];
          for (k = 0; k <= LAST_BIN_DEV_; ++k)
            mixgtk_device_set_format (k, IS_DEC (new_decs_, k));
          decs_ = new_decs_;
          g_snprintf (value, 20, "%d", decs_);
          mixgtk_config_update (DEV_FORMAT_KEY_, value);
        }
    }

  gtk_widget_hide (devdlg_);
}

void
on_decradio_toggled (GtkToggleButton *button)
{
  if (gtk_toggle_button_get_active (button))
    SET_DEC (new_decs_, get_device_idx_ ());
  else
    CLEAR_DEC (new_decs_, get_device_idx_ ());
}

void
on_devset_button_clicked ()
{
  static gint32 ON = 0xffff, OFF = 0;
  new_decs_ = (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dtoggle_)))?
    ON : OFF;
}

void
on_devdir_activate ()
{
  static const gchar *DEVDIR_ENTRY_NAME = "devdir_entry";
  if (devdir_dlg_ == NULL)
    {
      devdir_dlg_ = mixgtk_widget_factory_get_dialog (MIXGTK_DEVDIR_DIALOG);
      g_assert (devdir_dlg_);
      devdir_entry_ = GTK_ENTRY
	(mixgtk_widget_factory_get_child_by_name (MIXGTK_DEVDIR_DIALOG,
						  DEVDIR_ENTRY_NAME));
      g_assert (devdir_entry_);
    }
  gtk_entry_set_text (devdir_entry_, mix_device_get_dir ());
  gtk_widget_show (devdir_dlg_);
}

void
on_devdir_browse_clicked ()
{
  GtkWidget *dialog;
  const gchar *current = gtk_entry_get_text (devdir_entry_);

  dialog =
    gtk_file_chooser_dialog_new (_("Devices folder"),
                                 GTK_WINDOW (devdir_dlg_),
                                 GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
                                 GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                 GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
                                 NULL);

  if (current != NULL)
    gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (dialog), current);

  if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
    {
      char *filename
        = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
      gtk_entry_set_text (devdir_entry_, filename);
      g_free (filename);
    }

  gtk_widget_destroy (dialog);
}

void
on_devdir_cancel_clicked ()
{
  gtk_widget_hide (devdir_dlg_);
}

void
on_devdir_ok_clicked ()
{
  const gchar *dirname = gtk_entry_get_text (devdir_entry_);
  gchar *cmd = g_strconcat (mix_vm_command_to_string (MIX_CMD_SDDIR),
			    " ", dirname, NULL);
  gtk_widget_hide (devdir_dlg_);
  mixgtk_cmd_dispatcher_dispatch (cmd);
  g_free (cmd);
}



/* fonts */
static GtkWidget **
font_widgets_ (gint * n)
{
  gint no = gtk_notebook_get_n_pages (dev_nb_);
  GtkWidget **w = g_new (GtkWidget *, no + 1);
  gint k, j;

  for (k = 0, j = 0; k < no && j < mix_dev_INVALID; ++j)
    {
      struct mixgtk_device_t * dev =
        (struct mixgtk_device_t *) mix_vm_get_device (vm_, j);
      if (dev != NULL) w[k++] = dev->widget;
    }

  g_assert (k == no);
  w[k] = GTK_WIDGET (dev_nb_);

  if (n) *n = no;

  return w;
}

void
on_devices_font_activate ()
{
  gint no = 0;
  GtkWidget **w = font_widgets_ (&no);
  mixgtk_fontsel_query_font (MIX_FONT_DEVICES, w, no + 1);
  g_free (w);
}

void
mixgtk_device_update_fonts (void)
{
  gint k, n = 0;
  GtkWidget **w = font_widgets_ (&n);

  g_assert (n >= 0);

  for (k = 0; k < n; ++k)
    mixgtk_fontsel_set_font (MIX_FONT_DEVICES, w[k]);
}
