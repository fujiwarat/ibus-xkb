/* -*- mode: C; c-basic-offset: 4; indent-tabs-mode: nil; -*- */
/* vim:set et sts=4: */
/* ibus-xkb - IBus XKB
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

#include "ibus.h"
#include "ibusxkbxml.h"

#ifndef XKB_RULES_XML_FILE
#define XKB_RULES_XML_FILE "/usr/share/X11/xkb/rules/evdev.xml"
#endif

#define IBUS_XKB_CONFIG_REGISTRY_GET_PRIVATE(o)  \
   (G_TYPE_INSTANCE_GET_PRIVATE ((o), IBUS_TYPE_XKB_CONFIG_REGISTRY, IBusXKBConfigRegistryPrivate))

typedef struct _IBusXKBConfigRegistryPrivate IBusXKBConfigRegistryPrivate;

struct _IBusXKBConfigRegistryPrivate {
    GHashTable *layout_list;
    GHashTable *layout_lang;
    GHashTable *layout_desc;
    GHashTable *variant_desc;
};


/* functions prototype */
static void         ibus_xkb_config_registry_destroy
                                           (IBusXKBConfigRegistry *xkb_config);

G_DEFINE_TYPE (IBusXKBConfigRegistry, ibus_xkb_config_registry, IBUS_TYPE_OBJECT)

static void
parse_xkb_xml_languagelist_node (IBusXKBConfigRegistryPrivate *priv,
                                 XMLNode *parent_node,
                                 const gchar *layout_name)
{
    XMLNode *node = parent_node;
    XMLNode *sub_node;
    GList *p;
    GList *lang_list = NULL;

    g_assert (node != NULL);
    g_assert (layout_name != NULL);
    for (p = node->sub_nodes; p; p = p->next) {
        sub_node = (XMLNode *) p->data;
        if (g_strcmp0 (sub_node->name, "iso639Id") == 0) {
            lang_list = g_list_append (lang_list,
                                       (gpointer) g_strdup (sub_node->text));
            continue;
        }
    }
    if (lang_list == NULL) {
        /* some nodes have no lang */
        return;
    }
    if (g_hash_table_lookup (priv->layout_lang, layout_name) != NULL) {
        g_warning ("duplicated name %s exists", layout_name);
        return;
    }
    g_hash_table_insert (priv->layout_lang,
                         (gpointer) g_strdup (layout_name),
                         (gpointer) lang_list);
}

static const gchar *
parse_xkb_xml_configitem_node (IBusXKBConfigRegistryPrivate *priv,
                               XMLNode *parent_node)
{
    XMLNode *node = parent_node;
    XMLNode *sub_node;
    GList *p;
    gchar *name = NULL;
    gchar *description = NULL;

    g_assert (node != NULL);
    for (p = node->sub_nodes; p; p = p->next) {
        sub_node = (XMLNode *) p->data;
        if (g_strcmp0 (sub_node->name, "name") == 0) {
            name = sub_node->text;
            continue;
        }
        if (g_strcmp0 (sub_node->name, "description") == 0) {
            description = sub_node->text;
            continue;
        }
        if (g_strcmp0 (sub_node->name, "languageList") == 0) {
            if (name == NULL) {
                g_warning ("layout name is NULL in node %s", node->name);
                continue;
            }
            parse_xkb_xml_languagelist_node (priv, sub_node, name);
            continue;
        }
    }
    if (name == NULL) {
        g_warning ("No name in layout node");
        return NULL;
    }
    if (g_hash_table_lookup (priv->layout_desc, name) != NULL) {
        g_warning ("duplicated name %s exists", name);
        return name;
    }
    g_hash_table_insert (priv->layout_desc,
                         (gpointer) g_strdup (name),
                         (gpointer) g_strdup (description));

    return name;
}

