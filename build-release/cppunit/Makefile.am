AUTOMAKE_OPTIONS = 1.4
ACLOCAL_AMFLAGS = -I config

SUBDIRS = src include examples doc

pkgconfigdatadir = $(libdir)/pkgconfig

pkgconfigdata_DATA = cppunit.pc

bin_SCRIPTS = cppunit-config
man_MANS = cppunit-config.1


EXTRA_DIST = BUGS INSTALL-unix INSTALL-WIN32.txt CodingGuideLines.txt	\
	cppunit-config.1				                \
	cppunit.m4 cppunit.spec.in cppunit.spec	        \
	$(m4sources)					                \
	contrib/msvc/CppUnit.WWTpl 			            \
	contrib/msvc/readme.txt                         \
	contrib/msvc/AddingUnitTestMethod.dsm           \
	contrib/bc5/bcc-makefile.zip \
	contrib/xml-xsl/tests.xml \
	contrib/xml-xsl/report.xsl \
	src/CppUnitLibraries.dsw \
	lib/.keepme

m4sources = \
	config/ac_create_prefix_config_h.m4		\
	config/ac_cxx_have_sstream.m4			\
	config/ac_cxx_have_strstream.m4			\
	config/ax_cxx_gcc_abi_demangle.m4		\
	config/ac_cxx_namespaces.m4			\
	config/ac_cxx_rtti.m4				\
	config/ac_cxx_string_compare_string_first.m4	\
	config/bb_enable_doxygen.m4			\
	config/ac_dll.m4

m4datadir = $(datadir)/aclocal
m4data_DATA = cppunit.m4

# Not sure what is creating the timestamp file.
# The so_locations file only happens on IRIX.
DISTCLEANFILES = config/stamp-h1 so_locations

dist-hook:
	cp -dpR $(top_srcdir)/src/msvc6 $(distdir)/src
	cp -dpR $(top_srcdir)/src/qttestrunner $(distdir)/src
	cp -dpR $(top_srcdir)/include/msvc6 $(distdir)/include 
	cp -dpR $(top_srcdir)/examples/msvc6 $(distdir)/examples
	cp -dpR $(top_srcdir)/examples/qt $(distdir)/examples
	test -d $(distdir)/lib || mkdir $(distdir)/lib
	find $(distdir) -name CVS | xargs rm -rf
	perl -pi -e 's/\n/\r\n/g' `find $(distdir) -name '*.ds?'` \
		$(distdir)/contrib/msvc/*			  \
		$(distdir)/INSTALL-WIN32.txt

.PHONY: release snapshot rpm docs doc-dist

release:
	rm -rf .deps */.deps
	$(MAKE) distcheck

snapshot:
	$(MAKE) dist distdir=$(PACKAGE)-`date +%Y-%m-%d`

rpm: dist
	rpm -ta $(PACKAGE)-$(VERSION).tar.gz
	mv -f /usr/src/redhat/SRPMS/$(PACKAGE)-$(VERSION)-*.rpm .
	mv -f /usr/src/redhat/RPMS/*/$(PACKAGE)-$(VERSION)-*.rpm .
	mv -f /usr/src/redhat/RPMS/*/$(PACKAGE)-doc-$(VERSION)-*.rpm .

debian:
	chmod a+x debian/rules
	dpkg-buildpackage -rfakeroot -sa -us -uc -tc 

doc-dist:
	$(MAKE) -C doc doc-dist
	mv -f doc/$(PACKAGE)-docs-$(VERSION).tar.gz .


