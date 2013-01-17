/* vim:set et sts=4 sw=4:
 *
 * ibus - The Input Bus
 *
 * Copyright 2012 Red Hat, Inc.
 * Copyright(c) 2012 Peng Huang <shawn.p.huang@gmail.com>
 * Copyright(c) 2012 Takao Fujiwara <tfujiwar@redhat.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or(at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA  02111-1307  USA
 */

public extern const bool HAVE_IBUS_GKBD;
public extern const int XKB_LAYOUTS_MAX_LENGTH;

class XKBLayout
{
    string m_xkb_command = "setxkbmap";
    IBus.Config m_config = null;
    string[] m_xkb_latin_layouts = {};
    GLib.Pid m_xkb_pid = -1;
    GLib.Pid m_xmodmap_pid = -1;
    string m_xmodmap_command = "xmodmap";
    bool m_use_xmodmap = true;
    string[] m_xmodmap_known_files = {".xmodmap", ".xmodmaprc",
                                      ".Xmodmap", ".Xmodmaprc"};
    string m_default_layout = "";
    string m_default_variant = "";
    string m_default_option = "";

    public XKBLayout(IBus.Config? config) {
        m_config = config;

        if (config != null) {
            var value  = config.get_value("general", "xkb_latin_layouts");
            for (int i = 0; value != null && i < value.n_children(); i++) {
                m_xkb_latin_layouts +=
                        value.get_child_value(i).dup_string();
            }
            if (m_use_xmodmap) {
                m_use_xmodmap = config.get_value("general", "use_xmodmap").get_boolean();
            }
        }
    }

    private string get_output_from_cmdline(string arg, string element) {
        string[] exec_command = {};
        exec_command += m_xkb_command;
        exec_command += arg;
        string standard_output = null;
        string standard_error = null;
        int exit_status = 0;
        string retval = "";
        try {
            GLib.Process.spawn_sync(null,
                                    exec_command,
                                    null,
                                    GLib.SpawnFlags.SEARCH_PATH,
                                    null,
                                    out standard_output,
                                    out standard_error,
                                    out exit_status);
        } catch (GLib.SpawnError err) {
            stderr.printf("IBUS_ERROR: %s\n", err.message);
        }
        if (exit_status != 0) {
            stderr.printf("IBUS_ERROR: %s\n", standard_error ?? "");
        }
        if (standard_output == null) {
            return "";
        }
        foreach (string line in standard_output.split("\n")) {
            if (element.length <= line.length &&
                line[0:element.length] == element) {
                retval = line[element.length:line.length];
                if (retval == null) {
                    retval = "";
                } else {
                    retval = retval.strip();
                }
            }
        }
        return retval;
    }

    private void set_layout_cb(GLib.Pid pid, int status) {
        if (m_xkb_pid != pid) {
            stderr.printf("IBUS_ERROR: set_layout_cb has another pid\n");
            return;
        }
        GLib.Process.close_pid(m_xkb_pid);
        m_xkb_pid = -1;
        set_xmodmap();
    }

    private void set_xmodmap_cb(GLib.Pid pid, int status) {
        if (m_xmodmap_pid != pid) {
            stderr.printf("IBUS_ERROR: set_xmodmap_cb has another pid\n");
            return;
        }
        GLib.Process.close_pid(m_xmodmap_pid);
        m_xmodmap_pid = -1;
    }

    private string get_fullpath(string command) {
        string envpath = GLib.Environment.get_variable("PATH");
        foreach (string dir in envpath.split(":")) {
            string filepath = GLib.Path.build_filename(dir, command);
            if (GLib.FileUtils.test(filepath, GLib.FileTest.EXISTS)) {
                return filepath;
            }
        }
        return "";
    }

