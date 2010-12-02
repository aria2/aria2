SUBDIRS =  po intl lib src test doc

ACLOCAL_AMFLAGS = -I m4 --install

EXTRA_DIST = config.rpath

dist_doc_DATA = README README.asciidoc README.html

dist_noinst_DATA = LICENSE.OpenSSL

if HAVE_ASCIIDOC
README.html: README.asciidoc
	@ASCIIDOC@ -d article -b xhtml11 -n README.asciidoc
endif # HAVE_ASCIIDOC