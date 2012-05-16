# vim:set noet ts=4:
#
# ibus-xkb - IBus XKB
#
# Copyright (C) 2011-2012 Takao Fujiwara <takao.fujiwara1@gmail.com>
# Copyright (c) 2011 Peng Huang <shawn.p.huang@gmail.com>
# Copyright (c) 2011-2012 Red Hat, Inc.
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this program; if not, write to the
# Free Software Foundation, Inc., 59 Temple Place, Suite 330,
# Boston, MA  02111-1307  USA

libibus_xkb = $(top_builddir)/src/libibus-xkb-common.la

xkblayoutdir = $(datadir)/ibus/component
xkblayout_in_files = xkblayout.xml.in
xkblayout_DATA = $(xkblayout_in_files:.xml.in=.xml)

xkblayoutconfigdir = $(datadir)/ibus/xkb
xkblayoutconfig_in_files = xkblayoutconfig.xml.in
xkblayoutconfig_DATA = $(xkblayoutconfig_in_files:.xml.in=.xml)

INCLUDES = \
    -I$(top_srcdir) \
    -I$(top_srcdir)/src \
	-DIBUS_LOCALEDIR=\"$(datadir)/locale\" \
	-DLIBEXECDIR=\""$(libexecdir)"\" \
	-DUSE_BRIDGE_HOTKEY=$(USE_BRIDGE_HOTKEY) \
	-DDEFAULT_BRIDGE_ENGINE_NAME=\"$(DEFAULT_BRIDGE_ENGINE_NAME)\" \
	-DXKBLAYOUTCONFIG_FILE=\""$(xkblayoutconfigdir)/$(xkblayoutconfig_DATA)"\" \
    $(NULL)

libexec_PROGRAMS = ibus-xkb ibus-engine-xkb

ibus_xkb_SOURCES = \
	ibus-xkb-main.c \
	xkblib.h \
	xkblib.c \
	$(NULL)
ibus_xkb_CFLAGS = \
	@XKB_CFLAGS@ \
	@X11_CFLAGS@ \
	@GLIB2_CFLAGS@ \
	$(NULL)
ibus_xkb_LDADD = \
	@XKB_LIBS@ \
	@X11_LIBS@ \
	@GLIB2_LIBS@ \
	$(NULL)

ibus_engine_xkb_SOURCES = \
        ibus-engine-xkb-main.c \
        ibus-engine-xkb-main.h \
        ibus-simple-engine.c \
        ibus-simple-engine.h \
        gtkimcontextsimpleseqs.h \
        xkbxml.c \
        xkbxml.h \
        $(NULL)
ibus_engine_xkb_CFLAGS = \
        @GLIB2_CFLAGS@ \
        @GOBJECT2_CFLAGS@ \
        @IBUS_CFLAGS@ \
        $(NULL)
ibus_engine_xkb_LDADD = \
        @GLIB2_LIBS@ \
        @GOBJECT2_LIBS@ \
        @IBUS_LIBS@ \
        $(libibus_xkb) \
        $(NULL)

%.xml : %.xml.in
	@sed -e "s|\@libexecdir\@|$(libexecdir)|g" \
	     -e "s|\@datadir\@|$(datadir)|g" \
	     -e "s|\@XKB_PRELOAD_LAYOUTS\@|$(XKB_PRELOAD_LAYOUTS)|g" \
	$< > $@

EXTRA_DIST = \
        $(xkblayout_in_files) \
        $(xkblayoutconfig_in_files) \
        $(NULL)

DISTCLEANFILES = \
        $(xkblayout_DATA) \
        $(xkblayoutconfig_DATA) \
        $(NULL)
