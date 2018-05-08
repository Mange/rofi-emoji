include $(top_srcdir)/m4/glib-tap.mk

ACLOCAL_AMFLAGS=-I m4
plugindir=${libdir}/rofi/

# test binaries
test_programs = \
		src/tests/foo

# data distributed in the tarball
dist_test_data = \
		emoji-test.txt

plugin_LTLIBRARIES = emoji.la

emoji_la_SOURCES=\
		src/emoji.c \
		src/emoji_list.c \
		src/loader.c \
		src/plugin.c

AM_CFLAGS= @glib_CFLAGS@ @rofi_CFLAGS@
emoji_la_LIBADD= @glib_LIBS@ @rofi_LIBS@
src_tests_foo_LDADD= @glib_LIBS@ @rofi_LIBS@
emoji_la_LDFLAGS= -module -avoid-version
