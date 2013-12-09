# Any copyright is dedicated to the Public Domain.
# http://creativecommons.org/publicdomain/zero/1.0/
# Written by Nils Maier

# This make file will:
#  - Download a set of dependencies and verify the known-good hashes.
#  - Build static libraries of aria2 dependencies.
#  - Create a statically linked, universal build (i386,x86_64) aria2 release.
#    - The build will have all major features enabled, and will use
#      AppleTLS.
#  - Create a corresponding .tar.bz containing the binaries:
#  - Create a corresponding .pkg installer.
#  - Create a corresponding .dmg image containing said installer.
#
# This Makefile will also run all `make check` targets.
#
# The dependencies currently build are:
#  - zlib (compression, in particular web compression)
#  - c-ares (asynchronous DNS resolver)
#  - expat (XML parser, for metalinks)
#  - gmp (multi-precision arithmetric library, for DHKeyExchange, BitTorrent)
#  - sqlite3 (self-contained SQL database, for Firefox3 cookie reading)
#  - cppunit (unit tests for C++, framework in use by aria2 `make check`)
#
#
# To use this Makefile, do something along the lines of
#  - $ mkdir build-release
#  - $ cd build-release
#  - $ virtualenv .
#  - $ . bin/activate
#  - $ pip install sphinx-build
#  - $ ln -s ../makerelease-os.mk Makefile
#  - $ make
#
# Note: In theory, everything can be build in parallel, however the sub-makes
# will be called with an appropriate -j flag. Building the `deps` target in
# parallel before a general make might be beneficial, as the dependencies
# usually bottle-neck on the configure steps.
#
# Note: Of course, you need to have XCode with the command line tools
# installed for this to work, aka. a working compiler...
#
# Note: We're locally building the dependencies here, static libraries only.
# This is required, because when using brew or MacPorts, which also provide
# dynamic libraries, the linker will pick up the dynamic versions, always,
# with no way to instruct the linker otherwise.
# If you're building aria2 just for yourself and your system, using brewed
# libraries is fine as well.
#
# Note: This Makefile is riddled with mac-isms. It will not work on *nix.
#
# Note: The convoluted way to create separate arch builds and later merge them
# with lipo is because of two things:
#  1) Avoid patching c-ares, which hardcodes some sizes in its headers.
#  2) Make it easy in the future to enable -flto (currently, -flto builds crash)
#
# Note: This Makefile uses resources from doc/osx-package when creating the
# *.pkg and *.dmg targets

SHELL := bash

# A bit awkward, but OSX doesn't have a proper `readlink -f`.
SRCDIR := $(shell dirname $(lastword $(shell stat -f "%N %Y" $(lastword $(MAKEFILE_LIST)))))

# Same as in script-helper, but a bit easier on the eye (but more error prone)
# and Makefile compatible
BASE_VERSION := $(shell grep AC_INIT $(SRCDIR)/configure.ac | cut -d'[' -f3 | cut -d']' -f1)
ifeq ($(NON_RELEASE),)
	VERSION := $(BASE_VERSION)
else
ifeq ($(NON_RELEASE),force)
	VERSION := $(BASE_VERSION)
else
	VERSION := $(subst release-,,$(shell git describe --tags))
endif
endif

# Set up compiler.
CC ?= cc
export CC
CXX ?= c++
export CXX

# Set up compiler/linker flags.
OPTFLAGS ?= -Os
CFLAGS ?= -mmacosx-version-min=10.7 $(OPTFLAGS)
export CFLAGS
CXXFLAGS ?= -Os -mmacosx-version-min=10.7 $(OPTFLAGS)
export CXXFLAGS
LDFLAGS ?= -Wl,-dead_strip
export LDFLAGS


# Dependency versions
zlib_version = 1.2.8
zlib_hash = a4d316c404ff54ca545ea71a27af7dbc29817088
zlib_url = http://zlib.net/zlib-$(zlib_version).tar.gz

expat_version = 2.1.0
expat_hash = b08197d146930a5543a7b99e871cba3da614f6f0
expat_url = http://sourceforge.net/projects/expat/files/expat/$(expat_version)/expat-$(expat_version).tar.gz

