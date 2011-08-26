# vim:set et sts=4 sw=4:
#
# ibus-xkb - IBus XKB
#
# Copyright (c) 2011 Takao Fujiwara <takao.fujiwara1@gmail.com>
# Copyright (c) 2011 Peng Huang <shawn.p.huang@gmail.com>
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

import gettext
import os
import signal
import gtk
import ibus
import locale
from os import path
from enginecombobox import EngineComboBox
from enginetreeview import EngineTreeView
from xkbsetup import XKBSetup
from i18n import DOMAINNAME, _, N_, init as i18n_init

class Setup(object):
    def __flush_gtk_events(self):
        while gtk.events_pending():
            gtk.main_iteration()

    def __init__(self):
        super(Setup, self).__init__()
        gtk_builder_file = path.join(path.dirname(__file__), "./setup.ui")
        self.__builder = gtk.Builder()
        self.__builder.set_translation_domain(DOMAINNAME)
        self.__builder.add_from_file(gtk_builder_file);
        self.__bus = None
        self.__init_bus()
        self.__init_ui()
        self.__init_about_vbox()

    def __init_ui(self):
        # add icon search path
        self.__window = self.__builder.get_object("window_preferences")
        self.__window.connect("delete-event", gtk.main_quit)

        self.__button_close = self.__builder.get_object("button_close")
        self.__button_close.connect("clicked", gtk.main_quit)

        self.__config = self.__bus.get_config()

        # init engine page
        self.__engines = self.__bus.list_engines()

        XKBSetup(self.__config, self.__builder)

    def __init_bus(self):
        try:
            self.__bus = ibus.Bus()
        except:
            if self.__bus == None:
                message = _("IBus is not running.")
                dlg = gtk.MessageDialog(type = gtk.MESSAGE_INFO,
                                        buttons = gtk.BUTTONS_OK,
                                        message_format = message)
                id = dlg.run()
                dlg.destroy()
                exit(-1)

    def __init_about_vbox(self):
        about_dialog = self.__builder.get_object("about_dialog")
        about_vbox = self.__builder.get_object("about_vbox")

        content_area = about_dialog.get_content_area()
        list = content_area.get_children()
        vbox = list[0]
        for w in vbox.get_children():
            old_parent = w.parent
            w.unparent()
            w.set_parent_window(None)
            w.emit("parent-set", old_parent)
            about_vbox.pack_start(w, False, False, 0)

    def __sigusr1_cb(self, *args):
        self.__window.present()

    def run(self):
        self.__window.show_all()
        signal.signal(signal.SIGUSR1, self.__sigusr1_cb)
        gtk.main()

if __name__ == "__main__":
    locale.setlocale(locale.LC_ALL, '')
    i18n_init()
    setup = Setup()
    setup.run()
