# vim:set et sts=4 sw=4:
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

import locale

from gi.repository import GObject
from gi.repository import Gtk
from gi.repository import IBus
from gi.repository import Pango

from icon import load_icon
from i18n import _, N_

class EngineComboBox(Gtk.ComboBox):
    __gtype_name__ = 'EngineComboBox'
    __gproperties__ = {
        'active-engine' : (
            object,
            'selected engine',
            'selected engine',
            GObject.ParamFlags.READABLE)
    }

    def __init__(self):
        super(EngineComboBox, self).__init__()
        self.connect("notify::active", self.__notify_active_cb)

        self.__model = None
        self.__all_model = None
        self.__config = None
        self.__show_sub_lang = False

        renderer = Gtk.CellRendererPixbuf()
        renderer.set_property("xalign", 0)
        renderer.set_property("xpad", 2)
        self.pack_start(renderer, False)
        self.set_cell_data_func(renderer, self.__icon_cell_data_cb, None)

        renderer = Gtk.CellRendererText()
        renderer.set_property("xalign", 0)
        renderer.set_property("xpad", 2)
        self.pack_start(renderer, True)
        self.set_cell_data_func(renderer, self.__name_cell_data_cb, None)

    def __gconf_get_lang_list_from_locale(self):
        common_list = ['en', 'Other']
        if  self.__config == None:
            return None
        loc = None
        try:
           loc = locale.setlocale (locale.LC_ALL)
        except:
            pass
        if loc == None:
            return common_list
        current_lang = IBus.get_language_name(loc)
        if current_lang == None:
            return common_list
        group_list = self.__config.get_value("general/xkblayoutconfig",
                                             "group_list")
        if group_list == None:
            return [loc] + common_list
        group_list = list(group_list)
        lang_list = None
        for group in group_list:
            group = str(group)
            langs = list(self.__config.get_value("general/xkblayoutconfig",
                                                  group))
            for lang in langs:
                lang = str(lang)
                if current_lang == IBus.get_language_name(lang):
                    lang_list = langs
                    break
            if lang_list != None:
                break
        if lang_list == None:
            return [loc] + common_list
        return lang_list + common_list

    def __has_engine_in_lang_list(self, engine, lang_list):
        retval = False
        for lang in lang_list:
            if IBus.get_language_name(lang) == \
                IBus.get_language_name(engine.props.language):
                retval = True
                break
        return retval

    def __model_append_langs(self, model, langs, visible):
        keys = langs.keys()
        keys.sort(locale.strcoll)
        loc = locale.getlocale()[0]
        # None on C locale
        if loc == None:
            loc = 'en_US'
        current_lang = IBus.get_language_name(loc)
        # move current language to the first place
        if current_lang in keys:
            keys.remove(current_lang)
            keys.insert(0, current_lang)

        #add "Others" to the end of the combo box
        if IBus.get_language_name("Other") in keys:
            keys.remove(IBus.get_language_name("Other"))
            keys += [IBus.get_language_name("Other")]
        for l in keys:
            iter1 = model.append(None)
            model.set(iter1, 0, l)
            def cmp_engine(a, b):
                if a.get_rank() == b.get_rank():
                    return locale.strcoll(a.get_longname(), b.get_longname())
                return int(b.get_rank() - a.get_rank())
            langs[l].sort(cmp_engine)
            for e in langs[l]:
                iter2 = model.append(iter1)
                model.set(iter2, 0, e)

    def set_engines(self, engines):
        self.__model = Gtk.TreeStore(object)

        iter1 = self.__model.append(None)
        self.__model.set(iter1, 0, 0)
        lang_list = self.__gconf_get_lang_list_from_locale()
        lang = {}
        sub_lang = {}
        for e in engines:
            l = IBus.get_language_name(e.props.language)
            if lang_list == None or \
                self.__has_engine_in_lang_list(e, lang_list):
                if l not in lang:
                    lang[l] = []
                lang[l].append(e)
            else:
                if l not in sub_lang:
                    sub_lang[l] = []
                sub_lang[l].append(e)

        self.__model_append_langs(self.__model, lang, True)
        iter1 = self.__model.append(None)
        self.__model.set(iter1, 0, -1)

        self.__all_model = Gtk.TreeStore(object)
        iter1 = self.__all_model.append(None)
        self.__all_model.set(iter1, 0, 0)
        self.__model_append_langs(self.__all_model, lang, False)
        iter1 = self.__all_model.append(None)
        self.__all_model.set(iter1, 0, -1)
        self.__model_append_langs(self.__all_model, sub_lang, False)

        self.__toggle_sub_lang()

    def __toggle_sub_lang(self):
        self.set_model(None)
        if self.__show_sub_lang:
            self.set_model(self.__all_model)
        else:
            self.set_model(self.__model)
        self.set_active(0)

    def __icon_cell_data_cb(self, celllayout, renderer, model, iter, data):
        model = self.get_model()
        engine = model.get_value(iter, 0)

        if isinstance(engine, str) or isinstance (engine, unicode):
            renderer.set_property("visible", False)
            renderer.set_property("sensitive", False)
        elif isinstance(engine, int):
            if engine == 0:
                renderer.set_property("visible", False)
                renderer.set_property("sensitive", False)
                renderer.set_property("pixbuf", None)
            elif engine < 0:
                if not self.__show_sub_lang:
                    pixbuf = load_icon("go-bottom", Gtk.IconSize.LARGE_TOOLBAR)
                else:
                    pixbuf = load_icon("go-up", Gtk.IconSize.LARGE_TOOLBAR)
                if pixbuf == None:
                    pixbuf = load_icon(Gtk.STOCK_MISSING_IMAGE,
                                       Gtk.IconSize.LARGE_TOOLBAR)
                if pixbuf == None:
                    renderer.set_property("visible", False)
                    renderer.set_property("sensitive", False)
                    return
                renderer.set_property("visible", True)
                renderer.set_property("sensitive", True)
                renderer.set_property("pixbuf", pixbuf)
        else:
            renderer.set_property("visible", True)
            renderer.set_property("sensitive", True)
            pixbuf = load_icon(engine.get_icon(), Gtk.IconSize.LARGE_TOOLBAR)
            renderer.set_property("pixbuf", pixbuf)

    def __name_cell_data_cb(self, celllayout, renderer, model, iter, data):
        model = self.get_model()
        engine = model.get_value(iter, 0)

        if isinstance (engine, str) or isinstance (engine, unicode):
            renderer.set_property("sensitive", False)
            renderer.set_property("text", engine)
            renderer.set_property("weight", Pango.Weight.NORMAL)
        elif isinstance(engine, int):
            renderer.set_property("sensitive", True)
            if engine == 0:
                renderer.set_property("text", _("Select an input method"))
                renderer.set_property("weight", Pango.Weight.NORMAL)
            elif engine < 0:
                if not self.__show_sub_lang:
                    renderer.set_property("text", _("Show all input methods"))
                else:
                    renderer.set_property("text", _("Show only input methods for your region"))
                renderer.set_property("weight", Pango.Weight.BOLD)
        else:
            renderer.set_property("sensitive", True)
            renderer.set_property("text", engine.get_longname())
            renderer.set_property("weight",
                    Pango.Weight.BOLD if engine.get_rank() > 0 else Pango.Weight.NORMAL)

    def __notify_active_cb(self, combobox, property):
        self.notify("active-engine")

    def do_get_property(self, property):
        if property.name == "active-engine":
            i = self.get_active()
            if i == 0 or i == -1:
                return None
            iter = self.get_active_iter()
            model = self.get_model()
            if model[iter][0] == -1:
                self.__show_sub_lang = not self.__show_sub_lang
                self.__toggle_sub_lang()
                return None
            return model[iter][0]
        else:
            raise AttributeError, 'unknown property %s' % property.name

    def set_config(self, config):
        self.__config = config

    def get_active_engine(self):
        return self.get_property("active-engine")

if __name__ == "__main__":
    combo = EngineComboBox()
    combo.set_engines([IBus.EngineDesc(language="zh")])
    w = Gtk.Window()
    w.add(combo)
    w.show_all()
    Gtk.main()
