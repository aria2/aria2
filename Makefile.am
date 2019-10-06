SUBDIRS =  po lib deps src doc test

ACLOCAL_AMFLAGS = -I m4 --install
RST2HTML = @RST2HTML@

EXTRA_DIST = config.rpath \
	     script-helper \
	     makerelease \
	     mingw-config mingw-release mingw-build-memo \
	     android-config \
	     makerelease-osx.mk osx-package/DS_Store osx-package/etc/paths.d/aria2c osx-package/etc/manpaths.d/aria2 \
	     Dockerfile.mingw \
	     Dockerfile.raspberrypi \
	     examples/libaria2ex.cc examples/libaria2wx.cc

dist_doc_DATA = README README.rst README.html

.PHONY: clang-format

if HAVE_RST2HTML
README.html: README.rst
	$(RST2HTML)  $< > $@
else # !HAVE_RST2HTML
README.html: README.rst
	@echo Warning: rst2html not installed. Placing only a dummy file for $@
	echo "<!doctype html>" > $@
	echo "<p>Readme has not been built!.<br>Instead see <a href='README.rst'>README.rst</a></p>" >> $@
endif # !HAVE_RST2HTML

dist_noinst_DATA = LICENSE.OpenSSL

# Format source files using clang-format.  Don't format source files
# under deps directory since we are not responsible for their coding
# style.
clang-format:
	CLANGFORMAT=`git config --get clangformat.binary`; \
	test -z $${CLANGFORMAT} && CLANGFORMAT="clang-format"; \
	$${CLANGFORMAT} -i $(top_srcdir)/src/*.{c,cc,h} $(top_srcdir)/src/includes/aria2/*.h \
	$(top_srcdir)/examples/*.cc $(top_srcdir)/test/*.{cc,h}
