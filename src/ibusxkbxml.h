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
#ifndef __IBUS_XKBXML_H_
#define __IBUS_XKBXML_H_

#include "ibus.h"

/*
 * Type macros.
 */
/* define IBusXKBConfigRegistry macros */
#define IBUS_XKB_TYPE_CONFIG_REGISTRY                   \
    (ibus_xkb_config_registry_get_type ())
#define IBUS_XKB_CONFIG_REGISTRY(obj)                   \
    (G_TYPE_CHECK_INSTANCE_CAST ((obj), IBUS_XKB_TYPE_CONFIG_REGISTRY, IBusXKBConfigRegistry))
#define IBUS_XKB_CONFIG_REGISTRY_CLASS(klass)           \
    (G_TYPE_CHECK_CLASS_CAST ((klass), IBUS_XKB_TYPE_CONFIG_REGISTRY, IBusXKBConfigRegistryClass))
#define IBUS_IS_XKB_CONFIG_REGISTRY(obj)                \
    (G_TYPE_CHECK_INSTANCE_TYPE ((obj), IBUS_XKB_TYPE_CONFIG_REGISTRY))
#define IBUS_IS_XKB_CONFIG_REGISTRY_CLASS(klass)        \
    (G_TYPE_CHECK_CLASS_TYPE ((klass), IBUS_XKB_TYPE_CONFIG_REGISTRY))
#define IBUS_XKB_CONFIG_REGISTRY_GET_CLASS(obj)         \
    (G_TYPE_INSTANCE_GET_CLASS ((obj), IBUS_XKB_TYPE_CONFIG_REGISTRY, IBusXKBConfigRegistryClass))

G_BEGIN_DECLS

typedef struct _IBusXKBConfigRegistry IBusXKBConfigRegistry;
typedef struct _IBusXKBConfigRegistryClass IBusXKBConfigRegistryClass;

struct _IBusXKBConfigRegistry {
    IBusObject parent;
};

struct _IBusXKBConfigRegistryClass {
    IBusObjectClass parent;
    /* signals */
    /*< private >*/
    /* padding */
    gpointer pdummy[8];
};


GType            ibus_xkb_config_registry_get_type
                                                 (void);

/**
 * ibus_xkb_config_registry_new:
 * @returns: A newly allocated IBusXKBConfigRegistry
 *
 * New an IBusXKBConfigRegistry.
 */
IBusXKBConfigRegistry *
                 ibus_xkb_config_registry_new
                                                 (void);

/**
 * ibus_xkb_config_registry_get_layout_list: (skip)
 * @xkb_config: An IBusXKBConfigRegistry.
 * @returns: A const GHashTable
 *
 * a const GHashTable
 */
const GHashTable *
                 ibus_xkb_config_registry_get_layout_list
                                                 (IBusXKBConfigRegistry *xkb_config);

/**
 * ibus_xkb_config_registry_get_layout_lang: (skip)
 * @xkb_config: An IBusXKBConfigRegistry.
 * @returns: A const GHashTable
 *
 * a const GHashTable
 */
const GHashTable *
                 ibus_xkb_config_registry_get_layout_lang
                                                 (IBusXKBConfigRegistry *xkb_config);

/**
 * ibus_xkb_config_registry_get_layout_desc: (skip)
 * @xkb_config: An IBusXKBConfigRegistry.
 * @returns: A const GHashTable
 *
 * a const GHashTable
 */
const GHashTable *
                 ibus_xkb_config_registry_get_layout_desc
                                                 (IBusXKBConfigRegistry *xkb_config);

/**
 * ibus_xkb_config_registry_get_variant_desc: (skip)
 * @xkb_config: An IBusXKBConfigRegistry.
 * @returns: A const GHashTable
 *
 * a const GHashTable
 */
const GHashTable *
                 ibus_xkb_config_registry_get_variant_desc
                                                 (IBusXKBConfigRegistry *xkb_config);

/**
 * ibus_xkb_config_registry_layout_list_get_layouts:
 * @xkb_config: An IBusXKBConfigRegistry.
 * @returns: (transfer container) (element-type utf8): A GList of layouts
 *
 * a GList of layouts
 */
GList *
                 ibus_xkb_config_registry_layout_list_get_layouts
                                                 (IBusXKBConfigRegistry *xkb_config);

/**
 * ibus_xkb_config_registry_layout_list_get_variants:
 * @xkb_config: An IBusXKBConfigRegistry.
 * @layout: A layout.
 * @returns: (transfer container) (element-type utf8): A GList
 *
 * a GList
 */
GList *
                 ibus_xkb_config_registry_layout_list_get_variants
                                                 (IBusXKBConfigRegistry *xkb_config,
                                                  const gchar           *layout);

/**
 * ibus_xkb_config_registry_layout_lang_get_langs:
 * @xkb_config: An IBusXKBConfigRegistry.
 * @layout: A layout.
 * @returns: (transfer container) (element-type utf8): A GList
 *
 * a GList
 */
GList *
                 ibus_xkb_config_registry_layout_lang_get_langs
                                                 (IBusXKBConfigRegistry *xkb_config,
                                                  const gchar           *layout);

/**
 * ibus_xkb_config_registry_layout_desc_get_desc:
 * @xkb_config: An IBusXKBConfigRegistry.
 * @layout: A layout.
 * @returns: A layout description
 *
 * a layout description
 */
gchar *
                 ibus_xkb_config_registry_layout_desc_get_desc
                                                 (IBusXKBConfigRegistry *xkb_config,
                                                  const gchar           *layout);

/**
 * ibus_xkb_config_registry_variant_desc_get_desc:
 * @xkb_config: An IBusXKBConfigRegistry.
 * @variant: A variant.
 * @returns: A variant description
 *
 * a variant description
 */
gchar *
                 ibus_xkb_config_registry_variant_desc_get_desc
                                                 (IBusXKBConfigRegistry *xkb_config,
                                                  const gchar           *variant);
G_END_DECLS
#endif