    private string[] get_xkb_group_layout (string layout,
                                           string variant,
                                           int layouts_max_length) {
        int group_id = 0;
        int i = 0;
        string[] layouts = m_default_layout.split(",");
        string[] variants = m_default_variant.split(",");
        string group_layouts = "";
        string group_variants = "";
        bool has_variant = false;
        bool include_keymap = false;

        for (i = 0; i < layouts.length; i++) {
            if (i >= layouts_max_length - 1) {
                break;
            }

            if (i == 0) {
                group_layouts = layouts[i];
            } else {
                group_layouts = "%s,%s".printf(group_layouts, layouts[i]);
            }

            if (i >= variants.length) {
                if (i == 0) {
                    group_variants = "";
                } else {
                    group_variants += ",";
                }
                if (layout == layouts[i] && variant == "") {
                    include_keymap = true;
                    group_id = i;
                }
                continue;
            }
            if (layout == layouts[i] && variant == variants[i]) {
                include_keymap = true;
                group_id = i;
            }

            if (variants[i] != "") {
                has_variant = true;
            }

            if (i == 0) {
                group_variants = variants[i];
            } else {
                group_variants = "%s,%s".printf(group_variants, variants[i]);
            }
        }

        if (variant != "") {
            has_variant = true;
        }

        if (!include_keymap) {
            group_layouts = "%s,%s".printf(group_layouts, layout);
            group_variants = "%s,%s".printf(group_variants, variant);
            group_id = i;
        }

        if (!has_variant) {
            group_variants = null;
        }

        return {group_layouts, group_variants, group_id.to_string()};
    }

    public string[] get_variant_from_layout(string layout) {
        int left_bracket = layout.index_of("(");
        int right_bracket = layout.index_of(")");
        if (left_bracket >= 0 && right_bracket > left_bracket) {
            return {layout[0:left_bracket] +
                    layout[right_bracket + 1:layout.length],
                    layout[left_bracket + 1:right_bracket]};
        }
        return {layout, "default"};
    }

    public string[] get_option_from_layout(string layout) {
        int left_bracket = layout.index_of("[");
        int right_bracket = layout.index_of("]");
        if (left_bracket >= 0 && right_bracket > left_bracket) {
            return {layout[0:left_bracket] +
                    layout[right_bracket + 1:layout.length],
                    layout[left_bracket + 1:right_bracket]};
        }
        return {layout, "default"};
    }

    public string get_layout() {
        return get_output_from_cmdline("-query", "layout: ");
    }

    public string get_variant() {
        return get_output_from_cmdline("-query", "variant: ");
    }

    public string get_option() {
        return get_output_from_cmdline("-query", "options: ");
    }

    /*
    public string get_group() {
        return get_output_from_cmdline("--get-group", "group: ");
    }
    */

