/* -*- mode: C; c-basic-offset: 4; indent-tabs-mode: nil; -*- */
/* vim:set et sts=4: */
/* ibus-xkb - IBus XKB
 * Copyright (C) 2011-2012 Takao Fujiwara <takao.fujiwara1@gmail.com>
 * Copyright (C) 2011 Peng Huang <shawn.p.huang@gmail.com>
 * Copyright (C) 2011-2012 Red Hat, Inc.
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

#include <ibus.h>
#include <glib/gi18n-lib.h>
#include <stdlib.h>

#ifdef ENABLE_NLS
#include <locale.h>
#endif

#include "xkbxml.h"
#include "ibusxkbxml.h"
#include "ibus-simple-engine.h"


static IBusBus *bus = NULL;
static IBusFactory *factory = NULL;
static gboolean ibus = FALSE;
static gboolean xml = FALSE;

static const GOptionEntry entries[] =
{
    { "ibus", 'i', 0, G_OPTION_ARG_NONE, &ibus, "component is executed by ibus", NULL },
    { "xml", 'x', 0, G_OPTION_ARG_NONE, &xml, "print component xml", NULL },
    { NULL },
};

static void
ibus_disconnected_cb (IBusBus  *bus,
                      gpointer  user_data)
{
    g_debug ("bus disconnected");
    ibus_quit ();
}

IBusEngine *
_factory_create_engine_cb (IBusFactory *factory,
                           const gchar *engine_name,
                           gpointer     data)
{
    IBusEngine *engine = NULL;
    gchar *object_path = NULL;
    static int id = 0;

    g_return_val_if_fail (engine_name != NULL, NULL);

    if (g_strcmp0 (engine_name, "xkb:layout:us") == 0) {
        return NULL;
    }

    object_path = g_strdup_printf ("/org/freedesktop/IBus/XKBEngine/%d",
                                   ++id);
    engine = ibus_engine_new_with_type (IBUS_TYPE_SIMPLE_ENGINE,
                                        engine_name,
                                        object_path,
                                        ibus_service_get_connection (IBUS_SERVICE (factory)));
    g_free (object_path);
    return engine;
}

static void
start_component (int argc, char **argv)
{
    IBusComponent *component;

    ibus_init ();

    bus = ibus_bus_new ();
    g_signal_connect (bus, "disconnected", G_CALLBACK (ibus_disconnected_cb), NULL);

    component = ibus_component_new ("org.freedesktop.IBus.XKB",
                                    "XKB Component",
                                    VERSION,
                                    "LGPL2.1",
                                    "Takao Fujiwara <takao.fujiwara1@gmail.com>",
                                    "http://code.google.com/p/ibus/",
                                    "",
                                    GETTEXT_PACKAGE);
    ibus_component_add_engine (component,
                               ibus_xkb_engine_desc_new ("eng",
                                                         "us",
                                                         "USA",
                                                         NULL,
                                                         NULL,
                                                         NULL));

    factory = ibus_factory_new (ibus_bus_get_connection (bus));

    ibus_factory_add_engine (factory, "xkb:layout:us", IBUS_TYPE_SIMPLE_ENGINE);

    g_signal_connect (G_OBJECT (factory), "create-engine",
                      G_CALLBACK (_factory_create_engine_cb),
                      NULL);
    if (ibus) {
        ibus_bus_request_name (bus, "org.freedesktop.IBus.XKB", 0);
    }
    else {
        ibus_bus_register_component (bus, component);
    }

    g_object_unref (component);

    ibus_main ();
}

static gboolean
is_included_engine_in_preload (const GList * preload_xkb_engines,
                               const gchar *layout,
                               const gchar *variant)
{
    const GList *list = preload_xkb_engines;
    gchar *key = NULL;
    gboolean retval = FALSE;

    g_return_val_if_fail (layout != NULL, FALSE);

    if (variant == NULL) {
        key = g_strdup (layout);
    } else {
        key = g_strdup_printf ("%s(%s)", layout, variant);
    }
    while (list) {
        if (list->data == NULL) {
            continue;
        }
        if (g_strcmp0 ((const gchar *) list->data,
                       (const gchar *) key) == 0) {
            retval = TRUE;
            break;
        }
        list = list->next;
    }
    g_free (key);
    return retval;
}

static void
print_component ()
{
    IBusXKBLayoutConfig *layout_config;
    IBusXKBConfigRegistry *config_registry;
    GHashTable *layout_list;
    GHashTable *layout_lang;
    GHashTable *layout_desc;
    GHashTable *variant_desc;
    IBusComponent *component;
    IBusEngineDesc *engine;
    const GList *preload_xkb_engines = NULL;
    GList *keys;
    GList *variants;
    GList *langs;
    gboolean is_preload;
    gchar *layout_name;
    const gchar *desc;
    gchar *output;
    GString *str;
#if USE_BRIDGE_HOTKEY
    int i;
#endif

#ifdef XKBLAYOUTCONFIG_FILE
    layout_config = ibus_xkb_layout_config_new (XKBLAYOUTCONFIG_FILE);
    preload_xkb_engines = ibus_xkb_layout_config_get_preload_layouts (layout_config);
#endif

    config_registry = ibus_xkb_config_registry_new ();
    layout_list = (GHashTable *) ibus_xkb_config_registry_get_layout_list (config_registry);
    layout_lang = (GHashTable *) ibus_xkb_config_registry_get_layout_lang (config_registry);
    layout_desc = (GHashTable *) ibus_xkb_config_registry_get_layout_desc (config_registry);
    variant_desc = (GHashTable *) ibus_xkb_config_registry_get_variant_desc (config_registry);
    component = ibus_xkb_component_new ();
#if USE_BRIDGE_HOTKEY
    for (i = 0; i < 4; i++) {
        gchar *name = g_strdup_printf ("%s#%d", DEFAULT_BRIDGE_ENGINE_NAME, i);
        engine = ibus_xkb_engine_desc_new ("eng",
                                           "us",
                                           _("Default Layout"),
                                           NULL,
                                           NULL,
                                           name);
        g_free (name);
        ibus_component_add_engine (component, engine);
    }
#endif
    for (keys = g_hash_table_get_keys (layout_list); keys; keys = keys->next) {
        if (keys->data == NULL) {
            continue;
        }
        desc = (const gchar *) g_hash_table_lookup (layout_desc, keys->data);
        langs = (GList *) g_hash_table_lookup (layout_lang, keys->data);
        for (;langs; langs = langs->next) {
            if (langs->data == NULL) {
                continue;
            }
            is_preload = FALSE;
            if (!preload_xkb_engines) {
                is_preload = TRUE;
            } else {
                is_preload = is_included_engine_in_preload (preload_xkb_engines,
                                                            (const gchar *) keys->data,
                                                            NULL);
            }
            if (is_preload) {
                engine = ibus_xkb_engine_desc_new ((const gchar *) langs->data,
                                                   (const gchar *) keys->data,
                                                   desc,
                                                   NULL,
                                                   NULL,
                                                   NULL);
                ibus_component_add_engine (component, engine);
            }
        }
        variants = (GList *) g_hash_table_lookup (layout_list, keys->data);
        for (;variants; variants = variants->next) {
            if (variants->data == NULL) {
                continue;
            }
            layout_name = g_strdup_printf ("%s(%s)", (gchar *) keys->data,
                                                     (gchar *) variants->data);
            langs = (GList *) g_hash_table_lookup (layout_lang, layout_name);
            if (langs == NULL) {
                g_free (layout_name);
                layout_name = g_strdup ((gchar *) keys->data);
                langs = (GList *) g_hash_table_lookup (layout_lang, layout_name);
            }
            g_free (layout_name);
            for (;langs; langs = langs->next) {
                if (langs->data == NULL) {
                    continue;
                }
                is_preload = FALSE;
                if (!preload_xkb_engines) {
                   is_preload = TRUE;
                } else {
                    is_preload = is_included_engine_in_preload (preload_xkb_engines,
                                                                (const gchar *) keys->data,
                                                                (const gchar *) variants->data);
                }
                if (is_preload) {
                    engine = ibus_xkb_engine_desc_new ((const gchar *) langs->data,
                                                       (const gchar *) keys->data,
                                                       desc,
                                                       (const gchar *) variants->data,
                                                       (const gchar *) g_hash_table_lookup (variant_desc, variants->data),
                                                       NULL);
                    ibus_component_add_engine (component, engine);
                }
            }
        }
    }
    g_object_unref (G_OBJECT (config_registry));
#ifdef XKBLAYOUTCONFIG_FILE
    g_object_unref (G_OBJECT (layout_config));
#endif

    str = g_string_new (NULL);
    ibus_component_output_engines (component , str, 0);
    g_object_unref (G_OBJECT (component));

    output = g_string_free (str, FALSE);
    g_print ("%s\n", output);
    g_free (output);
}

int
main (int argc, char **argv)
{
    GError *error = NULL;
    GOptionContext *context;

#ifdef ENABLE_NLS
    setlocale (LC_ALL, "");
    bindtextdomain (GETTEXT_PACKAGE, IBUS_LOCALEDIR);
    bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
#endif

    g_type_init ();

    context = g_option_context_new ("- ibus xkb engine component");

    g_option_context_add_main_entries (context, entries, "ibus-xbl");

    if (!g_option_context_parse (context, &argc, &argv, &error)) {
        g_print ("Option parsing failed: %s\n", error->message);
        exit (-1);
    }

    if (xml) {
        print_component ();
        return 0;
    }
    start_component (argc, argv);

    return 0;
}
