ACLOCAL_AMFLAGS=-I m4
plugindir=${rofi_PLUGIN_INSTALL_DIR}/

plugin_LTLIBRARIES = emoji.la

dist_pkgdata_DATA = all_emojis.txt README.md screenshot.png
dist_pkgdata_SCRIPTS = clipboard-adapter.sh

emoji_la_SOURCES=\
		 src/emoji.c \
		 src/utils.c \
		 src/loader.c \
		 src/formatter.c \
		 src/plugin.c

emoji_la_CFLAGS= @glib_CFLAGS@ @rofi_CFLAGS@ @cairo_CFLAGS@
emoji_la_LIBADD= @glib_LIBS@ @rofi_LIBS@ @cairo_LIBS@
emoji_la_LDFLAGS= -module -avoid-version

noinst_PROGRAMS = tests/check_utils tests/check_emoji
TESTS = tests/check_utils tests/check_emoji

tests_check_utils_SOURCES = tests/check_utils.c src/utils.c
tests_check_utils_CFLAGS = $(CFLAGS) $(CHECK_CFLAGS) @glib_CFLAGS@ @rofi_CFLAGS@ @cairo_CFLAGS@
tests_check_utils_LDADD = $(LDFLAGS) $(CHECK_LIBS) @glib_LIBS@ @rofi_LIBS@ @cairo_LIBS@

tests_check_emoji_SOURCES = tests/check_emoji.c src/emoji.c
tests_check_emoji_CFLAGS = $(CFLAGS) $(CHECK_CFLAGS) @glib_CFLAGS@ @rofi_CFLAGS@ @cairo_CFLAGS@
tests_check_emoji_LDADD = $(LDFLAGS) $(CHECK_LIBS) @glib_LIBS@ @rofi_LIBS@ @cairo_LIBS@
