# vim:set et ts=4:
#
# ibus-xkb - IBus XKB
#
# Copyright (c) 2011-2012 Takao Fujiwara <takao.fujiwara1@gmail.com>
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
AC_PREFEQ([2.62])

AC_CONFIG_HEADERS([config.h])

# if not 1, append datestamp to the version number.
m4_define([ibus_released], [0])
m4_define([ibus_major_version], [1])
m4_define([ibus_minor_version], [4])
m4_define([ibus_micro_version], [99])
m4_define([ibus_abi_current], [5])
m4_define([ibus_abi_revision],
          [m4_eval(100 * ibus_minor_version + ibus_micro_version)])
m4_define([ibus_abi_age], [0])
m4_define(ibus_maybe_datestamp,
    m4_esyscmd([if test x]ibus_released[ != x1; then date +.%Y%m%d | tr -d '\n\r'; fi]))
m4_define([ibus_xkb_version],
    ibus_major_version.ibus_minor_version.ibus_micro_version[]ibus_maybe_datestamp)
# This is the X.Y used in -libus-X.Y
m4_define([ibus_xkb_api_version], [ibus_major_version.0])
m4_define([ibus_abi_current_minus_age],
          [m4_eval(ibus_abi_current - ibus_abi_age)])
m4_define([ibus_xkb_binary_version],
          [ibus_major_version.ibus_abi_current_minus_age.ibus_abi_age.ibus_abi_revision])

# Required versions of other packages
m4_define([ibus_required_version], [1.4.99])
m4_define([glib_required_version], [2.26.0])


AC_INIT([ibus-xkb], [ibus_xkb_version],
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
IBUS_XKB_ABI_CURRENT=ibus_abi_current
IBUS_XKB_ABI_REVISION=ibus_abi_revision
IBUS_XKB_ABI_AGE=ibus_abi_age
IBUS_XKB_VERSION=ibus_xkb_version
IBUS_XKB_API_VERSION=ibus_xkb_api_version
IBUS_XKB_BINARY_VERSION=ibus_xkb_binary_version

AC_SUBST(IBUS_XKB_MAJOR_VERSION)
AC_SUBST(IBUS_XKB_MINOR_VERSION)
AC_SUBST(IBUS_XKB_MICRO_VERSION)
AC_SUBST(IBUS_XKB_ABI_CURRENT)
AC_SUBST(IBUS_XKB_ABI_REVISION)
AC_SUBST(IBUS_XKB_ABI_AGE)
AC_SUBST(IBUS_XKB_VERSION)
AC_SUBST(IBUS_XKB_API_VERSION)
AC_SUBST(IBUS_XKB_BINARY_VERSION)

# Check for programs
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_CC_STDC
AM_PROG_VALAC([0.14])
AC_PROG_INSTALL
AC_PROG_MAKE_SET

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

# libtool versioning
#
# If LT_VERSION_INFO="lt_current:lt_revision:lt_age", libibus is
# libibus-ibus_api_version.so.(lt_current - lt_age).lt_age.lt_revision
# If the abi is changed, but it is compatible with last version,
# ibus_abi_current++, ibus_abi_age++;
# If the abi is not compatible with last version,
# ibus_abi_current++, ibus_abi_age = 0;
m4_define([lt_current], [ibus_abi_current])
m4_define([lt_revision], [ibus_abi_revision])
m4_define([lt_age], [ibus_abi_age])
LT_VERSION_INFO="lt_current:lt_revision:lt_age"
AC_SUBST(LT_VERSION_INFO)

# Check for ibus 
PKG_CHECK_MODULES(IBUS, [
    ibus-1.0 >= ibus_required_version
])

# Check glib2
AM_PATH_GLIB_2_0
PKG_CHECK_MODULES(GLIB2, [
    glib-2.0 >= glib_required_version
])
PKG_CHECK_MODULES(GOBJECT2, [
    gobject-2.0 >= glib_required_version
])
PKG_CHECK_MODULES(GIO2, [
    gio-2.0 >= glib_required_version
])
PKG_CHECK_MODULES(GTHREAD2, [
    gthread-2.0 >= glib_required_version
])

# --disable-gtk3 option.
AC_ARG_ENABLE(gtk3,
    AS_HELP_STRING([--disable-gtk3],
                   [Do not build gtk3 im module]),
    [enable_gtk3=$enableval],
    [enable_gtk3=yes]
)
AM_CONDITIONAL([ENABLE_GTK3], [test x"$enable_gtk3" = x"yes"])

if test x"$enable_gtk3" = x"yes"; then
    # check for gtk3, gdk3
    PKG_CHECK_MODULES(GTK3, [
        gtk+-3.0
    ])

    PKG_CHECK_MODULES(GDK3, [
        gdk-3.0
    ])
else
    enable_gtk3="no (disabled, use --enable-gtk3 to enable)"
fi

# For source compatibility with ibus patch.
AM_CONDITIONAL([ENABLE_XKB], [1])

# Check for x11
PKG_CHECK_MODULES(X11, [
    x11
])
AC_CHECK_HEADERS([X11/XKBlib.h])

PKG_CHECK_MODULES(XKB,
    [xkbfile],,
    [XKB_LIBS="-lxkbfile"]
)

# For source compatibility with ibus patch.
AC_DEFINE(HAVE_XKB, 1, [define to 1 if you have xkbfile])
HAVE_IBUS_XKB=true
AC_SUBST(HAVE_IBUS_XKB)

# --enable-libgnomekbd option.
AC_ARG_ENABLE(libgnomekbd,
    AS_HELP_STRING([--enable-libgnomekbd],
                   [Use libgnomekbd to handle the keymaps]),
    [enable_libgnomekbd=$enableval],
    [enable_libgnomekbd=no]
)
AM_CONDITIONAL([ENABLE_LIBGNOMEKBD], [test x"$enable_libgnomekbd" = x"yes"])
if test x"$enable_libgnomekbd" = x"yes"; then
    # check for libgnomekbd
    PKG_CHECK_MODULES(LIBGNOMEKBDUI, [
        libgnomekbdui
    ])
    PKG_CHECK_MODULES(ATK, [
        atk
    ])
    HAVE_IBUS_GKBD=true
else
    enable_libgnomekbd="no (disabled, use --enable-libgnomekbd to enable)"
    HAVE_IBUS_GKBD=false
fi
AC_SUBST(HAVE_IBUS_GKBD)

# Define XKB rules file
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
engine/Makefile
engine/xkb.xml.in
setup/Makefile
setup/ibus-setup-xkb
src/Makefile
ui/Makefile
ui/gtk3/Makefile
ui/gtk3/gtkxkbpanel.xml.in
])

AC_OUTPUT
AC_MSG_RESULT([
Build options:
  Version                   $IBUS_XKB_VERSION
  Install prefix            $prefix
  Build ui gtk3             $enable_gtk3
  Build gconf modules       $enable_gconf
  Build python modules      $enable_python
  Build introspection       $found_introspection
  IBus-1.0.gir scannerflags "$IBUS_GIR_SCANNERFLAGS"
  Build libgnomebkd         $enable_libgnomekbd
])