static const gchar *
parse_xkb_xml_variant_configitem_node (IBusXKBConfigRegistryPrivate *priv,
                            XMLNode *parent_node,
                            const gchar *layout_name)
{
    XMLNode *node = parent_node;
    XMLNode *sub_node;
    GList *p;
    gchar *name = NULL;
    gchar *description = NULL;
    gchar *variant_lang_name = NULL;

    g_assert (node != NULL);
    g_assert (layout_name != NULL);
    for (p = node->sub_nodes; p; p = p->next) {
        sub_node = (XMLNode *) p->data;
        if (g_strcmp0 (sub_node->name, "name") == 0) {
            name = sub_node->text;
            continue;
        }
        if (g_strcmp0 (sub_node->name, "description") == 0) {
            description = sub_node->text;
            continue;
        }
        if (g_strcmp0 (sub_node->name, "languageList") == 0) {
            if (name == NULL) {
                g_warning ("layout name is NULL in node %s", node->name);
                continue;
            }
            variant_lang_name = g_strdup_printf ("%s(%s)", layout_name, name);
            parse_xkb_xml_languagelist_node (priv, sub_node, variant_lang_name);
            g_free (variant_lang_name);
            continue;
        }
    }
    if (name == NULL) {
        g_warning ("No name in layout node");
        return NULL;
    }
    if (g_hash_table_lookup (priv->variant_desc, name) != NULL) {
        /* This is an expected case. */
        return name;
    }
    variant_lang_name = g_strdup_printf ("%s(%s)", layout_name, name);
    g_hash_table_insert (priv->variant_desc,
                         (gpointer) variant_lang_name,
                         (gpointer) g_strdup (description));
    return name;
}

static const gchar *
parse_xkb_xml_variant_node (IBusXKBConfigRegistryPrivate *priv,
                            XMLNode *parent_node,
                            const gchar *layout_name)
{
    XMLNode *node = parent_node;
    XMLNode *sub_node;
    GList *p;
    const gchar *variant_name = NULL;

    g_assert (node != NULL);
    g_assert (layout_name != NULL);
    for (p = node->sub_nodes; p; p = p->next) {
        sub_node = (XMLNode *) p->data;
        if (g_strcmp0 (sub_node->name, "configItem") == 0) {
            variant_name = parse_xkb_xml_variant_configitem_node (priv, sub_node, layout_name);
            continue;
        }
    }
    return variant_name;
}

static GList *
parse_xkb_xml_variantlist_node (IBusXKBConfigRegistryPrivate *priv,
                                XMLNode *parent_node,
                                const gchar *layout_name,
                                GList *variant_list)
{
    XMLNode *node = parent_node;
    XMLNode *sub_node;
    GList *p;
    const gchar *variant_name = NULL;

    g_assert (node != NULL);
    g_assert (layout_name != NULL);
    for (p = node->sub_nodes; p; p = p->next) {
        sub_node = (XMLNode *) p->data;
        if (g_strcmp0 (sub_node->name, "variant") == 0) {
            variant_name = parse_xkb_xml_variant_node (priv, sub_node, layout_name);
            if (variant_name != NULL) {
                variant_list = g_list_append (variant_list,
                                              (gpointer) g_strdup (variant_name));
            }
            continue;
        }
    }
    return variant_list;
}

static void
parse_xkb_xml_layout_node (IBusXKBConfigRegistryPrivate *priv,
                           XMLNode *parent_node)
{
    XMLNode *node = parent_node;
    XMLNode *sub_node;
    GList *p;
    const gchar *name = NULL;
    GList *variant_list = NULL;

    g_assert (node != NULL);
    for (p = node->sub_nodes; p; p = p->next) {
        sub_node = (XMLNode *) p->data;
        if (g_strcmp0 (sub_node->name, "configItem") == 0) {
            name = parse_xkb_xml_configitem_node (priv, sub_node);
            continue;
        }
        if (g_strcmp0 (sub_node->name, "variantList") == 0) {
            if (name == NULL) {
                g_warning ("layout name is NULL in node %s", node->name);
                continue;
            }
            variant_list = parse_xkb_xml_variantlist_node (priv, sub_node,
                                                           name,
                                                           variant_list);
            continue;
        }
    }
    if (g_hash_table_lookup (priv->layout_list, name) != NULL) {
        g_warning ("duplicated name %s exists", name);
        return;
    }
    g_hash_table_insert (priv->layout_list,
                         (gpointer) g_strdup (name),
                         (gpointer) variant_list);
}

