man_MANS = aria2c.1
EXTRA_DIST = $(man_MANS)
dist_doc_DATA = aria2c.1.html aria2c.1.asciidoc
doc_xmlrpcdir = $(docdir)/xmlrpc
dist_doc_xmlrpc_DATA = xmlrpc/aria2mon \
	xmlrpc/aria2rpc \
	xmlrpc/README.txt

aria2c.1: aria2c.1.asciidoc
	a2x -f manpage aria2c.1.asciidoc

aria2c.1.html: aria2c.1.asciidoc
	asciidoc -d manpage -b xhtml11 -a toc -a max-width=55em aria2c.1.asciidoc
