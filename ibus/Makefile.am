# vim:set noet ts=4:
#
# ibus-xkb - IBus XKB
#
# Copyright (c) 2011 Takao Fujiwara <takao.fujiwara1@gmail.com>
# Copyright (c) 2011 Red Hat, Inc.
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
ibus_PYTHON_in_files = \
	xkblayout.py.in \
	xkbxml.py.in \
	$(NULL)
ibus_PYTHON_gen_files = $(ibus_PYTHON_in_files:.py.in=.py)
ibus_PYTHON = \
	$(ibus_PYTHON_gen_files) \
	__init__.py \
	$(NULL)

ibusdir = @pkgpythondir@

XKB_COMMAND=\\\"$(libexecdir)/ibus-xkb\\\"

%.py : %.py.in
	@sed -e "s|\@XKB_COMMAND\@|$(XKB_COMMAND)|g" \
	     -e "s|\@XKB_RULES_XML_FILE\@|$(XKB_RULES_XML_FILE)|g" \
	     -e "s|\@HAVE_XKB\@|$(HAVE_XKB)|g" \
	     -e "s|\@datadir\@|$(datadir)|g" \
	$< > $@

EXTRA_DIST = \
	$(ibus_PYTHON_in_files) \
	$(NULL)

CLEANFILES = \
	*.pyc \
	$(ibus_PYTHON_gen_files) \
	$(NULL)
