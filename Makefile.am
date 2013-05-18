SUBDIRS =  po intl lib deps src doc test

ACLOCAL_AMFLAGS = -I m4 --install
RST2HTML = @RST2HTML@

EXTRA_DIST = config.rpath mingw-config android-config android-make\
	build_osx_release.sh \
	examples/libaria2ex.cc examples/libaria2wx.cc

dist_doc_DATA = README README.rst

if HAVE_RST2HTML
dist_doc_DATA += README.html
README.html: README.rst
	$(RST2HTML) README.rst > README.html
endif # HAVE_RST2HTML

dist_noinst_DATA = LICENSE.OpenSSL

