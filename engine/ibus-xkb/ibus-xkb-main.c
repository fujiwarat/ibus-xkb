/* -*- mode: C; c-basic-offset: 4; indent-tabs-mode: nil; -*- */
/* vim:set et sts=4: */
/* bus - The Input Bus
 * Copyright (C) 2012 Takao Fujiwara <takao.fujiwara1@gmail.com>
 * Copyright (C) 2012 Peng Huang <shawn.p.huang@gmail.com>
 * Copyright (C) 2012 Red Hat, Inc.
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

static gboolean query = FALSE;
static gboolean get_group = FALSE;
static gchar *layout = NULL;
static gchar *variant = NULL;
static gchar *option = NULL;
static int group = 0;

static const GOptionEntry entries[] =
{
    { "get", 'g', 0, G_OPTION_ARG_NONE, &query, N_("Query the current xkb layout"), NULL },
    { "layout", 'l', 0, G_OPTION_ARG_STRING, &layout, N_("Set xkb LAYOUT"), N_("LAYOUT") },
    { "variant", 'v', 0, G_OPTION_ARG_STRING, &variant, N_("Set xkb VARIANT"), N_("VARIANT") },
    { "option", 'o', 0, G_OPTION_ARG_STRING, &option, N_("Set xkb OPTION"), N_("OPTION") },
    { "get-group", 'G', 0, G_OPTION_ARG_NONE, &get_group, N_("Get current xkb state"), NULL },
    { NULL },
};


gboolean
parse_setxkbmap_args (int *pargc, char ***pargv)
{
    int argc = *pargc;
    char **argv = *pargv;
    char **new_argv = NULL;
    int n = 1;
    int i;
    gboolean retval = FALSE;

    for (i = 0; i < argc; i++) {
        if (g_strcmp0 (argv[i], "-layout") == 0 && i + 1 < argc) {
            g_free (layout);
            layout = g_strdup (argv[i + 1]);
            i++;
            retval = TRUE;
            continue;
        }
        if (g_strcmp0 (argv[i], "-variant") == 0 && i + 1 < argc) {
            g_free (variant);
            variant = g_strdup (argv[i + 1]);
            i++;
            retval = TRUE;
            continue;
        }
        if (g_strcmp0 (argv[i], "-option") == 0 && i + 1 < argc) {
            g_free (option);
            option = g_strdup (argv[i + 1]);
            i++;
            retval = TRUE;
            continue;
        }
        if (g_strcmp0 (argv[i], "-query") == 0) {
            query = TRUE;
            retval = TRUE;
            continue;
        }

        new_argv = g_renew(char*, new_argv, n);
        new_argv[n - 1] = g_strdup (argv[i]);
        n++;
    }

    *pargc = n - 1;
    *pargv = new_argv;

    return retval;
}

int
main (int argc, char *argv[])
{
    gboolean parsed;
    GOptionContext *context;
    GError *error = NULL;
    Display *xdisplay;

#ifdef ENABLE_NLS
    setlocale (LC_ALL, "");

    bindtextdomain (GETTEXT_PACKAGE, GLIB_LOCALE_DIR);
    bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
#endif

    parsed = parse_setxkbmap_args (&argc, &argv);

    context = g_option_context_new ("- ibus daemon");

    g_option_context_set_translation_domain (context, GETTEXT_PACKAGE);
    g_option_context_add_main_entries (context, entries, GETTEXT_PACKAGE);

    if (!parsed && !g_option_context_parse (context, &argc, &argv, &error)) {
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
        if (option == NULL) {
            option = ibus_xkb_get_current_option ();
        }

        ibus_xkb_set_layout (layout, variant, option);
    }

    if (query) {
        g_free (layout);
        g_free (variant);
        g_free (option);

        layout = ibus_xkb_get_current_layout ();
        variant = ibus_xkb_get_current_variant ();
        option = ibus_xkb_get_current_option ();
        g_printf ("layout: %s\n"
                  "variant: %s\n"
                  "option: %s\n",
                  layout ? layout : "",
                  variant ? variant : "",
                  option ? option : "");
        g_free (layout);
        g_free (variant);
        g_free (option);
    }
    if (get_group) {
        group = ibus_xkb_get_current_group ();
        g_printf ("group: %d\n", group);
    }

    ibus_xkb_finit ();

    return 0;
}
