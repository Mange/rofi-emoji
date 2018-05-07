ACLOCAL_AMFLAGS=-I m4
plugindir=${libdir}/rofi/

plugin_LTLIBRARIES = emoji.la

emoji_la_SOURCES=\
		 src/emoji.c

emoji_la_CFLAGS= @glib_CFLAGS@ @rofi_CFLAGS@
emoji_la_LIBADD= @glib_LIBS@ @rofi_LIBS@
emoji_la_LDFLAGS= -module -avoid-version
