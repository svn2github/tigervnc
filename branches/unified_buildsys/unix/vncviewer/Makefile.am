bin_PROGRAMS = vncviewer

man1_MANS = vncviewer.man

HDRS = AboutDialog.h CConn.h DesktopWindow.h gettext.h InfoDialog.h \
	OptionsDialog.h parameters.h PasswdDialog.h ServerDialog.h

vncviewer_SOURCES = $(HDRS) DesktopWindow.cxx CConn.cxx vncviewer.cxx \
	buildtime.c
# X_CFLAGS are really CPPFLAGS
vncviewer_CPPFLAGS = -I$(top_srcdir)/common -I$(top_srcdir)/unix \
	-I$(top_srcdir)/unix/tx -I$(top_srcdir)/intl \
	-DLOCALEDIR=\"$(localedir)\" @X_CFLAGS@

vncviewer_LDADD = $(top_srcdir)/unix/tx/libtx.la \
	$(top_srcdir)/common/rfb/librfb.la \
	$(top_srcdir)/common/network/libnetwork.la \
	$(top_srcdir)/common/rdr/librdr.la \
	@X_PRE_LIBS@ @X_LIBS@ -lXext -lX11 @X_EXTRA_LIBS@ # @LIBINTL@

EXTRA_DIST = vncviewer.man