cares_version = 1.10.0
cares_hash = e44e6575d5af99cb3a38461486e1ee8b49810eb5
cares_url = http://c-ares.haxx.se/download/c-ares-$(cares_version).tar.gz
cares_confflags = "--enable-optimize=$(OPTFLAGS)"

sqlite_version = autoconf-3080100
sqlite_hash = 42464b07df2d6f8aa28f73ce4cc6d48b47be810e
sqlite_url = http://sqlite.org/2013/sqlite-$(sqlite_version).tar.gz

gmp_version = 5.1.3
gmp_hash = b35928e2927b272711fdfbf71b7cfd5f86a6b165
gmp_url = https://ftp.gnu.org/gnu/gmp/gmp-$(gmp_version).tar.bz2
gmp_flags = --disable-cxx --enable-assembly

cppunit_version = 1.12.1
cppunit_hash = f1ab8986af7a1ffa6760f4bacf5622924639bf4a
cppunit_url = http://sourceforge.net/projects/cppunit/files/cppunit/$(cppunit_version)/cppunit-$(cppunit_version).tar.gz


# ARCHLIBS that can be template build
ARCHLIBS = expat cares sqlite gmp cppunit
# NONARCHLIBS that cannot be template build
NONARCHLIBS = zlib


# Tags
THIS_TAG := $(shell git describe --abbrev=0 $$(git rev-list --tags --max-count=1))
PREV_TAG := $(shell git describe --abbrev=0 $(THIS_TAG)~1)


# Aria2 setup
ARIA2 := aria2-$(VERSION)
ARIA2_PREFIX := $(PWD)/$(ARIA2)
ARIA2_DIST := $(PWD)/$(ARIA2)-osx-darwin
ARIA2_CONFFLAGS = \
        --enable-static \
        --disable-shared \
        --enable-metalink \
        --enable-bittorrent \
        --disable-nls \
        --with-appletls \
        --with-libgmp \
        --with-sqlite3 \
        --with-libz \
        --with-libexpat \
        --with-libcares \
        --without-libuv \
        --without-gnutls \
        --without-openssl \
        --without-libnettle \
        --without-libgcrypt \
        --without-libxml2 \
        ARIA2_STATIC=yes
ARIA2_DOCDIR = $(ARIA2_PREFIX)/share/doc/aria2
ARIA2_DOCS = \
	     $(ARIA2_DOCDIR)/AUTHORS \
	     $(ARIA2_DOCDIR)/COPYING \
	     $(ARIA2_DOCDIR)/NEWS
ARIA2_CHANGELOG = $(ARIA2_DOCDIR)/Changelog

# Yeah, inlined XML, go figure :p
define ARIA2_DISTXML
<?xml version="1.0" encoding="utf-8" standalone="no"?>
<installer-gui-script minSpecVersion="1">
	<title>aria1 $(VERSION)</title>
	<welcome file="README.html"/>
	<pkg-ref id="aria2"/>
	<pkg-ref id="aria2.paths"/>
	<options customize="never" require-scripts="false" rootVolumeOnly="true"/>
	<volume-check>
		<allowed-os-versions>
			<os-version min="10.7"/>
		</allowed-os-versions>
	</volume-check>
	<domains enable_anywhere="false" enable_currentUserHome="false" enable_localSystem="true"/>
	<choices-outline>
		<line choice="default">
			<line choice="aria2"/>
			<line choice="aria2.paths"/>
		</line>
	</choices-outline>
	<choice id="default"/>
	<choice id="aria2" visible="false">
		<pkg-ref id="aria2"/>
	</choice>
	<choice id="aria2.paths" visible="false">
		<pkg-ref id="aria2.paths"/>
	</choice>
	<pkg-ref id="aria2" version="$(VERSION)" onConclusion="none">out.pkg</pkg-ref>
	<pkg-ref id="aria2.paths" version="$(VERSION)" onConclusion="none">paths.pkg</pkg-ref>
</installer-gui-script>
endef
export ARIA2_DISTXML


# Detect numer of CPUs to be used with make -j
CPUS = $(shell sysctl hw.ncpu | cut -d" " -f2)


# default target
all::
	@if test "x$(NON_RELEASE)" = "x" && !(git describe --tags --exact); then \
		echo 'Not on a release tag; override by defining NON_RELEASE!'; \
		exit 1; \
	fi

