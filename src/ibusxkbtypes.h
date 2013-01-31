/* -*- mode: C; c-basic-offset: 4; indent-tabs-mode: nil; -*- */
/* vim:set et sts=4: */
/* ibus - The Input Bus
 * Copyright (C) 2012 Takao Fujiwara <takao.fujiwara1@gmail.com>
 * Copyright (C) 2008-2010 Peng Huang <shawn.p.huang@gmail.com>
 * Copyright (C) 2008-2012 Red Hat, Inc.
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

/**
 * SECTION: ibustypes
 * @short_description: Generic types for  IBus.
 * @stability: Stable
 *
 * This section consists generic types for IBus, including shift/control key modifiers,
 * and a rectangle structure.
 */
#ifndef __IBUS_XKBTYPES_H_
#define __IBUS_XKBTYPES_H_

/**
 * IBusXKBPreloadEngineMode:
 * @IBUS_XKB_PRELOAD_ENGINE_MODE_USER: user custimized engines
 * @IBUS_XKB_PRELOAD_ENGINE_MODE_LANG_RELATIVE: language related engines.
 */
typedef enum {
    IBUS_XKB_PRELOAD_ENGINE_MODE_USER          = 0,
    IBUS_XKB_PRELOAD_ENGINE_MODE_LANG_RELATIVE = 1,
} IBusXKBPreloadEngineMode;

#endif

