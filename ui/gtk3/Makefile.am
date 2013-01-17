# vim:set noet ts=4:
#
# ibus-xkb - IBus XKB
#
# Copyright (c) 2012 Takao Fujiwara <takao.fujiwara1@gmail.com>
# Copyright (c) 2007-2010 Peng Huang <shawn.p.huang@gmail.com>
# Copyright (c) 2007-2012 Red Hat, Inc.
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

NULL =

libibus_xkb = $(top_builddir)/src/libibus-xkb-common.la

component_DATA = \
	gtkxkbpanel.xml \
	$(NULL)
componentdir = $(datadir)/ibus/component

gtkxkbpanel.xml: gtkxkbpanel.xml.in
	$(AM_V_GEN) \
	( \
		libexecdir=${libexecdir}; \
		s=`cat $<`; \
		eval "echo \"$${s}\""; \
	) > $@

INCLUDES = \
	-I$(top_srcdir)/src \
	-I$(top_builddir)/src \
	$(NULL)

USE_SYMBOL_ICON = TRUE

# force include config.h before gi18n.h.
AM_CPPFLAGS = -include $(CONFIG_HEADER)

HAVE_IBUS_GKBD_C = $(strip $(subst false, FALSE, $(subst true, TRUE, $(HAVE_IBUS_GKBD))))

AM_CFLAGS = \
	@GLIB2_CFLAGS@ \
	@GIO2_CFLAGS@ \
	@GTHREAD2_CFLAGS@ \
	@GTK3_CFLAGS@ \
	@X11_CFLAGS@ \
	@IBUS_CFLAGS@ \
	$(INCLUDES) \
	-DG_LOG_DOMAIN=\"IBUS\" \
	-DBINDIR=\"$(bindir)\" \
	-DIBUS_DISABLE_DEPRECATED \
	-DPANEL_IBUS_VERSION="\"@IBUS_XKB_VERSION@ (IBus: @IBUS_VERSION@)\"" \
	-DSWITCHER_USE_SYMBOL_ICON=$(USE_SYMBOL_ICON) \
	-DHAVE_IBUS_GKBD=$(HAVE_IBUS_GKBD_C) \
	-DXKB_LAYOUTS_MAX_LENGTH=4 \
	-Wno-unused-variable \
	-Wno-unused-but-set-variable \
	-Wno-unused-function \
	$(NULL)

AM_LDADD = \
	@GOBJECT2_LIBS@ \
	@GLIB2_LIBS@ \
	@GIO2_LIBS@ \
	@GTHREAD2_LIBS@ \
	@GTK3_LIBS@ \
	@X11_LIBS@ \
	@IBUS_LIBS@ \
	-lXi \
	$(libibus_xkb) \
	$(NULL)

AM_VALAFLAGS = \
	--vapidir=$(top_builddir)/bindings/vala \
	--vapidir=$(top_srcdir)/bindings/vala \
	--pkg=posix \
	--pkg=gtk+-3.0 \
	--pkg=gdk-x11-3.0 \
	--pkg=ibus-1.0 \
	--pkg=ibus-xkb-1.0 \
	--pkg=config \
	--pkg=xi \
	$(NULL)

libexec_PROGRAMS = ibus-xkb-ui-gtk3

ibus_xkb_ui_gtk3_SOURCES = \
	application.vala \
	candidatearea.vala \
	candidatepanel.vala \
	gkbdlayout.vala \
	handle.vala \
	iconwidget.vala \
	keybindingmanager.vala \
	panel.vala \
	pango.vala \
	property.vala \
	separator.vala \
	switcher.vala \
	xkblayout.vala \
	$(NULL)

ibus_xkb_ui_gtk3_LDADD = \
	$(AM_LDADD) \
	$(NULL)

if ENABLE_LIBGNOMEKBD
AM_CFLAGS += \
	@LIBGNOMEKBDUI_CFLAGS@ \
	@ATK_CFLAGS@ \
	$(NULL)

AM_LDADD += \
	@LIBGNOMEKBDUI_LIBS@ \
	@ATK_LIBS@ \
	$(NULL)

AM_VALAFLAGS += \
	--vapidir=$(top_srcdir)/ui/gtk3 \
	--metadatadir=$(top_srcdir)/bindings/vala \
	--pkg=glib-2.0 \
	--pkg=gmodule-2.0 \
	--pkg=gkbd \
	--pkg=Xkl-1.0 \
	$(NULL)

$(builddir)/gkbdlayout.vala: $(top_builddir)/bindings/vala/gkbd.vapi
	@cp $(srcdir)/gkbdlayout.vala.true $(builddir)/gkbdlayout.vala
else
$(builddir)/gkbdlayout.vala:
	@cp $(srcdir)/gkbdlayout.vala.false $(builddir)/gkbdlayout.vala
endif

CLEANFILES = \
	gkbdlayout.vala \
	gtkxkbpanel.xml \
	$(NULL)

EXTRA_DIST = \
	gkbdlayout.vala.false \
	gkbdlayout.vala.true \
	gtkxkbpanel.xml.in.in \
	$(NULL)


-include $(top_srcdir)/git.mk
