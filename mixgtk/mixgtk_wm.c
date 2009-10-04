/* -*-c-*- -------------- mixgtk_wm.c :
 * Implementation of the functions declared in mixgtk_wm.h
 * ------------------------------------------------------------------
 * Copyright (C) 2001, 2002, 2004, 2006, 2007, 2008 Free Software Foundation, Inc.
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

#include <string.h>

#include "mixgtk_wm.h"

#include "mixgtk_config.h"
#include "mixgtk_device.h"
#include "mixgtk_mixvm.h"
#include "mixgtk_mixal.h"
#include "mixgtk_widgets.h"
#include "mixgtk_cmd_dispatcher.h"
#include "mixgtk_gen_handlers.h"
#include "mixgtk.h"


typedef struct window_info_t_
{
  mixgtk_dialog_id_t dialog;
  GtkWidget *widget;
  GtkCheckMenuItem *menu;
  const gchar *menu_name;
  const gchar *config_key;
  gboolean detached;
  void (*detach) (void);
  void (*attach) (void);
} window_info_t_;

static const gchar *DETACH_YES_ = "Yes";
static const gchar *DETACH_NO_ = "No";
static GtkWidget *about_ = NULL;

static GtkContainer *mixvm_container_ = NULL;
static GtkContainer *mixal_container_ = NULL;
static GtkContainer *dev_container_ = NULL;

static GtkToolItem *attach_button_ = NULL;
static GtkToolItem *detach_button_ = NULL;

static const gchar *TB_MENU_NAME_ = "show_toolbars";
static GtkCheckMenuItem *tb_menu_ = NULL;
static GtkNotebook *notebook_ = NULL;

static mix_vm_t *vm_ = NULL;

static void init_info_ (void);
static void init_notebook_ (void);
static void init_dispatcher_ (void);
static void init_mixvm_ (void);
static void init_mixal_ (void);
static void init_dev_ (void);
static void init_signals_ (void);
static void init_visibility_ (void);
static void init_tb_ (void);
static void init_about_ (void);
static void init_autosave_ (void);
static void set_tb_style_ (guint style);
static void show_toolbars_ (gboolean show);
static void add_page_ (GtkWidget *w, mixgtk_window_id_t id);
static void mixvm_attach_ (void);
static void mixvm_detach_ (void);
static void mixal_attach_ (void);
static void mixal_detach_ (void);
static void dev_attach_ (void);
static void dev_detach_ (void);
static void update_attach_buttons_ (void);
static void on_tb_style_ (GtkMenuItem *w, gpointer style);
static void on_nb_switch_ (GtkNotebook *notebook, GtkWidget *page,
                           guint page_num, gpointer user_data);

static window_info_t_ infos_[] = {
  {MIXGTK_MIXVM_DIALOG, NULL, NULL, "detach_vm", "MIX.detach",
   FALSE, mixvm_detach_, mixvm_attach_},
  {MIXGTK_MIXAL_DIALOG, NULL, NULL, "detach_source", "MIXAL.detach",
   FALSE, mixal_detach_, mixal_attach_},
  {MIXGTK_DEVICES_DIALOG, NULL, NULL, "detach_dev", "Devices.detach",
   FALSE, dev_detach_, dev_attach_}
};

static size_t INF_NO_ = sizeof (infos_) / sizeof (infos_[0]);


gboolean
mixgtk_wm_init (void)
{
  gint k;

  init_info_ ();
  init_notebook_ ();
  init_dispatcher_ ();
  init_mixvm_ ();
  init_mixal_ ();
  init_dev_ ();
  init_tb_ ();

  for (k = 0; k < INF_NO_; ++k)
    {
      if (infos_[k].detached)
        mixgtk_wm_detach_window (k);
      else
        mixgtk_wm_attach_window (k);
    }

  init_about_ ();
  init_autosave_ ();
  init_visibility_ ();
  init_signals_ ();

  return TRUE;
}

void
mixgtk_wm_detach_window (mixgtk_window_id_t w)
{
  if (w < INF_NO_)
    {
      (*(infos_[w].detach)) ();
      infos_[w].detached = TRUE;
      gtk_check_menu_item_set_active (infos_[w].menu, TRUE);
      mixgtk_config_update (infos_[w].config_key, DETACH_YES_);
      if (infos_[w].widget != NULL) gtk_widget_show (infos_[w].widget);
      if (gtk_notebook_get_n_pages (notebook_) < 1)
        gtk_widget_hide (GTK_WIDGET (notebook_));
      gtk_widget_show (mixgtk_widget_factory_get_dialog (infos_[w].dialog));
      update_attach_buttons_ ();
    }
}

void
mixgtk_wm_attach_window (mixgtk_window_id_t w)
{
  if (w < INF_NO_)
    {
      gint page = gtk_notebook_page_num (notebook_, infos_[w].widget);

      gtk_widget_hide (mixgtk_widget_factory_get_dialog (infos_[w].dialog));

      if (page < 0) (*(infos_[w].attach)) ();

      infos_[w].detached = FALSE;
      gtk_check_menu_item_set_active (infos_[w].menu, FALSE);
      mixgtk_config_update (infos_[w].config_key, DETACH_NO_);
      if (gtk_notebook_get_n_pages (notebook_) == 1)
        gtk_widget_show (GTK_WIDGET (notebook_));
      update_attach_buttons_ ();
    }
}



/* callbacks */
void
on_detach_clicked (GtkWidget *ignored)
{
  gint page = gtk_notebook_get_current_page (notebook_);
  if (page >= 0)
    {
      gint k;
      GtkWidget *w = gtk_notebook_get_nth_page (notebook_, page);

      for (k = 0; k < INF_NO_; ++k)
        if (infos_[k].widget == w) mixgtk_wm_detach_window (k);
    }
}

