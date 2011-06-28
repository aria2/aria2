# Russian translation of man page.
mandir=@mandir@/ru
man_MANS = aria2c.1
EXTRA_DIST = $(man_MANS)
doc_rudir = $(docdir)/ru
dist_doc_ru_DATA = aria2c.1.html aria2c.1.asciidoc

if HAVE_A2X
aria2c.1: aria2c.1.asciidoc
	@A2X@ -a lang=ru -f manpage aria2c.1.asciidoc
endif # HAVE_A2X

if HAVE_ASCIIDOC
aria2c.1.html: aria2c.1.asciidoc
	@ASCIIDOC@ -a lang=ru -d manpage -b xhtml11 -a toc -a max-width=55em aria2c.1.asciidoc
endif # HAVE_ASCIIDOC
