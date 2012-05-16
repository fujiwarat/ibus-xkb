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

schemasdir = $(GCONF_SCHEMA_FILE_DIR)
schemas_in_files = ibus-xkb.schemas.in
schemas_DATA = $(schemas_in_files:.schemas.in=.schemas)
@INTLTOOL_SCHEMAS_RULE@

install-data-local:
if GCONF_SCHEMAS_INSTALL
	if test -z "$(DESTDIR)" ; then \
		for p in $(schemas_DATA) ; do \
			GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) \
				--makefile-install-rule $(top_builddir)/data/$$p >&1 > /dev/null; \
		done \
	fi
endif

EXTRA_DIST = \
	$(schemas_in_files) \
	$(NULL)

CLEANFILES = \
	$(schemas_DATA) \
	$(NULL)