void
on_attach_all_clicked ()
{
  gint k;
  for (k = INF_NO_; k > 0; --k)
    if (infos_[k - 1].detached) mixgtk_wm_attach_window (k - 1);
}

void
on_attach_toggled (GtkCheckMenuItem *item)
{
  gint k;
  for (k = 0; k < INF_NO_; ++k)
    if (item == infos_[k].menu) break;
  g_return_if_fail (k < INF_NO_);
  if (item->active) mixgtk_wm_detach_window (k);
  else mixgtk_wm_attach_window (k);
  mixgtk_config_update (infos_[k].config_key,
                        (item->active)? DETACH_YES_ : DETACH_NO_);
}

void
on_window_hide (GtkWidget *w)
{
  gint k;
  for (k = 0; k < INF_NO_; ++k)
    if (w == mixgtk_widget_factory_get_dialog (infos_[k].dialog)) break;
  g_return_if_fail (k < INF_NO_);
  mixgtk_wm_attach_window (k);
}

void
on_show_toolbars_toggled (GtkCheckMenuItem *item)
{
  if (item->active != mixgtk_config_show_toolbars ())
    show_toolbars_ (item->active);
}

void
on_widget_attach (GtkWidget *ign, gpointer id)
{
  mixgtk_wm_attach_window (GPOINTER_TO_INT (id));
}

/* about box */
void
on_about_activate (GtkWidget *w, gpointer data)
{
  if (!about_) init_about_ ();
  gtk_widget_show (about_);
}

void
on_about_clicked (GtkWidget *w)
{
  if (about_) gtk_widget_hide (about_);
}



static void
init_info_ (void)
{
  gint k;
  for (k = 0; k < INF_NO_; ++k)
    {
      const gchar *txt;

      infos_[k].menu =
        GTK_CHECK_MENU_ITEM
        (mixgtk_widget_factory_get_child_by_name (MIXGTK_MAIN,
                                                  infos_[k].menu_name));
      g_assert (infos_[k].menu != NULL);
      txt = mixgtk_config_get (infos_[k].config_key);
      infos_[k].detached = txt && !g_ascii_strcasecmp (txt, DETACH_YES_);
    }
}

static void
init_notebook_ (void)
{
  notebook_ =
    GTK_NOTEBOOK (mixgtk_widget_factory_get (MIXGTK_MAIN,
                                             MIXGTK_WIDGET_NOTEBOOK));
  g_assert (notebook_ != NULL);
  gtk_widget_show (GTK_WIDGET (notebook_));
}

static void
add_page_ (GtkWidget *page, mixgtk_window_id_t id)
{
  static const gchar *LABELS[] = {
    N_("_Virtual machine"), N_("_Source"), N_("_Devices")
  };

  gint p = 0;

  g_assert (page != NULL);
  g_assert (id < INF_NO_);
  g_assert (notebook_ != NULL);

  if (id > MIXGTK_MIXVM_WINDOW)
    {
      if (id == MIXGTK_DEVICES_WINDOW)
        p = gtk_notebook_get_n_pages (notebook_);
      else
        p = infos_[MIXGTK_MIXVM_WINDOW].detached ? 0 : 1;
    }

  gtk_notebook_insert_page (notebook_, page,
                            gtk_label_new_with_mnemonic (LABELS[id]),
                            p);
  gtk_notebook_set_current_page (notebook_, p);
  gtk_widget_show (page);
  gtk_widget_show (GTK_WIDGET (notebook_));
}

