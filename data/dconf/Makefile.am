# vim:set noet ts=4:
#
# ibus - The Input Bus
#
# Copyright (c) 2012 Takao Fujiwara <takao.fujiwara1@gmail.com>
# Copyright (c) 2007-2010 Peng Huang <shawn.p.huang@gmail.com>
# Copyright (c) 2011 Daiki Ueno <ueno@unixuser.org>
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

gsettings_schemas_in_files = org.freedesktop.ibus.xkb.gschema.xml.in
gsettings_SCHEMAS = $(gsettings_schemas_in_files:.gschema.xml.in=.gschema.xml)
gsettingsconvertdir = $(datadir)/GConf/gsettings
dist_gsettingsconvert_DATA = ibus-xkb.convert
@GSETTINGS_RULES@
@INTLTOOL_XML_NOMERGE_RULE@

EXTRA_DIST = \
	$(gsettings_schemas_in_files) \
	make-dconf-override-db.sh \
	profile/ibus-xkb \
	01-xkb \
	$(NULL)

CLEANFILES = \
	$(gsettings_SCHEMAS) \
	$(NULL)

MAINTAINERCLEANFILES = \
	$(gsettings_schemas_in_files) \
	01-xkb \
	$(NULL)

#dconfprofiledir = $(sysconfdir)/dconf/profile
#dconfprofile_DATA = profile/ibus-xkb

dconfdbdir = $(sysconfdir)/dconf/db/ibus.d
dconfdb_DATA = 01-xkb

org.freedesktop.ibus.xkb.gschema.xml.in: $(top_srcdir)/data/ibus-xkb.schemas.in
	$(AM_V_GEN) gsettings-schema-convert --force --gconf --xml \
		--schema-id "org.freedesktop.ibus.xkb" \
		--output $@ $<

01-xkb: $(srcdir)/make-dconf-override-db.sh | $(gsettings_SCHEMAS)
	@$(MKDIR_P) db
	$(AM_V_GEN) $(srcdir)/make-dconf-override-db.sh > $@ || \
		{ rc=$$?; $(RM) -rf $@; exit $$rc; }

install-data-hook:
	dconf update

-include $(top_srcdir)/git.mk