static void
parse_xkb_xml_top_node (IBusXKBConfigRegistryPrivate *priv,
                        XMLNode *parent_node)
{
    XMLNode *node = parent_node;
    XMLNode *sub_node;
    GList *p;

    g_assert (priv != NULL);
    g_assert (node != NULL);

    if (g_strcmp0 (node->name, "xkbConfigRegistry") != 0) {
        g_warning ("node has no xkbConfigRegistry name");
        return;
    }
    for (p = node->sub_nodes; p; p = p->next) {
        sub_node = (XMLNode *) p->data;
        if (g_strcmp0 (sub_node->name, "layoutList") == 0) {
            break;
        }
    }
    if (p == NULL) {
        g_warning ("xkbConfigRegistry node has no layoutList node");
        return;
    }
    node = sub_node;
    for (p = node->sub_nodes; p; p = p->next) {
        sub_node = (XMLNode *) p->data;
        if (g_strcmp0 (sub_node->name, "layout") == 0) {
            parse_xkb_xml_layout_node (priv, sub_node);
            continue;
        }
    }
}

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

static void
parse_xkb_config_registry_file (IBusXKBConfigRegistryPrivate *priv,
                                const gchar *file)
{
    XMLNode *node;

    g_assert (file != NULL);

    priv->layout_list = g_hash_table_new_full (g_str_hash,
                                               (GEqualFunc) g_str_equal,
                                               (GDestroyNotify) g_free,
                                               (GDestroyNotify) free_lang_list);
    priv->layout_desc = g_hash_table_new_full (g_str_hash,
                                               (GEqualFunc) g_str_equal,
                                               (GDestroyNotify) g_free,
                                               (GDestroyNotify) g_free);
    priv->layout_lang = g_hash_table_new_full (g_str_hash,
                                               (GEqualFunc) g_str_equal,
                                               (GDestroyNotify) g_free,
                                               (GDestroyNotify) free_lang_list);
    priv->variant_desc = g_hash_table_new_full (g_str_hash,
                                               (GEqualFunc) g_str_equal,
                                               (GDestroyNotify) g_free,
                                               (GDestroyNotify) g_free);
    node = ibus_xml_parse_file (file);
    parse_xkb_xml_top_node (priv, node);
    ibus_xml_free (node);
}

static void
ibus_xkb_config_registry_init (IBusXKBConfigRegistry *xkb_config)
{
    IBusXKBConfigRegistryPrivate *priv;
    const gchar *file = XKB_RULES_XML_FILE;

    priv = IBUS_XKB_CONFIG_REGISTRY_GET_PRIVATE (xkb_config);
    parse_xkb_config_registry_file (priv, file);
}

static void
ibus_xkb_config_registry_destroy (IBusXKBConfigRegistry *xkb_config)
{
    IBusXKBConfigRegistryPrivate *priv;

    g_return_if_fail (xkb_config != NULL);

    priv = IBUS_XKB_CONFIG_REGISTRY_GET_PRIVATE (xkb_config);

    g_hash_table_destroy (priv->layout_list);
    priv->layout_list = NULL;
    g_hash_table_destroy (priv->layout_lang);
    priv->layout_lang= NULL;
    g_hash_table_destroy (priv->layout_desc);
    priv->layout_desc= NULL;
    g_hash_table_destroy (priv->variant_desc);
    priv->variant_desc = NULL;

    IBUS_OBJECT_CLASS(ibus_xkb_config_registry_parent_class)->destroy (IBUS_OBJECT (xkb_config));
}