static void
init_signals_ (void)
{
  gint k;
  for (k = 0; k < INF_NO_; ++k)
    {
      GObject *dialog =
        G_OBJECT (mixgtk_widget_factory_get_dialog (infos_[k].dialog));

      GObject *button =
        G_OBJECT (mixgtk_widget_factory_get (infos_[k].dialog,
                                             MIXGTK_WIDGET_ATTACH_BUTTON));

      g_assert (dialog != NULL);
      g_assert (button != NULL);

      g_signal_connect (button, "clicked",
                        G_CALLBACK (on_widget_attach), GINT_TO_POINTER (k));
      g_signal_connect (dialog, "destroy",
                        G_CALLBACK (on_window_hide), NULL);
      g_signal_connect (dialog, "destroy_event",
                        G_CALLBACK (on_window_hide), NULL);
      g_signal_connect (dialog, "delete_event",
                        G_CALLBACK (on_window_hide), NULL);
      g_signal_connect (G_OBJECT (infos_[k].menu), "toggled",
                        G_CALLBACK (on_attach_toggled), NULL);
    }

  GtkWindow *mainw =
    GTK_WINDOW (mixgtk_widget_factory_get_dialog (MIXGTK_MAIN));

  g_signal_connect (mainw, "destroy",
                    G_CALLBACK (on_file_exit_activate), NULL);
}

static void
init_visibility_ (void)
{
  gint k;
  for (k = 0; k < INF_NO_; ++k)
    if (infos_[k].detached)
      gtk_widget_show (mixgtk_widget_factory_get_dialog (infos_[k].dialog));

  if (gtk_notebook_get_n_pages (notebook_) > 0)
    gtk_notebook_set_current_page (notebook_, 0);

  gtk_widget_show (mixgtk_widget_factory_get_dialog (MIXGTK_MAIN));
}

static void
init_dispatcher_ (void)
{
  if (!mixgtk_cmd_dispatcher_init (MIXGTK_MAIN))
    g_assert (FALSE);
  vm_ = mixgtk_cmd_dispatcher_get_vm ();
  g_assert (vm_ != NULL);
}

static void
init_mixvm_ (void)
{
  GtkContainer *vm =
    GTK_CONTAINER
    (mixgtk_widget_factory_get (MIXGTK_MIXVM_DIALOG, MIXGTK_WIDGET_MIXVM));

  mixvm_container_ =
    GTK_CONTAINER (mixgtk_widget_factory_get (MIXGTK_MIXVM_DIALOG,
                                              MIXGTK_WIDGET_MIXVM_CONTAINER));

  g_assert (mixvm_container_ != NULL);

  g_assert (vm != NULL);
  g_object_ref (G_OBJECT (vm));
  infos_[MIXGTK_MIXVM_WINDOW].widget = GTK_WIDGET (vm);

  mixgtk_mixvm_init (vm_);
  mixgtk_mixvm_update_vm_widgets ();

  gtk_widget_show (GTK_WIDGET (vm));

}

static void
init_mixal_ (void)
{
  GtkWidget *page = gtk_scrolled_window_new (NULL, NULL);
  GtkWidget *mixal = mixgtk_mixal_init (vm_);

  g_assert (page != NULL);
  g_assert (mixal != NULL);

  g_object_ref (page);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (page),
                                  GTK_POLICY_AUTOMATIC,
                                  GTK_POLICY_AUTOMATIC);

  gtk_container_add (GTK_CONTAINER (page), mixal);

  infos_[MIXGTK_MIXAL_WINDOW].widget = page;

  mixal_container_ =
    GTK_CONTAINER (mixgtk_widget_factory_get (MIXGTK_MIXAL_DIALOG,
                                              MIXGTK_WIDGET_MIXAL_CONTAINER));

  g_assert (mixal_container_ != NULL);

  gtk_container_add (mixal_container_, page);

  gtk_widget_show (mixal);
  gtk_widget_show (page);
  gtk_widget_show (GTK_WIDGET (mixal_container_));
}

