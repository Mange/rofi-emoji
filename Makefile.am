ACLOCAL_AMFLAGS=-I m4
plugindir=${libdir}/rofi/

plugin_LTLIBRARIES = emoji.la

dist_pkgdata_DATA = emoji-test.txt README.md screenshot.png

emoji_la_SOURCES=\
		 src/emoji.c \
		 src/emoji_list.c \
		 src/loader.c \
		 src/plugin.c

emoji_la_CFLAGS= @glib_CFLAGS@ @rofi_CFLAGS@
emoji_la_LIBADD= @glib_LIBS@ @rofi_LIBS@
emoji_la_LDFLAGS= -module -avoid-version
