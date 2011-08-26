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
#include <string.h>

#include "xkbxml.h"
#include "ibus.h"

#define IBUS_XKB_LAYOUT_CONFIG_GET_PRIVATE(o)  \
   (G_TYPE_INSTANCE_GET_PRIVATE ((o), IBUS_TYPE_XKB_LAYOUT_CONFIG, IBusXKBLayoutConfigPrivate))

typedef struct _IBusXKBLayoutConfigPrivate IBusXKBLayoutConfigPrivate;

enum {
    PROP_0,
    PROP_SYSTEM_CONFIG_FILE,
};

struct _IBusXKBLayoutConfigPrivate {
    gchar *system_config_file;
    GList *preload_layouts;
};

/* functions prototype */
static void         ibus_xkb_layout_config_destroy
                                           (IBusXKBLayoutConfig *xkb_layout_config);

G_DEFINE_TYPE (IBusXKBLayoutConfig, ibus_xkb_layout_config, IBUS_TYPE_OBJECT)

static void
free_lang_list (GList *list)
{
    GList *l = list;
    while (l) {
        g_free (l->data);
        l->data = NULL;
        l = l->next;
    }
    g_list_free (list);
}

static GList *
parse_xkblayoutconfig_file (gchar *path)
{
    XMLNode *node = NULL;
    XMLNode *sub_node;
    XMLNode *sub_sub_node;
    GList *p;
    GList *retval = NULL;
    gchar **array;
    int i;

    node = ibus_xml_parse_file (path);
    if (node == NULL) {
        return NULL;
    }
    if (g_strcmp0 (node->name, "xkblayout") != 0) {
        ibus_xml_free (node);
        return NULL;
    }
    for (p = node->sub_nodes; p != NULL; p = p->next) {
        sub_node = (XMLNode *) p->data;
        if (g_strcmp0 (sub_node->name, "config") == 0) {
            GList *pp;
            for (pp = sub_node->sub_nodes; pp != NULL; pp = pp->next) {
                sub_sub_node = (XMLNode *) pp->data;
                if  (g_strcmp0 (sub_sub_node->name, "preload_layouts") == 0) {
                    if (sub_sub_node->text != NULL) {
                        array = g_strsplit ((gchar *) sub_sub_node->text,
                                            ",", -1);
                        for (i = 0; array[i]; i++) {
                            retval = g_list_append (retval, g_strdup (array[i]));
                        }
                        g_strfreev (array);
                        break;
                    }
                }
            }
        }
        if (retval != NULL) {
            break;
        }
    }

    ibus_xml_free (node);
    return retval;
}

static void
parse_xkb_layout_config (IBusXKBLayoutConfigPrivate *priv)
{
    gchar *basename;
    gchar *user_config;
    GList *list = NULL;

    g_return_if_fail (priv->system_config_file != NULL);

    basename = g_path_get_basename (priv->system_config_file);
    user_config = g_build_filename (g_get_user_config_dir (),
                                    "ibus", "xkb",
                                    basename, NULL);
    g_free (basename);
    list = parse_xkblayoutconfig_file (user_config);
    g_free (user_config);
    if (list) {
        priv->preload_layouts = list;
        return;
    }
    list = parse_xkblayoutconfig_file (priv->system_config_file);
    priv->preload_layouts = list;
}

static void
ibus_xkb_layout_config_init (IBusXKBLayoutConfig *xkb_layout_config)
{
    IBusXKBLayoutConfigPrivate *priv;

    priv = IBUS_XKB_LAYOUT_CONFIG_GET_PRIVATE (xkb_layout_config);
    priv->system_config_file = NULL;
    priv->preload_layouts = NULL;
}

static GObject *
ibus_xkb_layout_config_constructor (GType type,
                                    guint n_construct_params,
                                    GObjectConstructParam *construct_params)
{
    GObject *obj;
    IBusXKBLayoutConfig *xkb_layout_config;
    IBusXKBLayoutConfigPrivate *priv;

    obj = G_OBJECT_CLASS (ibus_xkb_layout_config_parent_class)->constructor (type, n_construct_params, construct_params);
    xkb_layout_config = IBUS_XKB_LAYOUT_CONFIG (obj);
    priv = IBUS_XKB_LAYOUT_CONFIG_GET_PRIVATE (xkb_layout_config);
    parse_xkb_layout_config (priv);

    return obj;
}

static void
ibus_xkb_layout_config_destroy (IBusXKBLayoutConfig *xkb_layout_config)
{
    IBusXKBLayoutConfigPrivate *priv;

    g_return_if_fail (xkb_layout_config != NULL);

    priv = IBUS_XKB_LAYOUT_CONFIG_GET_PRIVATE (xkb_layout_config);

    g_free (priv->system_config_file);
    priv->system_config_file = NULL;
    free_lang_list (priv->preload_layouts);
    priv->preload_layouts = NULL;
}

static void
ibus_xkb_layout_config_set_property (IBusXKBLayoutConfig *xkb_layout_config,
                                     guint                prop_id,
                                     const GValue        *value,
                                     GParamSpec          *pspec)
{
    IBusXKBLayoutConfigPrivate *priv;

    g_return_if_fail (xkb_layout_config != NULL);
    priv = IBUS_XKB_LAYOUT_CONFIG_GET_PRIVATE (xkb_layout_config);

    switch (prop_id) {
    case PROP_SYSTEM_CONFIG_FILE:
        g_assert (priv->system_config_file == NULL);
        priv->system_config_file = g_strdup (g_value_get_string (value));
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (xkb_layout_config, prop_id, pspec);
    }
}

