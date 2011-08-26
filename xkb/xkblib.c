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
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/XKBlib.h>
#include <stdio.h> /* for XKBrules.h */
#include <X11/extensions/XKBrules.h>
#include <X11/extensions/XKBstr.h>
#include <string.h>

#include "xkblib.h"

#ifndef XKB_RULES_XML_FILE
#define XKB_RULES_XML_FILE "/usr/share/X11/xkb/rules/evdev.xml"
#endif

static gchar          **default_layouts;
static gchar          **default_models;
static gchar          **default_options;
static int              default_layout_group;

static Display *
get_xdisplay (Display *xdisplay)
{
    static Display *saved_xdisplay = NULL;
    if (xdisplay != NULL) {
        saved_xdisplay = xdisplay;
    }
    return saved_xdisplay;
}

static void
init_xkb_default_layouts (Display *xdisplay)
{
    XkbStateRec state;
    Atom xkb_rules_name, type;
    int format;
    unsigned long l, nitems, bytes_after;
    unsigned char *prop = NULL;

    xkb_rules_name = XInternAtom (xdisplay, "_XKB_RULES_NAMES", TRUE);
    if (xkb_rules_name == None) {
        g_warning ("Could not get XKB rules atom");
        return;
    }
    if (XGetWindowProperty (xdisplay,
                            XDefaultRootWindow (xdisplay),
                            xkb_rules_name,
                            0, 1024, FALSE, XA_STRING,
                            &type, &format, &nitems, &bytes_after, &prop) != Success) {
        g_warning ("Could not get X property");
        return;
    }
    if (nitems < 3) {
        g_warning ("Could not get group layout from X property");
        return;
    }
    for (l = 0; l < 2; l++) {
        prop += strlen ((const char *) prop) + 1;
    }
    if (prop == NULL || *prop == '\0') {
        g_warning ("No layouts form X property");
        return;
    }
    default_layouts = g_strsplit ((gchar *) prop, ",", -1);
    prop += strlen ((const char *) prop) + 1;
    default_models = g_strsplit ((gchar *) prop, ",", -1);
    prop += strlen ((const char *) prop) + 1;
    default_options = g_strsplit ((gchar *) prop, ",", -1);

    if (XkbGetState (xdisplay, XkbUseCoreKbd, &state) != Success) {
        g_warning ("Could not get state");
        return;
    }
    default_layout_group = state.group;
}

static Bool
set_xkb_rules (Display *xdisplay,
               const char *rules_file, const char *model,
               const char *all_layouts, const char *all_variants,
               const char *all_options)
{
    gchar *rules_path;
    XkbRF_RulesPtr rules;
    XkbRF_VarDefsRec rdefs;
    XkbComponentNamesRec rnames;
    XkbDescPtr xkb;

    rules_path = g_strdup ("./rules/evdev");
    rules = XkbRF_Load (rules_path, "C", TRUE, TRUE);
    if (rules == NULL) {
        g_return_val_if_fail (XKB_RULES_XML_FILE != NULL, FALSE);

        g_free (rules_path);
        if (g_str_has_suffix (XKB_RULES_XML_FILE, ".xml")) {
            rules_path = g_strndup (XKB_RULES_XML_FILE,
                                    strlen (XKB_RULES_XML_FILE) - 4);
        } else {
            rules_path = g_strdup (XKB_RULES_XML_FILE);
        }
        rules = XkbRF_Load (rules_path, "C", TRUE, TRUE);
    }
    g_return_val_if_fail (rules != NULL, FALSE);

    memset (&rdefs, 0, sizeof (XkbRF_VarDefsRec));
    memset (&rnames, 0, sizeof (XkbComponentNamesRec));
    rdefs.model = model ? g_strdup (model) : NULL;
    rdefs.layout = all_layouts ? g_strdup (all_layouts) : NULL;
    rdefs.variant = all_variants ? g_strdup (all_variants) : NULL;
    rdefs.options = all_options ? g_strdup (all_options) : NULL;
    XkbRF_GetComponents (rules, &rdefs, &rnames);
    xkb = XkbGetKeyboardByName (xdisplay, XkbUseCoreKbd, &rnames,
                                XkbGBN_AllComponentsMask,
                                XkbGBN_AllComponentsMask &
                                (~XkbGBN_GeometryMask), True);
    if (!xkb) {
        g_warning ("Cannot load new keyboard description.");
        return FALSE;
    }
    XkbRF_SetNamesProp (xdisplay, rules_path, &rdefs);
    g_free (rules_path);
    g_free (rdefs.model);
    g_free (rdefs.layout);
    g_free (rdefs.variant);
    g_free (rdefs.options);

    return TRUE;
}