# No dice without sphinx
all::
	@if test "x$$(which sphinx-build)" = "x"; then \
		echo "sphinx-build not present"; \
		exit 1; \
	fi;

deps::


# All those .PRECIOUS files, because otherwise gmake will treat them as
# intermediates and remove them when the build completes. Thanks gmake!
.PRECIOUS: %.tar.gz
%.tar.gz:
	curl -o $@ -A 'curl/0; like wget' -L \
		$($(basename $(basename $@))_url)

.PRECIOUS: %.check
%.check: %.tar.gz
	@if test "$$(shasum -a1 $< | awk '{print $$1}')" != "$($(basename $@)_hash)"; then \
		echo "Invalid $@ hash"; \
		rm -f $<; \
		exit 1; \
	fi;
	touch $@

.PRECIOUS: %.stamp
%.stamp: %.tar.gz %.check
	tar xf $<
	mv $(basename $@)-$($(basename $@)_version) $(basename $@)
	touch $@

.PRECIOUS: cares.stamp
cares.stamp: cares.tar.gz cares.check
	tar xzf $<
	mv c-ares-$($(basename $@)_version) $(basename $@)
	touch $@

# Using (NON)ARCH_template kinda stinks, but real multi-target pattern rules
# only exist in feverish dreams.
define NONARCH_template
$(1).build: $(1).x86_64.build $(1).i686.build
deps:: $(1).build
endef

.PRECIOUS: zlib.%.build
zlib.%.build: zlib.stamp
	$(eval BASE := $(basename $<))
	$(eval DEST := $(basename $@))
	$(eval ARCH := $(subst .,,$(suffix $(DEST))))
	rsync -a $(BASE)/ $(DEST)
	( cd $(DEST) && ./configure \
		--static --prefix=$(PWD)/$(ARCH) \
		)
	$(MAKE) -C $(DEST) -sj$(CPUS) CFLAGS="$(CFLAGS) -arch $(ARCH)"
	$(MAKE) -C $(DEST) -sj$(CPUS) CFLAGS="$(CFLAGS) -arch $(ARCH)" check
	$(MAKE) -C $(DEST) -s install
	touch $@

$(foreach lib,$(NONARCHLIBS),$(eval $(call NONARCH_template,$(lib))))

define ARCH_template
.PRECIOUS: $(1).%.build
$(1).%.build: $(1).stamp
	$$(eval DEST := $$(basename $$@))
	$$(eval ARCH := $$(subst .,,$$(suffix $$(DEST))))
	mkdir -p $$(DEST)
	( cd $$(DEST) && ../$(1)/configure \
		--host=$$(ARCH)-apple-darwin11.4.2 \
		--build=$$(ARCH)-apple-darwin11.4.2 \
		--enable-static --disable-shared \
		--prefix=$$(PWD)/$$(ARCH) \
		$$($(1)_confflags) \
		CFLAGS="$$(CFLAGS) -arch $$(ARCH)" \
		CXXFLAGS="$$(CXXFLAGS) -arch $$(ARCH) -stdlib=libc++ -std=c++11" \
		)
	$$(MAKE) -C $$(DEST) -sj$(CPUS)
	$$(MAKE) -C $$(DEST) -sj$(CPUS) check
	$$(MAKE) -C $$(DEST) -s install
	touch $$@

$(1).build: $(1).x86_64.build $(1).i686.build
deps:: $(1).build
endef

$(foreach lib,$(ARCHLIBS),$(eval $(call ARCH_template,$(lib))))

.PRECIOUS: aria2.%.build
aria2.%.build: zlib.%.build expat.%.build gmp.%.build cares.%.build sqlite.%.build cppunit.%.build
	$(eval DEST := $$(basename $$@))
	$(eval ARCH := $$(subst .,,$$(suffix $$(DEST))))
	mkdir -p $(DEST)
	( cd $(DEST) && ../$(SRCDIR)/configure \
		--prefix=$(ARIA2_PREFIX) \
		--bindir=$(ARIA2_PREFIX)/$(ARCH) \
		--sysconfdir=/etc \
		--with-cppunit-prefix=$(PWD)/$(ARCH) \
		$(ARIA2_CONFFLAGS) \
		CFLAGS="$(CFLAGS) -arch $(ARCH)" \
		CXXFLAGS="$(CXXFLAGS) -arch $(ARCH)" \
		PKG_CONFIG_PATH=$(PWD)/$(ARCH)/lib/pkgconfig \
		)
	$(MAKE) -C $(DEST) -sj$(CPUS) check
	$(MAKE) -C $(DEST) -sj$(CPUS) install-strip
	touch $@