static void
init_dev_ (void)
{
  GtkWidget *page = gtk_scrolled_window_new (NULL, NULL);
  GtkWidget *devs = mixgtk_device_init (vm_);

  g_assert (page != NULL);
  g_assert (devs != NULL);

  g_object_ref (page);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (page),
                                  GTK_POLICY_AUTOMATIC,
                                  GTK_POLICY_AUTOMATIC);

  gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (page), devs);

  infos_[MIXGTK_DEVICES_WINDOW].widget = page;

  dev_container_ =
    GTK_CONTAINER (mixgtk_widget_factory_get (MIXGTK_DEVICES_DIALOG,
                                              MIXGTK_WIDGET_DEV_CONTAINER));

  g_assert (dev_container_ != NULL);

  gtk_container_add (dev_container_, page);

  gtk_widget_show (devs);
  gtk_widget_show (page);
}

static void
init_tb_ (void)
{
  gchar *names[4];
  gint k;
  gint style = mixgtk_config_tb_style ();

  tb_menu_ = GTK_CHECK_MENU_ITEM
    (mixgtk_widget_factory_get_child_by_name (MIXGTK_MAIN,
  					      TB_MENU_NAME_));
  g_assert (tb_menu_ != NULL);

  gtk_check_menu_item_set_active (tb_menu_, mixgtk_config_show_toolbars ());
  show_toolbars_ (mixgtk_config_show_toolbars ());

  g_signal_connect (G_OBJECT (tb_menu_), "toggled",
                    G_CALLBACK (on_show_toolbars_toggled), NULL);

  attach_button_ = GTK_TOOL_ITEM
    (mixgtk_widget_factory_get (MIXGTK_MAIN, MIXGTK_WIDGET_ATTACH_BUTTON));
  detach_button_ = GTK_TOOL_ITEM
    (mixgtk_widget_factory_get (MIXGTK_MAIN, MIXGTK_WIDGET_DETACH_BUTTON));

  g_assert (attach_button_);
  g_assert (detach_button_);

  names[GTK_TOOLBAR_ICONS] = "tb_style_icons";
  names[GTK_TOOLBAR_TEXT] = "tb_style_labels";
  names[GTK_TOOLBAR_BOTH] = "tb_style_both";
  names[GTK_TOOLBAR_BOTH_HORIZ] = "tb_style_bothh";

  for (k = 0; k < 4; ++k)
    {
      GtkWidget *item =
        mixgtk_widget_factory_get_child_by_name (MIXGTK_MAIN, names[k]);
      g_signal_connect (G_OBJECT (item),
                        "activate",
                        G_CALLBACK (on_tb_style_), GUINT_TO_POINTER (k));
      gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (item), (k == style));
    }
  set_tb_style_ (style);
}

static void
on_tb_style_ (GtkMenuItem *w, gpointer style)
{
  guint ui_style = GPOINTER_TO_UINT (style);
  set_tb_style_ (ui_style);
  mixgtk_config_set_tb_style (ui_style);
}

static void
set_tb_style_ (guint style)
{
  static const gchar *TB_NAME = "main_toolbar";
  static const gchar *TB_DNAME = "dlg_toolbar";

  gint k;

  GtkToolbar *tb = GTK_TOOLBAR
    (mixgtk_widget_factory_get_child_by_name (MIXGTK_MAIN, TB_NAME));
  gtk_toolbar_set_style (tb, style);

  for (k = 0; k < INF_NO_; ++k)
    {
      GtkToolbar *tb = GTK_TOOLBAR
        (mixgtk_widget_factory_get_child_by_name (infos_[k].dialog, TB_DNAME));
      gtk_toolbar_set_style (tb, style);
    }
}

static void
show_toolbars_ (gboolean show)
{
  static const gchar *HANDLE_NAME = "tb_handle";

  gint k;

  GtkWidget *handle =
    mixgtk_widget_factory_get_child_by_name (MIXGTK_MAIN, HANDLE_NAME);

  if (show)
    gtk_widget_show (handle);
  else
    gtk_widget_hide (handle);

  for (k = 0; k < INF_NO_; ++k)
    {
      GtkWidget *hd =
        mixgtk_widget_factory_get_child_by_name (infos_[k].dialog, HANDLE_NAME);
      if (show) gtk_widget_show (hd); else gtk_widget_hide (hd);
    }

  mixgtk_config_set_show_toolbars (show);
}

