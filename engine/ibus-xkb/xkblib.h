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
#ifndef __XKBLIB_H_
#define __XKBLIB_H_

#include <X11/Xlib.h>

G_BEGIN_DECLS

void             ibus_xkb_init                   (Display *xdisplay);
void             ibus_xkb_finit                  (void);
gchar           *ibus_xkb_get_current_layout     (void);
gchar           *ibus_xkb_get_current_variant    (void);
gchar           *ibus_xkb_get_current_option     (void);
gboolean         ibus_xkb_set_layout             (const char *layouts,
                                                  const char *variants,
                                                  const char *options);
int              ibus_xkb_get_current_group      (void);

G_END_DECLS
#endif
