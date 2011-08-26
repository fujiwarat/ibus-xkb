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
import gobject
import gtk
import ibus

_ = lambda a : gettext.dgettext("ibus", a)
XKB_MAX_LAYOUTS = 4

class XKBSetup(gobject.GObject):
    def __init__(self, config, builder):
        super(XKBSetup, self).__init__()

        self.__config = config
        self.__builder = builder

        # system keyboard layout setting
        self.__button_system_keyboard_layout = self.__builder.get_object("button_system_keyboard_layout")
        text = str(self.__config.get_value("engine/xkb", "system_keyboard_layout", ''))
        if text == 'default' or text == '':
            text = _("Default")
        self.__button_system_keyboard_layout.set_label(text)
        if not self.__config.get_value("general", "use_system_keyboard_layout", True):
            self.__button_system_keyboard_layout.set_sensitive(False)
        self.__button_system_keyboard_layout.connect("clicked", self.__button_system_keyboard_layout_cb)

        # use system keyboard layout setting
        self.__config.connect("value-changed", self.__config_value_changed_cb)

        self.__xkblayoutconfig = None
        self.__preload_xkb_engines = []
        self.__other_xkb_engines = []
        self.__default_xkb_engine = None
        if ibus.XKBConfigRegistry.have_xkb():
            self.__xkblayoutconfig = ibus.XKBLayoutConfig()

        # config layouts dialog
        self.__init_config_layouts()

        # default system keyboard dialog
        self.__init_system_keyboard()

    def __config_value_changed_cb(self, bus, section, name, value):
        if section == "general" and name == "use_system_keyboard_layout":
            self.__button_system_keyboard_layout.set_sensitive(value)

    def __get_xkb_engines(self):
        xkb_engines = []
        xkbconfig = ibus.XKBConfigRegistry()
        layout_list = xkbconfig.get_layout_list()
        layout_desc = xkbconfig.get_layout_desc()
        layout_lang = xkbconfig.get_layout_lang()
        variant_desc = xkbconfig.get_variant_desc()
        for layout in layout_list.keys():
            langs = []
            if layout in layout_lang:
                langs = layout_lang[layout]
            for lang in langs:
                engine = ibus.XKBConfigRegistry.engine_desc_new(
                    lang,
                    layout,
                    layout_desc[layout],
                    None,
                    None)
                xkb_engines.append(engine)
            for variant in layout_list[layout]:
                label = "%s(%s)" % (layout, variant)
                sub_langs = []
                if label in layout_lang:
                    sub_langs = layout_lang[label]
                else:
                    sub_langs = langs
                for lang in sub_langs:
                    engine = ibus.XKBConfigRegistry.engine_desc_new(
                        lang,
                        layout,
                        layout_desc[layout],
                        variant,
                        variant_desc[label])
                    xkb_engines.append(engine)
        return xkb_engines

    def __get_default_xkb_engine(self):
        if self.__default_xkb_engine != None:
            return self.__default_xkb_engine
        self.__default_xkb_engine = ibus.XKBConfigRegistry.engine_desc_new(
            "other",
            "default",
            _("Default"),
            None,
            None)
        return self.__default_xkb_engine

    def __init_config_layouts(self):
        if not ibus.XKBConfigRegistry.have_xkb():
            button = self.__builder.get_object("button_config_layouts")
            button.hide()
            return

        self.__dialog_config_layouts = self.__builder.get_object("dialog_config_layouts")
        self.__button_config_layouts_cancel = self.__builder.get_object("button_config_layouts_cancel")
        self.__button_config_layouts_cancel.connect("clicked", self.__button_config_layouts_cancel_cb)
        self.__button_config_layouts_ok = self.__builder.get_object("button_config_layouts_ok")
        self.__button_config_layouts_ok.connect("clicked", self.__button_config_layouts_ok_cb)
        self.__vbox_all_keyboard_layouts = self.__builder.get_object("vbox_all_keyboard_layouts")

        xkb_engines = self.__get_xkb_engines()
        if len(xkb_engines) > 0:
            button = self.__builder.get_object("button_config_layouts")
            button.connect("clicked", self.__button_config_layouts_cb)
            button.set_sensitive(True)

        engine_dict = {}
        for engine in xkb_engines:
            if not engine.name.startswith("xkb:layout:"):
                continue
            lang = ibus.get_language_name(engine.language)
            if lang not in engine_dict:
                engine_dict[lang] = []
            engine_dict[lang].append(engine)

        keys = engine_dict.keys()
        keys.sort()
        if ibus.get_language_name("Other") in keys:
            keys.remove(ibus.get_language_name("Other"))
            keys += [ibus.get_language_name("Other")]

        preload_xkb_engines = self.__xkblayoutconfig.get_preload_layouts()
        for lang in keys:
            expander = gtk.Expander("")
            self.__vbox_all_keyboard_layouts.pack_start(expander, True, True, 0)
            expander.show()
            label = expander.get_label_widget()
            label.set_label(lang)
            align = gtk.Alignment(0, 0, 1, 0)
            align.set_padding(6, 0, 18, 0)
            expander.add(align)
            align.show()
            vbox = gtk.VBox(False, 0)
            align.add(vbox)
            vbox.show()

            def cmp_engine(a, b):
                if a.rank == b.rank:
                    return cmp(a.longname, b.longname)
                return int(b.rank - a.rank)
            engine_dict[lang].sort(cmp_engine)

            for engine in engine_dict[lang]:
                sub_name = engine.name[len("xkb:layout:"):]
                layout_list = sub_name.split(':')
                if len(layout_list) > 1:
                    layout = "%s(%s)" % (layout_list[0], layout_list[1])
                else:
                    layout = layout_list[0]
                has_preloaded = False
                for preload_name in preload_xkb_engines:
                    preload_name = str(preload_name)
                    if len(preload_name) == 0:
                        continue
                    if layout == preload_name:
                        has_preloaded = True
                        break

                checkbutton = gtk.CheckButton(engine.longname)
                checkbutton.set_data("layout", layout)
                if has_preloaded:
                    checkbutton.set_active(True)
                vbox.pack_start(checkbutton, False, True, 0)
                checkbutton.show()

    def __init_system_keyboard_layout(self):
        self.__dialog_system_keyboard_layout = self.__builder.get_object("dialog_system_keyboard_layout")
        self.__button_system_keyboard_layout_cancel = self.__builder.get_object("button_system_keyboard_layout_cancel")
        self.__button_system_keyboard_layout_cancel.connect("clicked", self.__button_system_keyboard_layout_cancel_cb)
        self.__button_system_keyboard_layout_ok = self.__builder.get_object("button_system_keyboard_layout_ok")
        self.__button_system_keyboard_layout_ok.connect("clicked", self.__button_system_keyboard_layout_ok_cb)

        # get xkb layouts
        xkb_engines = self.__get_xkb_engines()

        self.__combobox_system_keyboard_layout = self.__builder.get_object("combobox_system_keyboard_layout_engines")
        self.__combobox_system_keyboard_layout.set_engines(xkb_engines)
        self.__combobox_system_keyboard_layout.set_title(_("Select keyboard layouts"))
        self.__combobox_system_keyboard_layout.connect("notify::active-engine", self.__combobox_notify_active_system_keyboard_layout_cb)
        self.__treeview_system_keyboard_layout = self.__builder.get_object("treeview_system_keyboard_layout_engines")
        self.__treeview_system_keyboard_layout.connect("notify", self.__treeview_notify_system_keyboard_layout_cb)
        column = self.__treeview_system_keyboard_layout.get_column(0)
        column.set_title(_("Keyboard Layouts"))
        button = self.__builder.get_object("button_system_keyboard_layout_engine_add")
        button.connect("clicked", self.__button_system_keyboard_layout_add_cb)
        button = self.__builder.get_object("button_system_keyboard_layout_engine_remove")
        button.connect("clicked", self.__button_system_keyboard_layout_remove_cb)
        button = self.__builder.get_object("button_system_keyboard_layout_engine_up")
        button.connect("clicked", lambda *args:self.__treeview_system_keyboard_layout.move_up_engine())

        button = self.__builder.get_object("button_system_keyboard_layout_engine_down")
        button.connect("clicked", lambda *args:self.__treeview_system_keyboard_layout.move_down_engine())
        button = self.__builder.get_object("button_system_keyboard_layout_engine_reset")
        button.connect("clicked", self.__button_system_keyboard_layout_reset_cb)
        button_reset = button
        text = str(self.__config.get_value("engine/xkb", "system_keyboard_layout", ''))
        if text == "default" or text == None:
            engine = self.__get_default_xkb_engine()
            self.__treeview_system_keyboard_layout.set_engines([engine])
            button_reset.set_sensitive(False)
        else:
            for layout in text.split(','):
                layout_engine = None
                for engine in xkb_engines:
                    if layout == engine.layout:
                        layout_engine = engine
                        break
                if layout_engine != None:
                    self.__treeview_system_keyboard_layout.append_engine(layout_engine)
            button_reset.set_sensitive(True)
        label = self.__builder.get_object("label_system_keyboard_layout_engines")
        label.set_markup(_("<small><i>The system keyboard layouts "
                           "can be set less than or equal to %d.\n"
                           "You may use Up/Down buttons to change the order."
                           "</i></small>") % XKB_MAX_LAYOUTS)

    def __init_system_keyboard_option(self):
        self.__dialog_system_keyboard_option = self.__builder.get_object("dialog_system_keyboard_option")
        self.__button_system_keyboard_option_close = self.__builder.get_object("button_system_keyboard_option_close")
        self.__button_system_keyboard_option_close.connect(
            "clicked", lambda button: self.__dialog_system_keyboard_option.hide())

        button = self.__builder.get_object("button_system_keyboard_option_setup")
        button.connect("clicked", self.__button_system_keyboard_option_cb)
        self.__checkbutton_use_system_keyboard_option = self.__builder.get_object("checkbutton_use_system_keyboard_option")
        self.__vbox_system_keyboard_options = self.__builder.get_object("vbox_system_keyboard_options")
        option_array = []
        text = str(self.__config.get_value("engine/xkb", "system_keyboard_option", ''))
        if text == None or text == "default":
            self.__checkbutton_use_system_keyboard_option.set_active(True)
            self.__vbox_system_keyboard_options.set_sensitive(False)
        else:
            self.__checkbutton_use_system_keyboard_option.set_active(False)
            self.__vbox_system_keyboard_options.set_sensitive(True)
            option_array = text.split(',')
        self.__checkbutton_use_system_keyboard_option.connect(
            "toggled", lambda button: self.__vbox_system_keyboard_options.set_sensitive(not button.get_active()))

        xkbconfig = ibus.XKBConfigRegistry()
        option_list = xkbconfig.get_option_list()
        option_group_desc = xkbconfig.get_option_group_desc()
        option_desc = xkbconfig.get_option_desc()
        for option_group in option_list.keys():
            expander = gtk.Expander("")
            self.__vbox_system_keyboard_options.pack_start(expander, True, True, 0)
            expander.show()
            checked = 0
            label = expander.get_label_widget()
            label.set_label(option_group_desc[option_group])
            label.set_data("option_group", option_group)
            expander.set_data("checked", checked)
            align = gtk.Alignment(0, 0, 1, 0)
            align.set_padding(6, 0, 18, 0)
            expander.add(align)
            align.show()
            vbox = gtk.VBox(False, 0)
            align.add(vbox)
            vbox.show()
            for option in option_list[option_group]:
                checkbutton = gtk.CheckButton(option_desc[option])
                checkbutton.set_data("option", option)
                if option in option_array:
                    checkbutton.set_active(True)
                    label.set_markup("<b>" +
                                     option_group_desc[option_group] +
                                     "</b>")
                    checked = checked + 1
                    expander.set_data("checked", checked)
                checkbutton.connect("toggled",
                                    self.__checkbutton_system_keyboard_option_toggled_cb,
                                    expander)
                vbox.pack_start(checkbutton, False, True, 0)
                checkbutton.show()

    def __init_system_keyboard(self):
        if not ibus.XKBConfigRegistry.have_xkb():
            hbox = self.__builder.get_object("hbox_system_keyboard_layout")
            hbox.hide()
            return

        self.__init_system_keyboard_layout()
        self.__init_system_keyboard_option()

    def __combobox_notify_active_system_keyboard_layout_cb(self, combobox, property):
        engine = self.__combobox_system_keyboard_layout.get_active_engine()
        button = self.__builder.get_object("button_system_keyboard_layout_engine_add")
        engines = self.__treeview_system_keyboard_layout.get_engines()
        button.set_sensitive(engine != None and \
                             engine not in engines and \
                             len(engines) < XKB_MAX_LAYOUTS)

    def __treeview_notify_system_keyboard_layout_cb(self, treeview, property):
        if property.name != "active-engine" and property.name != "engines":
            return

        engines = self.__treeview_system_keyboard_layout.get_engines()
        engine = self.__treeview_system_keyboard_layout.get_active_engine()

        button = self.__builder.get_object("button_system_keyboard_layout_engine_remove")
        button.set_sensitive(engine != None)
        button = self.__builder.get_object("button_system_keyboard_layout_engine_up")
        button.set_sensitive(engine not in engines[:1])
        button = self.__builder.get_object("button_system_keyboard_layout_engine_down")
        button.set_sensitive(engine not in engines[-1:])

    def __button_system_keyboard_layout_add_cb(self, button):
        engines = self.__treeview_system_keyboard_layout.get_engines()
        engine = self.__combobox_system_keyboard_layout.get_active_engine()
        if len(engines) > 0 and engines[0].layout == "default":
            self.__treeview_system_keyboard_layout.set_engines([engine])
        else:
            self.__treeview_system_keyboard_layout.append_engine(engine)
        button_reset = self.__builder.get_object("button_system_keyboard_layout_engine_reset")
        button_reset.set_sensitive(True)
        if len(self.__treeview_system_keyboard_layout.get_engines()) >= XKB_MAX_LAYOUTS:
            button.set_sensitive(False)

    def __button_system_keyboard_layout_remove_cb(self, button):
        self.__treeview_system_keyboard_layout.remove_engine()
        if len(self.__treeview_system_keyboard_layout.get_engines()) < XKB_MAX_LAYOUTS:
            button_add = self.__builder.get_object("button_system_keyboard_layout_engine_add")
            button_add.set_sensitive(True)
        button_reset = self.__builder.get_object("button_system_keyboard_layout_engine_reset")
        button_reset.set_sensitive(True)

    def __button_system_keyboard_layout_reset_cb(self, button):
        engine = self.__get_default_xkb_engine()
        self.__treeview_system_keyboard_layout.set_engines([engine])
        button.set_sensitive(False)

    def __button_config_layouts_cb(self, button):
        self.__dialog_config_layouts.run()
        self.__dialog_config_layouts.hide()

    def __button_config_layouts_cancel_cb(self, button):
        self.__dialog_config_layouts.hide()

    def __button_config_layouts_ok_cb(self, button):
        self.__dialog_config_layouts.hide()
        engine_list = []
        for expander in self.__vbox_all_keyboard_layouts.get_children():
            align = expander.get_children()[0]
            vbox = align.get_children()[0]
            for checkbutton in vbox.get_children():
                if checkbutton.get_active():
                    engine_list.append(checkbutton.get_data("layout"))
        if len(engine_list) == 0:
            return
        engine_list.sort()
        self.__xkblayoutconfig.save_preload_layouts(engine_list)
        message = _("Please restart IBus to reload your configuration.")
        dlg = gtk.MessageDialog(type = gtk.MESSAGE_INFO,
                                buttons = gtk.BUTTONS_OK,
                                message_format = message)
        dlg.run()
        dlg.destroy()

    def __button_system_keyboard_layout_cb(self, button):
        self.__dialog_system_keyboard_layout.run()
        self.__dialog_system_keyboard_layout.hide()

    def __button_system_keyboard_layout_cancel_cb(self, button):
        self.__dialog_system_keyboard_layout.hide()

    def __button_system_keyboard_layout_ok_cb(self, button):
        self.__dialog_system_keyboard_layout.hide()
        layout = "default"
        for engine in self.__treeview_system_keyboard_layout.get_engines():
            if layout == "default":
                layout = engine.layout
            else:
                layout = "%s,%s" % (layout, engine.layout)
        if layout == None or layout == "":
            layout = "default"
        org_layout = str(self.__config.get_value("engine/xkb", "system_keyboard_layout", None))
        if layout != org_layout:
            self.__config.set_value("engine/xkb", "system_keyboard_layout", layout)
        if layout == "default":
            layout = _("Default")
        self.__button_system_keyboard_layout.set_label(layout)
        option = "default"
        if not self.__checkbutton_use_system_keyboard_option.get_active():
            for expander in self.__vbox_system_keyboard_options.get_children():
                align = expander.get_children()[0]
                vbox = align.get_children()[0]
                for checkbutton in vbox.get_children():
                    if checkbutton.get_active():
                        data = checkbutton.get_data("option")
                        if option == "default":
                            option = data
                        else:
                            option = "%s,%s" % (option, data)
        if option == None or option == "":
            option = "default"
        if option != "default" and option.find(':') < 0:
            message = _("The keyboard option cannot be chosen.")
            dlg = gtk.MessageDialog(type = gtk.MESSAGE_INFO,
                                    buttons = gtk.BUTTONS_OK,
                                    message_format = message)
            dlg.run()
            dlg.destroy()
            return
        org_option = str(self.__config.get_value("engine/xkb", "system_keyboard_option", None))
        if option != org_option:
            self.__config.set_value("engine/xkb", "system_keyboard_option", option)
        message = _("Please restart IBus to reload your configuration.")
        dlg = gtk.MessageDialog(type = gtk.MESSAGE_INFO,
                                buttons = gtk.BUTTONS_OK,
                                message_format = message)
        dlg.run()
        dlg.destroy()

    def __button_system_keyboard_option_cb(self, button):
        self.__dialog_system_keyboard_option.run()
        self.__dialog_system_keyboard_option.hide()

    def __checkbutton_system_keyboard_option_toggled_cb(self, button, user_data):
        expander = user_data
        checked = expander.get_data("checked")
        label = expander.get_label_widget()
        if button.get_active():
            checked = checked + 1
            label.set_markup("<b>" + label.get_text() + "</b>")
        else:
            checked = checked - 1
            if checked <= 0:
                label.set_text(label.get_text())
        expander.set_data("checked", checked)

