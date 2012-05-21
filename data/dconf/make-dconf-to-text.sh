#!/bin/bash

set -e

export TMPDIR=$(mktemp -d --tmpdir="$PWD")
export XDG_CONFIG_HOME="$TMPDIR/config"
export XDG_CACHE_HOME="$TMPDIR/cache"
export GSETTINGS_SCHEMA_DIR="$TMPDIR/schemas"
mkdir -p $XDG_CONFIG_HOME $XDG_CACHE_HOME $GSETTINGS_SCHEMA_DIR

eval `dbus-launch --sh-syntax`

trap 'rm -rf $TMPDIR; kill $DBUS_SESSION_BUS_PID' ERR

# in case that schema is not installed on the system
glib-compile-schemas --targetdir "$GSETTINGS_SCHEMA_DIR" "$PWD"

OUTPUT=$(mktemp -u --tmpdir="$PWD")

printf "%s\n" "
# This file is used to merge the dconf values into /desktop/ibus
#
# env DCONF_PROFILE=ibus dconf update
" > "$OUTPUT"

printf '[desktop/ibus/general]\n' >> "$OUTPUT"
schema='org.freedesktop.ibus.xkb'
gsettings list-keys $schema | \
while read key; do
    val=$(gsettings get $schema $key)
    printf "%s=%s\n" "$key" "$val" >> "$OUTPUT"
done

printf "\n" >> "$OUTPUT"

printf '[desktop/ibus/general/hotkey]\n' >> "$OUTPUT"
schema='org.freedesktop.ibus.xkb.hotkey'
gsettings list-recursively org.freedesktop.ibus.xkb.hotkey | \
while read schema key val; do
    printf "%s=%s\n" "$key" "$val" >> "$OUTPUT"
done

mv "$OUTPUT" "$1"
rm -rf $TMPDIR

kill $DBUS_SESSION_BUS_PID
