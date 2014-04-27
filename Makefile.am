SUBDIRS =  po lib deps src doc test

ACLOCAL_AMFLAGS = -I m4 --install
RST2HTML = @RST2HTML@

EXTRA_DIST = config.rpath \
	     script-helper \
	     makerelease \
	     mingw-config mingw-release mingw-build-memo \
	     android-config android-make \
	     makerelease-osx.mk osx-package/DS_Store osx-package/etc/paths.d/aria2c osx-package/etc/manpaths.d/aria2 \
	     examples/libaria2ex.cc examples/libaria2wx.cc

dist_doc_DATA = README README.rst README.html

if HAVE_RST2HTML

README.html: README.rst
	$(RST2HTML)  $< > README.html
endif # HAVE_RST2HTML

dist_noinst_DATA = LICENSE.OpenSSL