static void
ibus_xkb_config_registry_class_init (IBusXKBConfigRegistryClass *klass)
{
    IBusObjectClass *ibus_object_class = IBUS_OBJECT_CLASS (klass);

    g_type_class_add_private (klass, sizeof (IBusXKBConfigRegistryPrivate));

    ibus_object_class->destroy = (IBusObjectDestroyFunc) ibus_xkb_config_registry_destroy;
}

IBusXKBConfigRegistry *
ibus_xkb_config_registry_new (void)
{
    IBusXKBConfigRegistry *xkb_config;

    xkb_config = IBUS_XKB_CONFIG_REGISTRY (g_object_new (IBUS_TYPE_XKB_CONFIG_REGISTRY, NULL));
    return xkb_config;
}

#define TABLE_FUNC(field_name) const GHashTable *                       \
ibus_xkb_config_registry_get_##field_name  (IBusXKBConfigRegistry *xkb_config) \
{                                                                       \
    IBusXKBConfigRegistryPrivate *priv;                                 \
                                                                        \
    g_return_val_if_fail (xkb_config != NULL, NULL);                    \
    priv = IBUS_XKB_CONFIG_REGISTRY_GET_PRIVATE (xkb_config);           \
    return priv->field_name;                                            \
}

TABLE_FUNC (layout_list)
TABLE_FUNC (layout_lang)
TABLE_FUNC (layout_desc)
TABLE_FUNC (variant_desc)

#undef TABLE_FUNC

GList *
ibus_xkb_config_registry_layout_list_get_layouts (IBusXKBConfigRegistry *xkb_config)
{
    GHashTable *table;
    GList *list = NULL;

    table = (GHashTable *)
        ibus_xkb_config_registry_get_layout_list (xkb_config);
    list = (GList *) g_hash_table_get_keys (table);
    return list;
}

/* vala could use GLib.List<string> for the returned pointer and
 * the declaration calls g_list_foreach (retval, g_free, NULL).
 * When I think about GLib.List<string> v.s. GLib.List, probably
 * I think GLib.List<string> is better for the function and set
 * g_strdup() here. I do not know about GJS implementation.
 */
#define TABLE_LOOKUP_LIST_FUNC(field_name, value) GList *               \
ibus_xkb_config_registry_##field_name##_get_##value  (IBusXKBConfigRegistry *xkb_config, const gchar *key) \
{                                                                       \
    GHashTable *table;                                                  \
    GList *list = NULL;                                                 \
    GList *retval= NULL;                                                \
    GList *p = NULL;                                                    \
                                                                        \
    table = (GHashTable *)                                              \
        ibus_xkb_config_registry_get_##field_name (xkb_config);         \
    list = (GList *) g_hash_table_lookup (table, key);                  \
    retval = g_list_copy (list);                                        \
    for (p = retval; p; p = p->next) {                                  \
        p->data = g_strdup (p->data);                                   \
    }                                                                   \
    return retval;                                                      \
}

#define TABLE_LOOKUP_STRING_FUNC(field_name, value) gchar *             \
ibus_xkb_config_registry_##field_name##_get_##value  (IBusXKBConfigRegistry *xkb_config, const gchar *key) \
{                                                                       \
    GHashTable *table;                                                  \
    const gchar *desc = NULL;                                           \
                                                                        \
    table = (GHashTable *)                                              \
        ibus_xkb_config_registry_get_##field_name (xkb_config);         \
    desc = (const gchar *) g_hash_table_lookup (table, key);            \
    return g_strdup (desc);                                             \
}

TABLE_LOOKUP_LIST_FUNC (layout_list, variants)
TABLE_LOOKUP_LIST_FUNC (layout_lang, langs)
TABLE_LOOKUP_STRING_FUNC (layout_desc, desc)
TABLE_LOOKUP_STRING_FUNC (variant_desc, desc)

#undef TABLE_LOOKUP_LIST_FUNC
#undef TABLE_LOOKUP_STRING_FUNC