aria2.build: aria2.x86_64.build aria2.i686.build
	mkdir -p $(ARIA2_PREFIX)/bin
	# Got two binaries now. Merge them into one universal binary and remove
	# the old ones.
	lipo \
		-arch x86_64 $(ARIA2_PREFIX)/x86_64/aria2c \
		-arch i686 $(ARIA2_PREFIX)/i686/aria2c \
		-create -output $(ARIA2_PREFIX)/bin/aria2c
	rm -rf $(ARIA2_PREFIX)/x86_64 $(ARIA2_PREFIX)/i686
	# Basic sanity check
	arch -64 $(ARIA2_PREFIX)/bin/aria2c -v
	arch -32 $(ARIA2_PREFIX)/bin/aria2c -v
	touch $@

$(ARIA2_CHANGELOG): aria2.build
	git log --pretty=fuller --date=short $(PREV_TAG)..HEAD > $@

$(ARIA2_DOCS): aria2.build
	cp -av $(SRCDIR)/$(@F) $@

$(ARIA2_DIST).tar.bz2: aria2.build $(ARIA2_DOCS) $(ARIA2_CHANGELOG)
	find $(ARIA2_PREFIX) -exec touch "{}" \;
	tar -cf $(ARIA2_DIST).tar.bz2 \
		--use-compress-program=bzip2 \
		--options='compression-level=9' \
		$(ARIA2)

$(ARIA2_DIST).pkg: aria2.build $(ARIA2_DOCS) $(ARIA2_CHANGELOG)
	find $(ARIA2_PREFIX) -exec touch "{}" \;
	pkgbuild \
		--root $(ARIA2) \
		--identifier aria2 \
		--version $(VERSION) \
		--install-location /usr/local/aria2 \
		--ownership recommended \
		out.pkg
	pkgbuild \
		--root $(SRCDIR)/doc/osx-package/etc \
		--identifier aria2.paths \
		--version $(VERSION) \
		--install-location /etc \
		--ownership recommended \
		paths.pkg
	echo "$$ARIA2_DISTXML" > dist.xml
	productbuild \
		--distribution dist.xml \
		--resources $(ARIA2_PREFIX)/share/doc/aria2 \
		$@
	rm -rf out.pkg paths.pkg dist.xml

$(ARIA2_DIST).dmg: $(ARIA2_DIST).pkg
	-rm -rf dmg
	mkdir -p dmg/Docs
	cp -av $(ARIA2_DIST).pkg dmg/aria2.pkg
	find $(ARIA2_PREFIX)/share/doc/aria2 -type f -depth 1 -exec cp -av "{}" dmg/Docs \;
	rm -rf dmg/Docs/README dmg/Docs/README.rst
	cp $(SRCDIR)/doc/osx-package/DS_Store dmg/.DS_Store
	hdiutil create $@.uncompressed \
		-srcfolder dmg \
		-volname "aria2 $(VERSION) Intel Universal" \
		-ov
	hdiutil convert -format UDBZ -o $@ $@.uncompressed.dmg
	hdiutil flatten $@
	rm -rf $@.uncompressed.dmg dmg

dist.build: $(ARIA2_DIST).tar.bz2 $(ARIA2_DIST).pkg $(ARIA2_DIST).dmg
	echo 'Build success: $(ARIA2_DIST)'
	touch $@

all:: dist.build

clean-dist:
	rm -rf $(ARIA2_DIST).tar.bz2 $(ARIA2_DIST).pkg $(ARIA2_DIST).dmg

clean: clean-dist
	rm -rf *aria2*

cleaner: clean
	rm -rf *.build *.check *.stamp $(ARCHLIBS) $(NONARCHLIBS) *x86_64* *i686*

really-clean: cleaner
	rm -rf *.tar.*


.PHONY: all clean-dist clean cleaner really-clean