    public int[] set_layout(IBus.EngineDesc engine,
                            bool use_group_layout=false) {
        string layout = engine.get_layout();
        string variant = engine.get_layout_variant();
        string option = engine.get_layout_option();

        assert (layout != null);

        int xkb_group_id = 0;
        int changed_option = 0;

        if (m_xkb_pid != -1) {
            return {-1, 0};
        }

        if (layout == "default" &&
            (variant == "default" || variant == "") &&
            (option == "default" || option == "")) {
            return {-1, 0};
        }

        bool need_us_layout = false;
        foreach (string latin_layout in m_xkb_latin_layouts) {
            if (layout == latin_layout && variant != "eng") {
                need_us_layout = true;
                break;
            }
            if (variant != null &&
                "%s(%s)".printf(layout, variant) == latin_layout) {
                need_us_layout = true;
                break;
            }
        }

        int layouts_max_length =  XKB_LAYOUTS_MAX_LENGTH;
        if (need_us_layout) {
            layouts_max_length--;
        }

        if (m_default_layout == "") {
            m_default_layout = get_layout();
        }
        if (m_default_variant  == "") {
            m_default_variant  = get_variant();
        }
        if (m_default_option == "") {
            m_default_option = get_option();
        }

        if (layout == "default") {
            layout = m_default_layout;
            variant = m_default_variant;
        } else {
            if (HAVE_IBUS_GKBD) {
                if (variant == "default") {
                    variant = "";
                }
                string[] retval = get_xkb_group_layout (layout, variant,
                                                        layouts_max_length);
                layout = retval[0];
                variant = retval[1];
                xkb_group_id = int.parse(retval[2]);
            }
        }

        if (layout == "") {
            warning("Could not get the correct layout");
            return {-1, 0};
        }

        if (variant == "default" || variant == "") {
            variant = null;
        }

        if (option == "default" || option == "") {
            option = m_default_option;
        } else {
            if (!(option in m_default_option.split(","))) {
                option = "%s,%s".printf(m_default_option, option);
                changed_option = 1;
            } else {
                option = m_default_option;
            }
        }

        if (option == "") {
            option = null;
        }

        if (need_us_layout) {
            layout += ",us";
            if (variant != null) {
                variant += ",";
            }
        }

        string[] args = {};
        args += m_xkb_command;
        args += "-layout";
        args += layout;
        if (variant != null) {
            args += "-variant";
            args += variant;
        }
        if (option != null) {
            /* TODO: Need to get the session XKB options */
            args += "-option";
            args += "-option";
            args += option;
        }

        GLib.Pid child_pid;
        try {
            GLib.Process.spawn_async(null,
                                     args,
                                     null,
                                     GLib.SpawnFlags.DO_NOT_REAP_CHILD |
                                         GLib.SpawnFlags.SEARCH_PATH,
                                     null,
                                     out child_pid);
        } catch (GLib.SpawnError err) {
            stderr.printf("Execute setxkbmap failed: %s\n", err.message);
            return {-1, 0};
        }
        m_xkb_pid = child_pid;
        GLib.ChildWatch.add(m_xkb_pid, set_layout_cb);

        return {xkb_group_id, changed_option};
    }

    public void set_xmodmap() {
        if (!m_use_xmodmap) {
            return;
        }

        if (m_xmodmap_pid != -1) {
            return;
        }

        string xmodmap_cmdpath = get_fullpath(m_xmodmap_command);
        if (xmodmap_cmdpath == "") {
            xmodmap_cmdpath = m_xmodmap_command;
        }
        string homedir = GLib.Environment.get_home_dir();
        foreach (string xmodmap_file in m_xmodmap_known_files) {
            string xmodmap_filepath = GLib.Path.build_filename(homedir, xmodmap_file);
            if (!GLib.FileUtils.test(xmodmap_filepath, GLib.FileTest.EXISTS)) {
                continue;
            }
            string[] args = {xmodmap_cmdpath, xmodmap_filepath};

            GLib.Pid child_pid;
            try {
                GLib.Process.spawn_async(null,
                                         args,
                                         null,
                                         GLib.SpawnFlags.DO_NOT_REAP_CHILD |
                                             GLib.SpawnFlags.SEARCH_PATH,
                                         null,
                                         out child_pid);
            } catch (GLib.SpawnError err) {
                stderr.printf("IBUS_ERROR: %s\n", err.message);
                return;
            }
            m_xmodmap_pid = child_pid;
            GLib.ChildWatch.add(m_xmodmap_pid, set_xmodmap_cb);

            break;
        }
    }

    public void reset_layout() {
        m_default_layout = get_layout();
        m_default_variant = get_variant();
        m_default_option = get_option();
    }

    /*
    public static int main(string[] args) {
        IBus.Bus bus = new IBus.Bus();
        IBus.Config config = bus.get_config();
        XKBLayout xkblayout = new XKBLayout(config);
        stdout.printf ("layout: %s\n", xkblayout.get_layout());
        stdout.printf ("variant: %s\n", xkblayout.get_variant());
        stdout.printf ("option: %s\n", xkblayout.get_option());
        xkblayout.set_layout("jp");
        if (config != null) {
            IBus.main();
        } else {
            Gtk.init (ref args);
            Gtk.main();
        }
        return 0;
    }
    */
}
