/* -*- mode: C; c-basic-offset: 4; indent-tabs-mode: nil; -*- */
/* vim:set et sts=4: */
/* ibus-xkb - IBus XKB
 * Copyright (C) 2011 Takao Fujiwara <takao.fujiwara1@gmail.com>
 * Copyright (C) 2011 Peng Huang <shawn.p.huang@gmail.com>
 * Copyright (C) 2011 Red Hat, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib.h>
#include <glib/gprintf.h>
#include <glib/gi18n.h>
#include <X11/Xlib.h>

#ifdef ENABLE_NLS
#include <locale.h>
#endif

#include "xkblib.h"

static gboolean get_layout = FALSE;
static gboolean get_group = FALSE;
static gchar *layout = NULL;
static gchar *model = NULL;
static gchar *option = NULL;
static int group = 0;

static const GOptionEntry entries[] =
{
    { "get", 'g', 0, G_OPTION_ARG_NONE, &get_layout, N_("Get current xkb layout"), NULL },
    /* Translators: the "layout" should not be translated due to a variable. */
    { "layout", 'l', 0, G_OPTION_ARG_STRING, &layout, N_("Set xkb layout"), "layout" },
    { "model", 'm', 0, G_OPTION_ARG_STRING, &model, N_("Set xkb model"), "model" },
    { "option", 'o', 0, G_OPTION_ARG_STRING, &option, N_("Set xkb option"), "option" },
    { "get-group", 'G', 0, G_OPTION_ARG_NONE, &get_group, N_("Get current xkb state"), NULL },
    { NULL },
};

int
main (int argc, char *argv[])
{
    GOptionContext *context;
    GError *error = NULL;
    Display *xdisplay;

#ifdef ENABLE_NLS
    setlocale (LC_ALL, "");

    bindtextdomain (GETTEXT_PACKAGE, IBUS_LOCALEDIR);
    bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
#endif

    context = g_option_context_new ("- ibus daemon");

    g_option_context_set_translation_domain (context, GETTEXT_PACKAGE);
    g_option_context_add_main_entries (context, entries, GETTEXT_PACKAGE);

    if (!g_option_context_parse (context, &argc, &argv, &error)) {
        g_printerr ("Option parsing failed: %s\n", error->message);
        return -1;
    }

    xdisplay = XOpenDisplay (NULL);
    if (xdisplay == NULL) {
        g_warning ("Could not open display");
        return -1;
    }
    ibus_xkb_init (xdisplay);

    if (layout) {
        ibus_xkb_set_layout (layout, model, option);
    }
    if (get_layout) {
        layout = ibus_xkb_get_current_layout ();
        model = ibus_xkb_get_current_model ();
        option = ibus_xkb_get_current_option ();
        g_printf ("layout: %s\n"
                  "model: %s\n"
                  "option: %s\n",
                  layout ? layout : "",
                  model ? model : "",
                  option ? option : "");
        g_free (layout);
        g_free (model);
        g_free (option);
    }
    if (get_group) {
        group = ibus_xkb_get_current_group ();
        g_printf ("group: %d\n", group);
    }

    ibus_xkb_finit ();

    return 0;
}
