# vim:set noet ts=4:
#
# ibus-xkb - IBus XKB
#
# Copyright (c) 2012 Takao Fujiwara <takao.fujiwara1@gmail.com>
# Copyright (c) 2012 Red Hat, Inc.
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
	$(INCLUDES) \
	-DG_LOG_DOMAIN=\"IBUS\" \
	-DIBUS_DISABLE_DEPRECATED \
	-Wno-unused-variable \
	-Wno-unused-but-set-variable \
	-Wno-unused-function \
	$(NULL)

libexec_PROGRAMS = ibus-xkb

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
    $(libibus_xkb) \
    $(NULL)

$(libibus_xkb):
	$(MAKE) -C $(top_builddir)/src

-include $(top_srcdir)/git.mk
