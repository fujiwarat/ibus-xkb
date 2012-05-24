# vim:set noet ts=4:
#
# ibus-xkb - IBus XKB
#
# Copyright (c) 2012 Takao Fujiwara <takao.fujiwara1@gmail.com>
# Copyright (c) 2007-2010 Peng Huang <shawn.p.huang@gmail.com>
# Copyright (c) 2012 Red Hat, Inc.
# Copyright (c) 2010, Google Inc. All rights reserved.
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

INCLUDES = \
	-I$(top_srcdir)/src \
	-I$(top_builddir)/src \
	$(NULL)

AM_CFLAGS = \
	@GLIB2_CFLAGS@ \
	@GIO2_CFLAGS@ \
	@GTHREAD2_CFLAGS@ \
	@IBUS_CFLAGS@ \
	$(INCLUDES) \
	-DG_LOG_DOMAIN=\"IBUS\" \
	-DPKGDATADIR=\"$(pkgdatadir)\" \
	-DLIBEXECDIR=\"$(libexecdir)\" \
	-DBINDIR=\"@bindir@\" \
    -DIBUS_DISABLE_DEPRECATED \
	-Wno-unused-variable \
	-Wno-unused-but-set-variable \
	-Wno-unused-function \
	$(NULL)

AM_LDADD = \
	@GOBJECT2_LIBS@ \
	@GLIB2_LIBS@ \
	@GIO2_LIBS@ \
	@GTHREAD2_LIBS@ \
	@IBUS_LIBS@ \
	$(libibus_xkb) \
	$(NULL)

AM_VALAFLAGS = \
	--vapidir=$(top_builddir)/bindings/vala \
	--pkg=ibus-1.0 \
	--pkg=ibus-xkb-1.0 \
	$(NULL)

libexec_PROGRAMS = \
	ibus-engine-xkb \
	$(NULL)

ibus_engine_xkb_SOURCES = \
	main.vala \
	$(NULL)
ibus_engine_xkb_CFLAGS = \
	$(AM_CFLAGS) \
	$(NULL)
ibus_engine_xkb_LDADD = \
	$(AM_LDADD) \
	$(NULL)
ibus_engine_xkb_DEPENDENCIES = \
	$(libibus_xkb) \
	$(NULL)

component_DATA = \
	xkb.xml \
	$(NULL)

componentdir = $(datadir)/ibus/component

CLEANFILES = \
	xkb.xml \
	$(NULL)

EXTRA_DIST = \
	xkb.xml.in.in \
	$(NULL)

xkb.xml: xkb.xml.in
	$(AM_V_GEN) \
	( \
		libexecdir=${libexecdir}; \
		s=`cat $<`; \
		eval "echo \"$${s}\""; \
	) > $@

$(libibus_xkb):
	$(MAKE) -C $(top_builddir)/src

-include $(top_srcdir)/git.mk