static Bool
update_xkb_properties (Display *xdisplay,
                       const char *rules_file, const char *model,
                       const char *all_layouts, const char *all_variants,
                       const char *all_options)
{
    int len;
    char *pval;
    char *next;
    Atom rules_atom;
    Window root_window;

    len = (rules_file ? strlen (rules_file) : 0);
    len += (model ? strlen (model) : 0);
    len += (all_layouts ? strlen (all_layouts) : 0);
    len += (all_variants ? strlen (all_variants) : 0);
    len += (all_options ? strlen (all_options) : 0);

    if (len < 1) {
        return TRUE;
    }
    len += 5; /* trailing NULs */

    rules_atom = XInternAtom (xdisplay, _XKB_RF_NAMES_PROP_ATOM, False);
    root_window = XDefaultRootWindow (xdisplay);
    pval = next = g_new0 (char, len + 1);
    if (!pval) {
        return TRUE;
    }

    if (rules_file) {
        strcpy (next, rules_file);
        next += strlen (rules_file);
    }
    *next++ = '\0';
    if (model) {
        strcpy (next, model);
        next += strlen (model);
    }
    *next++ = '\0';
    if (all_layouts) {
        strcpy (next, all_layouts);
        next += strlen (all_layouts);
    }
    *next++ = '\0';
    if (all_variants) {
        strcpy (next, all_variants);
        next += strlen (all_variants);
    }
    *next++ = '\0';
    if (all_options) {
        strcpy (next, all_options);
        next += strlen (all_options);
    }
    *next++ = '\0';
    if ((next - pval) != len) {
        g_free (pval);
        return TRUE;
    }

    XChangeProperty (xdisplay, root_window,
                    rules_atom, XA_STRING, 8, PropModeReplace,
                    (unsigned char *) pval, len);
    XSync(xdisplay, False);

    return TRUE;
}

void
ibus_xkb_init (Display *xdisplay)
{
    get_xdisplay (xdisplay);
    init_xkb_default_layouts (xdisplay);
}

void
ibus_xkb_finit (void)
{
    g_strfreev (default_layouts);
    default_layouts = NULL;
    g_strfreev (default_models);
    default_models = NULL;
    g_strfreev (default_options);
    default_options = NULL;
}

gchar *
ibus_xkb_get_current_layout (void)
{
    if (default_layouts == NULL) {
        g_warning ("Your system seems not to support XKB.");
        return NULL;
    }

    return g_strjoinv (",", (gchar **) default_layouts);
}

gchar *
ibus_xkb_get_current_model (void)
{
    if (default_models == NULL) {
        return NULL;
    }

    return g_strjoinv (",", (gchar **) default_models);
}

gchar *
ibus_xkb_get_current_option (void)
{
    if (default_options == NULL) {
        return NULL;
    }

    return g_strjoinv (",", (gchar **) default_options);
}

gboolean
ibus_xkb_set_layout  (const char *layouts,
                      const char *variants,
                      const char *options)
{
    Display *xdisplay;
    gboolean retval;
    gchar *layouts_line;

    if (default_layouts == NULL) {
        g_warning ("Your system seems not to support XKB.");
        return FALSE;
    }

    if (layouts == NULL || g_strcmp0 (layouts, "default") == 0) {
        layouts_line = g_strjoinv (",", (gchar **) default_layouts);
    } else {
        layouts_line = g_strdup (layouts);
    }

    xdisplay = get_xdisplay (NULL);
    retval = set_xkb_rules (xdisplay,
                            "evdev", "evdev",
                            layouts_line, variants, options);
    update_xkb_properties (xdisplay,
                           "evdev", "evdev",
                           layouts_line, variants, options);
    g_free (layouts_line);

    return retval;
}

int
ibus_xkb_get_current_group (void)
{
    Display *xdisplay = get_xdisplay (NULL);
    XkbStateRec state;

    if (default_layouts == NULL) {
        g_warning ("Your system seems not to support XKB.");
        return 0;
    }

    if (xdisplay == NULL) {
        g_warning ("ibus-xkb is not initialized.");
        return 0;
    }

    if (XkbGetState (xdisplay, XkbUseCoreKbd, &state) != Success) {
        g_warning ("Could not get state");
        return 0;
    }

    return state.group;
}
