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

vapidir = $(datadir)/vala/vapi
dist_vapi_DATA = \
	ibus-xkb-@IBUS_XKB_API_VERSION@.vapi \
	ibus-xkb-@IBUS_XKB_API_VERSION@.deps \
	$(NULL)

# Don't rebuild vapi every time gir is updated.
vapi_deps = \
	$(srcdir)/IBusXKB-1.0.metadata \
	| \
	$(top_srcdir)/src/IBusXKB-@IBUS_XKB_API_VERSION@.gir \
	$(NULL)

ibus-xkb-@IBUS_XKB_API_VERSION@.vapi: $(vapi_deps)
	$(AM_V_GEN) $(VAPIGEN) --library ibus-xkb-@IBUS_XKB_API_VERSION@ \
			--metadatadir=$(srcdir) \
			--pkg=ibus-1.0 \
			$(top_srcdir)/src/IBusXKB-@IBUS_XKB_API_VERSION@.gir

EXTRA_DIST = \
	IBusXKB-1.0.metadata \
	config.vapi \
	xi.vapi \
	$(NULL)

MAINTAINERCLEANFILES = ibus-xkb-@IBUS_XKB_API_VERSION@.vapi

-include $(top_srcdir)/git.mk