static void
init_autosave_ (void)
{
#define AUTOSAVE_ITEM_ "save_on_exit"

  GtkCheckMenuItem *item = GTK_CHECK_MENU_ITEM
    (mixgtk_widget_factory_get_child_by_name (MIXGTK_MAIN, AUTOSAVE_ITEM_));
  if (item)
    {
      gtk_check_menu_item_set_active (item, mixgtk_config_is_autosave ());
    }
}

static void
init_about_ (void)
{
#define VERSION_LABEL_ "version_label"
#define GPL_TEXT_ "gpl_text"
  GtkWidget *label;
  about_ = mixgtk_widget_factory_get_dialog (MIXGTK_ABOUT_DIALOG);
  g_assert (about_ != NULL);
  label = mixgtk_widget_factory_get_child_by_name (MIXGTK_ABOUT_DIALOG,
						   VERSION_LABEL_);
  g_assert (label != NULL);
  gtk_label_set_text (GTK_LABEL (label), VERSION);
  gtk_widget_show (label);
}

static void
mixvm_attach_ (void)
{
  gtk_container_remove (mixvm_container_, infos_[MIXGTK_MIXVM_WINDOW].widget);
  add_page_ (infos_[MIXGTK_MIXVM_WINDOW].widget, MIXGTK_MIXVM_WINDOW);
}

static void
mixvm_detach_ (void)
{
  gtk_widget_reparent (infos_[MIXGTK_MIXVM_WINDOW].widget,
                       GTK_WIDGET (mixvm_container_));
}

static void
mixal_attach_ (void)
{
  static GtkStatusbar *stat = NULL;

  if (stat == NULL)
    {
      stat =
        GTK_STATUSBAR
        (mixgtk_widget_factory_get (MIXGTK_MAIN, MIXGTK_WIDGET_STATUSBAR));
      g_assert (stat);

      g_signal_connect (G_OBJECT (notebook_), "switch-page",
                        G_CALLBACK (on_nb_switch_), NULL);
    }

  mixgtk_mixal_reparent (stat);
  gtk_container_remove (mixal_container_, infos_[MIXGTK_MIXAL_WINDOW].widget);
  add_page_ (infos_[MIXGTK_MIXAL_WINDOW].widget, MIXGTK_MIXAL_WINDOW);
}

static void
mixal_detach_ (void)
{
  static GtkStatusbar *stat = NULL;

  if (stat == NULL)
    {
      stat =
        GTK_STATUSBAR
        (mixgtk_widget_factory_get (MIXGTK_MIXAL_DIALOG,
                                    MIXGTK_WIDGET_MIXAL_STATUSBAR));
      g_assert (stat);
    }

  mixgtk_mixal_reparent (stat);
  gtk_widget_reparent (infos_[MIXGTK_MIXAL_WINDOW].widget,
                       GTK_WIDGET (mixal_container_));
}

static void
dev_attach_ (void)
{
  gtk_container_remove (dev_container_, infos_[MIXGTK_DEVICES_WINDOW].widget);
  add_page_ (infos_[MIXGTK_DEVICES_WINDOW].widget, MIXGTK_DEVICES_WINDOW);
}

static void
dev_detach_ (void)
{
  gtk_widget_reparent (infos_[MIXGTK_DEVICES_WINDOW].widget,
                       GTK_WIDGET (dev_container_));
}

static void
on_nb_switch_ (GtkNotebook *notebook, GtkWidget *page,
               guint page_num, gpointer user_data)
{
  if (!(infos_[MIXGTK_MIXAL_WINDOW].detached)
      && (page != infos_[MIXGTK_MIXAL_WINDOW].widget))
    mixgtk_mixal_pop_status ();
}

static void
update_attach_buttons_ (void)
{
  gint k;
  gboolean wants_attach = FALSE;
  gboolean wants_detach = FALSE;

  g_assert (attach_button_);
  g_assert (detach_button_);

  for (k = 0; k < INF_NO_; ++k)
    {
      wants_attach = wants_attach || infos_[k].detached;
      wants_detach = wants_detach || !infos_[k].detached;
    }

  gtk_tool_item_set_visible_horizontal (attach_button_, wants_attach);
  gtk_tool_item_set_visible_vertical (attach_button_, wants_attach);
  gtk_tool_item_set_visible_horizontal (detach_button_, wants_detach);
  gtk_tool_item_set_visible_vertical (detach_button_, wants_detach);
}



