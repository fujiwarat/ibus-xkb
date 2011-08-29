# vim:set et ts=4:
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
AC_PREFEQ([2.62])

AC_CONFIG_HEADERS([config.h])

# if not 1, append datestamp to the version number.
m4_define([ibus_released], [0])
m4_define([ibus_major_version], [1])
m4_define([ibus_minor_version], [3])
m4_define([ibus_micro_version], [99])
m4_define(ibus_maybe_datestamp,
    m4_esyscmd([if test x]ibus_released[ != x1; then date +.%Y%m%d | tr -d '\n\r'; fi]))
m4_define([ibus_version],
    ibus_major_version.ibus_minor_version.ibus_micro_version[]ibus_maybe_datestamp)
# This is the X.Y used in -libus-X.Y
m4_define([ibus_xkb_api_version], [1.0])


# Required versions of other packages
m4_define([ibus_required_version], [1.3.0])
m4_define([glib_required_version], [2.26.0])


AC_INIT([ibus-xkb], [ibus_version],
        [http://code.google.com/p/ibus/issues/entry],
        [ibus-xkb])

# Init automake
AM_INIT_AUTOMAKE([1.10])
AM_MAINTAINER_MODE([enable])
AC_GNU_SOURCE

# Support silent build
m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

# Define sustituted variables:
IBUS_XKB_MAJOR_VERSION=ibus_major_version
IBUS_XKB_MINOR_VERSION=ibus_minor_version
IBUS_XKB_MICRO_VERSION=ibus_micro_version
IBUS_XKB_API_VERSION=ibus_xkb_api_version
AC_SUBST(IBUS_XKB_MAJOR_VERSION)
AC_SUBST(IBUS_XKB_MINOR_VERSION)
AC_SUBST(IBUS_XKB_MICRO_VERSION)
AC_SUBST(IBUS_XKB_API_VERSION)

# Check for programs
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_CC_STDC
AC_PROG_INSTALL
AC_PROG_CXX

# define PACKAGE_VERSION_* variables
AM_DISABLE_STATIC
AC_ISC_POSIX
AC_HEADER_STDC
AM_PROG_LIBTOOL
IT_PROG_INTLTOOL([0.35.0])

# dislpay Date
m4_define(ibus_datedisplay,
    m4_esyscmd(date '+%a %b %d %Y' | tr -d '\n\r'))
DATE_DISPLAY="ibus_datedisplay"
AC_SUBST(DATE_DISPLAY)

# If only source code changed, lt_revision + 1
# If any interface added, lt_age + 1
# If any interfaces changed or removed, lt_current + 1, lt_revision = 0, lt_age = 0
m4_define([lt_current], [0])
m4_define([lt_revision], [0])
m4_define([lt_age], [0])
LT_VERSION_INFO="lt_current:lt_revision:lt_age"
AC_SUBST(LT_VERSION_INFO)

# Check for ibus 
PKG_CHECK_MODULES(IBUS, [
    ibus-1.0 >= ibus_required_version
])

# check glib2
AM_PATH_GLIB_2_0
PKG_CHECK_MODULES(GLIB2, [
    glib-2.0 >= glib_required_version
])
PKG_CHECK_MODULES(GOBJECT2, [
    gobject-2.0 >= glib_required_version
])

# Check for x11
PKG_CHECK_MODULES(X11, [
    x11
])
AC_CHECK_HEADERS([X11/XKBlib.h])

PKG_CHECK_MODULES(XKB,
    [xkbfile],,
    [XKB_LIBS="-lxkbfile"]
)

# define XKB rules file
AC_ARG_WITH(xkb-rules-xml,
    AS_HELP_STRING([--with-xkb-rules-xml[=$DIR/evdev.xml]],
        [Set evdev.xml file path (default: /usr/share/X11/xkb/rules/evdev.xml)]),
    XKB_RULES_XML_FILE=$with_xkb_rules_xml,
    XKB_RULES_XML_FILE="/usr/share/X11/xkb/rules/evdev.xml"
)
AC_DEFINE_UNQUOTED(XKB_RULES_XML_FILE, "$XKB_RULES_XML_FILE",
    [Define file path of evdev.xml])
AC_SUBST(XKB_RULES_XML_FILE)

# define XKB preload layouts
AC_ARG_WITH(xkb-preload-layouts,
    AS_HELP_STRING([--with-xkb-preload-layouts[=layout,...]],
        [Set preload xkb layouts (default: us,fr,de,...)]),
    XKB_PRELOAD_LAYOUTS=$with_xkb_preload_layouts,
    [XKB_PRELOAD_LAYOUTS=""\
"us,us(chr),us(dvorak),ad,al,am,ara,az,ba,bd,be,bg,br,bt,by,"\
"de,dk,ca,ch,cn(tib),cz,ee,epo,es,et,fi,fo,fr,"\
"gb,ge,ge(dsb),ge(ru),ge(os),gh,gh(akan),gh(ewe),gh(fula),gh(ga),gh(hausa),"\
"gn,gr,hu,hr,ie,ie(CloGaelach),il,"\
"in,in(ben),in(guj),in(guru),in(jhelum),in(kan),in(mal),in(ori),in(tam),"\
"in(tel),in(urd-phonetic),in(bolnagri),iq,iq(ku),ir,ir(ku),is,it,"\
"kg,kh,kz,la,latam,lk,lk(tam_unicode),lt,lv,ma,ma(tifinagh),mal,mao,"\
"me,mk,mm,mt,mv,ng,ng(hausa),ng,ng(igbo),ng(yoruba),nl,no,no(smi),np,"\
"pk,pl,pl(csb),pt,ro,rs,ru,ru(cv),ru(kom),ru(sah),ru(tt),ru(xal),"\
"se,si,sk,sy,sy(ku),th,tj,tr,ua,uz,vn"]
)
AC_SUBST(XKB_PRELOAD_LAYOUTS)

# GObject introspection
GOBJECT_INTROSPECTION_CHECK([0.6.8])

IBUS_GIR_SCANNERFLAGS=
if test x"$found_introspection" = x"yes" ; then
    IBUS_GIR_SCANNERFLAGS="--identifier-prefix=IBusXKB --symbol-prefix=ibus_xkb"
    PKG_CHECK_EXISTS([gobject-introspection-1.0 >= 0.9.6],
                     [gir_symbol_prefix=yes],
                     [gir_symbol_prefix=no])
    if test x"$gir_symbol_prefix" = x"no" ; then
        IBUS_GIR_SCANNERFLAGS="--strip-prefix=IBusXKB"
    fi
fi
AC_SUBST(IBUS_GIR_SCANNERFLAGS)

AC_ARG_ENABLE(gconf,
    AS_HELP_STRING([--disable-gconf],
                   [Do not use GConf code]),
    [enable_gconf=$enableval],
    [enable_gconf=yes]
)
AM_CONDITIONAL([ENABLE_GCONF], [test x"$enable_gconf" = x"yes"])

if test x"$enable_gconf" = x"yes"; then
    # check gconf
    PKG_CHECK_MODULES(GCONF,
        [gconf-2.0 >= 2.12],
    )

    AC_PATH_PROG(GCONFTOOL, gconftool-2, no)
    if test x"$GCONFTOOL" = xno; then
        AC_MSG_ERROR([gconftool-2 executable not found in your path - should be installed with GConf])
    fi

    AM_GCONF_SOURCE_2
    # GCONF_SCHEMAS_INSTALL should be set in macro AM_GCONF_SOURCE_2
else
    AM_CONDITIONAL([GCONF_SCHEMAS_INSTALL], [false])
    enable_gconf="no (disabled, use --enable-gconf to enable)"
fi

AC_ARG_ENABLE(python,
    AS_HELP_STRING([--disable-python],
                   [Do not use Python code]),
    [enable_python=$enableval],
    [enable_python=yes]
)

AM_CONDITIONAL([ENABLE_PYTHON], [test x"$enable_python" = x"yes"])
AM_CONDITIONAL([ENABLE_DAEMON], [true])

if test x"$enable_python" = x"yes"; then
    # check python
    AM_PATH_PYTHON([2.5])
    AC_PATH_PROG(PYTHON_CONFIG, python$PYTHON_VERSION-config)
    if test x"$PYTHON_CONFIG" = x""; then
        AC_PATH_PROG(PYTHON_CONFIG, python-config)
    fi
    if test x"$PYTHON_CONFIG" != x""; then
        PYTHON_CFLAGS=`$PYTHON_CONFIG --includes`
        PYTHON_LIBS=`$PYTHON_CONFIG --libs`
    else
        PYTHON_CFLAGS=`$PYTHON $srcdir/python-config.py --includes`
        PYTHON_LIBS=`$PYTHON $srcdir/python-config.py --libs`
    fi
    PYTHON_INCLUDES="$PYTHON_CFLAGS"
    AC_SUBST(PYTHON_CFLAGS)
    AC_SUBST(PYTHON_INCLUDES)
    AC_SUBST(PYTHON_LIBS)
else
    enable_python="no (disabled, use --enable-python to enable)"
fi

# define GETTEXT_* variables
GETTEXT_PACKAGE=ibus-xkb
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [Gettext package])

