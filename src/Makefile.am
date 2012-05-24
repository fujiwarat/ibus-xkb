# vim:set noet ts=4:
#
# ibus-xkb - IBus XKB
#
# Copyright (c) 2011-2012 Takao Fujiwara <takao.fujiwara1@gmail.com>
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
libibus_xkb_common = libibus-xkb-common.la
# libibus_xkb = libibus-xkb-@IBUS_XKB_API_VERSION@.la
libibus_xkb = libibus-xkb-1.0.la

# gobject introspection
-include $(INTROSPECTION_MAKEFILE)
INTROSPECTION_SCANNER_ARGS =
INTROSPECTION_COMPILER_ARGS =                                           \
    --includedir=$(srcdir)                                              \
    --includedir=.                                                      \
    $(NULL)
INTROSPECTION_GIRS =
CLEANFILES =

# C preprocessor flags
AM_CPPFLAGS =                                                           \
    -DG_LOG_DOMAIN=\"IBUS_XKB\"                                         \
    @GLIB2_CFLAGS@                                                      \
    @GOBJECT2_CFLAGS@                                                   \
    @IBUS_CFLAGS@                                                       \
    -DISOCODES_PREFIX=\"$(ISOCODES_PREFIX)\"                            \
    $(NULL)

# ibus library
noinst_LTLIBRARIES= $(libibus_xkb_common)
lib_LTLIBRARIES = $(libibus_xkb)

ibus_xkb_common_sources =                                               \
    ibusxkbxml.c                                                        \
    $(NULL)
ibus_xkb_common_headers =                                               \
    ibusxkbxml.h                                                        \
    $(NULL)
ibus_xkb_include_HEADERS =                                              \
    $(ibus_xkb_common_headers)                                          \
    $(NULL)
ibus_xkb_includedir = $(includedir)/ibus-xkb-@IBUS_XKB_API_VERSION@

libibus_xkb_common_la_SOURCES =                                         \
    $(ibus_xkb_common_sources)                                          \
    $(NULL)
libibus_xkb_common_la_LIBADD =                                          \
    @GLIB2_LIBS@                                                        \
    @GOBJECT2_LIBS@                                                     \
    @IBUS_LIBS@                                                         \
    $(NULL)
libibus_xkb_common_la_LDFLAGS =                                         \
    -no-undefined                                                       \
    -export-symbols-regex "ibus_xkb_.*"                                 \
    $(NULL)

libibus_xkb_1_0_la_SOURCES = $(NULL)
libibus_xkb_1_0_la_LIBADD =                                             \
    $(libibus_xkb_common_la_LIBADD)                                     \
    $(libibus_xkb_common)                                               \
    $(NULL)
libibus_xkb_1_0_la_LDFLAGS =                                            \
    -no-undefined                                                       \
    -export-symbols-regex "ibus_xkb_.*"                                 \
    -version-info @LT_VERSION_INFO@                                     \
    $(NULL)

if HAVE_INTROSPECTION
introspection_files =                                                   \
    $(ibus_xkb_common_headers)                                          \
    $(ibus_xkb_common_c_sources)                                        \
    $(NULL)
IBusXKB-1.0.gir: $(libibus_xkb) Makefile
IBusXKB_1_0_gir_SCANNERFLAGS =                                          \
    --warn-all                                                          \
    --pkg-export=ibus-xkb                                               \
    --pkg=ibus-1.0                                                      \
    $(IBUS_GIR_SCANNERFLAGS)                                            \
    $(NULL)
IBusXKB_1_0_gir_INCLUDES = GLib-2.0 GObject-2.0 IBus-1.0
IBusXKB_1_0_gir_LIBS = $(libibus_xkb)
IBusXKB_1_0_gir_FILES = $(addprefix $(srcdir)/,$(introspection_files))
IBusXKB_1_0_gir_CFLAGS = -DIBUS_COMPILATION @IBUS_CFLAGS@
INTROSPECTION_GIRS += IBusXKB-1.0.gir

girdir = $(datadir)/gir-1.0
dist_gir_DATA = $(INTROSPECTION_GIRS)

typelibsdir = $(libdir)/girepository-1.0
typelibs_DATA = $(INTROSPECTION_GIRS:.gir=.typelib)

CLEANFILES += $(dist_gir_DATA) $(typelibs_DATA)
endif
