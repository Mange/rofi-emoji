#!/bin/sh
# Run this script in order to compile the plugin and run it in Rofi. This
# allows you to test this plugin while developing without having to `make
# install` it on every change, which would also clobber any OS-managed
# files if you have it installed that way.
set -eu

[ ! -f configure ] && autoreconf -i
[ ! -d build ] && mkdir build

cd build
[ ! -e .xdg ] && mkdir .xdg && ln -s ../.. .xdg/rofi-emoji
[ ! -f config.h ] && ../configure

make

if [ -z "${XDG_DATA_DIRS:-}" ]; then
  xdg_data_dirs="$(pwd)/.xdg"
else
  xdg_data_dirs="$(pwd)/.xdg:${XDG_DATA_DIRS}"
fi

XDG_DATA_DIRS="$xdg_data_dirs" \
  rofi \
  -plugin-path "$(pwd)/.libs" \
  -modi emoji \
  -show emoji \
  "$@"