static void
ibus_xkb_layout_config_get_property (IBusXKBLayoutConfig *xkb_layout_config,
                                     guint                prop_id,
                                     GValue              *value,
                                     GParamSpec          *pspec)
{
    IBusXKBLayoutConfigPrivate *priv;

    g_return_if_fail (xkb_layout_config != NULL);
    priv = IBUS_XKB_LAYOUT_CONFIG_GET_PRIVATE (xkb_layout_config);

    switch (prop_id) {
    case PROP_SYSTEM_CONFIG_FILE:
        g_value_set_string (value, priv->system_config_file);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (xkb_layout_config, prop_id, pspec);

    }
}

static void
ibus_xkb_layout_config_class_init (IBusXKBLayoutConfigClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    IBusObjectClass *ibus_object_class = IBUS_OBJECT_CLASS (klass);

    g_type_class_add_private (klass, sizeof (IBusXKBLayoutConfigPrivate));

    gobject_class->constructor = ibus_xkb_layout_config_constructor;
    gobject_class->set_property = (GObjectSetPropertyFunc) ibus_xkb_layout_config_set_property;
    gobject_class->get_property = (GObjectGetPropertyFunc) ibus_xkb_layout_config_get_property;
    ibus_object_class->destroy = (IBusObjectDestroyFunc) ibus_xkb_layout_config_destroy;

    /**
     * IBusProxy:interface:
     *
     * The interface of the proxy object.
     */
    g_object_class_install_property (gobject_class,
                    PROP_SYSTEM_CONFIG_FILE,
                    g_param_spec_string ("system_config_file",
                        "system_config_file",
                        "The system file of xkblayoutconfig",
                        NULL,
                        G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
}

IBusComponent *
ibus_xkb_component_new (void)
{
    IBusComponent *component;

    component = ibus_component_new ("org.freedesktop.IBus.XKB",
                                    "XKB Component",
                                    VERSION,
                                    "LGPL2.1",
                                    "Takao Fujiwara <takao.fujiwara1@gmail.com>",
                                    "http://code.google.com/p/ibus/",
                                    LIBEXECDIR "/ibus-engine-xkb --ibus",
                                    GETTEXT_PACKAGE);

    return component;
}

IBusEngineDesc *
ibus_xkb_engine_desc_new (const gchar *lang,
                          const gchar *layout,
                          const gchar *layout_desc,
                          const gchar *variant,
                          const gchar *variant_desc,
                          const gchar *alt_name)
{
    IBusEngineDesc *engine;
    gchar *name = NULL;
    gchar *longname = NULL;
    gchar *desc = NULL;
    gchar *engine_layout = NULL;
    const gchar *name_prefix = "xkb:layout:";
    const gchar *icon = "ibus-engine";

    g_return_val_if_fail (lang != NULL && layout != NULL, NULL);

    if (variant_desc) {
        longname = g_strdup (variant_desc);
    } else if (layout && variant) {
        longname = g_strdup_printf ("%s - %s", layout, variant);
    } else if (layout_desc) {
        longname = g_strdup (layout_desc);
    } else {
        longname = g_strdup (layout);
    }
    if (variant) {
        if (alt_name) {
            name = g_strdup (alt_name);
        } else {
            name = g_strdup_printf ("%s%s:%s", name_prefix, layout, variant);
        }
        desc = g_strdup_printf ("XKB %s(%s) keyboard layout", layout, variant);
        engine_layout = g_strdup_printf ("%s(%s)", layout, variant);
    } else {
        if (alt_name) {
            name = g_strdup (alt_name);
        } else {
            name = g_strdup_printf ("%s%s", name_prefix, layout);
        }
        desc = g_strdup_printf ("XKB %s keyboard layout", layout);
        engine_layout = g_strdup (layout);
    }
#if USE_BRIDGE_HOTKEY
    if (g_ascii_strncasecmp (name, DEFAULT_BRIDGE_ENGINE_NAME,
                             strlen (DEFAULT_BRIDGE_ENGINE_NAME)) == 0) {
        icon = "input-keyboard-symbolic";
    }
#endif

    engine = ibus_engine_desc_new (name,
                                   longname,
                                   desc,
                                   lang,
                                   "LGPL2.1",
                                   "Takao Fujiwara <takao.fujiwara1@gmail.com>",
                                   icon,
                                   engine_layout);

    g_free (name);
    g_free (longname);
    g_free (desc);
    g_free (engine_layout);

    return engine;
}

IBusXKBLayoutConfig *
ibus_xkb_layout_config_new (const gchar *system_config_file)
{
    IBusXKBLayoutConfig *xkb_layout_config;

    xkb_layout_config = IBUS_XKB_LAYOUT_CONFIG (g_object_new (IBUS_TYPE_XKB_LAYOUT_CONFIG,
                                                              "system_config_file",
                                                              system_config_file,
                                                              NULL));
    return xkb_layout_config;
}

const GList *
ibus_xkb_layout_config_get_preload_layouts (IBusXKBLayoutConfig *xkb_layout_config)
{
    IBusXKBLayoutConfigPrivate *priv;

    g_return_val_if_fail (xkb_layout_config != NULL, NULL);
    priv = IBUS_XKB_LAYOUT_CONFIG_GET_PRIVATE (xkb_layout_config);
    return (const GList *) priv->preload_layouts;
}
