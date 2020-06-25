#!/bin/sh
# Run this script in order to compile the plugin and run it in Rofi. This
# allows you to test this plugin while developing without having to `make
# install` it on every change, which would also clobber any OS-managed
# files if you have it installed that way.
set -e

[ ! -f configure ] && autoreconf -i
[ ! -d build ] && mkdir build

cd build
[ ! -e .xdg ] && mkdir .xdg && ln -s ../.. .xdg/rofi-emoji
[ ! -f config.h ] && ../configure

make
XDG_DATA_DIRS="$(pwd)/.xdg" rofi -plugin-path "$(pwd)/.libs" -modi emoji -show emoji