AM_GLIB_GNU_GETTEXT


# option for setup gui
AC_ARG_ENABLE(setup-gui,
    AS_HELP_STRING([--enable-setup-gui],
        [Enable Setup GUI]),
    [enable_setup_gui=$enableval],
    [enable_setup_gui=yes]
)

if test x"$enable_setup_gui" = x"yes"; then
    AC_DEFINE(ENABLE_SETUP_GUI, 1, [Define to 1 if you enable Setup GUI.])
else
    enable_setup_gui="no (disabled, use --enable-setup-gui to enable)"
fi

# option for bridge hotkey
AC_ARG_ENABLE(bridge-hotkey,
    AS_HELP_STRING([--enable-bridge-hotkey],
        [Enable bridge hotkey instead of ON/OFF hotkey]),
    [enable_bridge_hotkey=$enableval],
    [enable_bridge_hotkey=no]
)

if test x"$enable_bridge_hotkey" = x"yes"; then
    USE_BRIDGE_HOTKEY=1
else
    USE_BRIDGE_HOTKEY=0
    enable_bridge_hotkey="no (disabled, use --enable-bridge-hotkey to enable)"
fi
AC_SUBST(USE_BRIDGE_HOTKEY)

# define default bridge engine name
AC_ARG_WITH(bridge-engine,
    AS_HELP_STRING([--with-bridge-engine[=bridge_engine_name]],
        [Set bridge engine name in IM bridge hotkey. (default: xkb:layout:default:)]),
    [DEFAULT_BRIDGE_ENGINE_NAME=$with_bridge_engine],
    [DEFAULT_BRIDGE_ENGINE_NAME="xkb:layout:default:"]
)
AC_SUBST(DEFAULT_BRIDGE_ENGINE_NAME)

# check iso-codes
PKG_CHECK_MODULES(ISOCODES, [
    iso-codes
])
ISOCODES_PREFIX=`pkg-config iso-codes --variable=prefix`
AC_SUBST(ISOCODES_PREFIX)

# OUTPUT files
AC_CONFIG_FILES([ po/Makefile.in
Makefile
ibus-xkb.pc
ibus-xkb.spec
data/Makefile
ibus/Makefile
ibus/_config.py
src/Makefile
setup/Makefile
setup/ibus-setup-xkb
xkb/Makefile
])

AC_OUTPUT
AC_MSG_RESULT([
Build options:
  Version                   $VERSION
  Install prefix            $prefix
  Build gconf modules       $enable_gconf
  Build python modules      $enable_python
  Build introspection       $found_introspection
  IBus-1.0.gir scannerflags "$IBUS_GIR_SCANNERFLAGS"
  Enable Setup GUI          $enable_setup_gui
  Enable bridge hotkey      $enable_bridge_hotkey
  Default bridge engine     $DEFAULT_BRIDGE_ENGINE_NAME
])
