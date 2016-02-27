# Any copyright is dedicated to the Public Domain.
# http://creativecommons.org/publicdomain/zero/1.0/
# Written by Nils Maier

# This make file will:
#  - Download a set of dependencies and verify the known-good hashes.
#  - Build static libraries of aria2 dependencies.
#  - Create a statically linked, aria2 release.
#    - The build will have all major features enabled, and will use
#      AppleTLS and GMP.
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
# To make an universal build (x86_64, i686) use instead:
#  - $ make universal
#
# To make an both builds use instead:
#  - $ make multi
#
# If you haven't checkout out a release tag, you need to specify NON_RELEASE.
# $ export NON_RELEASE=1
# to generate a dist with git commit
# $ export NON_RELEASE=force
# to force this script to behave like it was on a tag.
#
# Note: This Makefile expects to be called from a git clone of aria2.
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
#
# Note: This Makefile uses resources from osx-package when creating the
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
CC = cc
export CC
CXX = c++
export CXX

# Set up compiler/linker flags.
OPTFLAGS ?= -Os
CFLAGS ?= -mmacosx-version-min=10.7 $(OPTFLAGS)
export CFLAGS
CXXFLAGS ?= -mmacosx-version-min=10.7 $(OPTFLAGS)
export CXXFLAGS
LDFLAGS ?= -Wl,-dead_strip
export LDFLAGS

LTO_FLAGS = -flto -ffunction-sections -fdata-sections

# Dependency versions
zlib_version = 1.2.8
zlib_hash = a4d316c404ff54ca545ea71a27af7dbc29817088
zlib_url = http://zlib.net/zlib-$(zlib_version).tar.gz

expat_version = 2.1.0
expat_hash = b08197d146930a5543a7b99e871cba3da614f6f0
expat_url = http://sourceforge.net/projects/expat/files/expat/$(expat_version)/expat-$(expat_version).tar.gz
expat_cflags=$(LTO_FLAGS)
expat_ldflags=$(CFLAGS) $(LTO_FLAGS)

cares_version = 1.10.0
cares_hash = e44e6575d5af99cb3a38461486e1ee8b49810eb5
cares_url = http://c-ares.haxx.se/download/c-ares-$(cares_version).tar.gz
cares_confflags = "--enable-optimize=$(OPTFLAGS)"
cares_cflags=$(LTO_FLAGS)
cares_ldflags=$(CFLAGS) $(LTO_FLAGS)

sqlite_version = autoconf-3081002
sqlite_hash = c2f2c17d3dc4c4e179d35cc04e4420636d48a152
sqlite_url = http://sqlite.org/2015/sqlite-$(sqlite_version).tar.gz
sqlite_cflags=$(LTO_FLAGS)
sqlite_ldflags=$(CFLAGS) $(LTO_FLAGS)

gmp_version = 5.1.3
gmp_hash = b35928e2927b272711fdfbf71b7cfd5f86a6b165
gmp_url = https://ftp.gnu.org/gnu/gmp/gmp-$(gmp_version).tar.bz2
gmp_confflags = --disable-cxx --enable-assembly --with-pic
gmp_confflags_x86_64 = --enable-fat

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

universal all::
	@if test "x$(NON_RELEASE)" = "x" && !(git describe --tags --exact); then \
		echo 'Not on a release tag; override by defining NON_RELEASE!'; \
		exit 1; \
	fi

# No dice without sphinx
universal all::
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
$(1).build: $(1).x86_64.build

$(1).universal.build: $(1).x86_64.build $(1).i686.build

deps:: $(1).build

deps.universal:: $(1).universal.build
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
	$(MAKE) -C $(DEST) -sj$(CPUS) CFLAGS="$(CFLAGS) $(LTO_FLAGS) -arch $(ARCH)"
	$(MAKE) -C $(DEST) -sj$(CPUS) CFLAGS="$(CFLAGS) $(LTO_FLAGS) -arch $(ARCH)" check
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
		$$($(1)_confflags) $$($(1)_confflags_$$(ARCH)) \
		CFLAGS="$$(CFLAGS) $$($(1)_cflags) -arch $$(ARCH)" \
		CXXFLAGS="$$(CXXFLAGS) $$($(1)_cxxflags) -arch $$(ARCH) -stdlib=libc++ -std=c++11" \
		LDFLAGS="$(LDFLAGS) $$($(1)_ldflags)" \
		)
	$$(MAKE) -C $$(DEST) -sj$(CPUS)
	$$(MAKE) -C $$(DEST) -sj$(CPUS) check
	$$(MAKE) -C $$(DEST) -s install
	touch $$@

$(1).build: $(1).x86_64.build

$(1).universal.build: $(1).x86_64.build $(1).i686.build

deps:: $(1).build

deps.universal:: $(1).universal.build
endef

$(foreach lib,$(ARCHLIBS),$(eval $(call ARCH_template,$(lib))))

.PRECIOUS: aria2.%.build
aria2.%.build: zlib.%.build expat.%.build gmp.%.build cares.%.build sqlite.%.build cppunit.%.build
	$(eval DEST := $$(basename $$@))
	$(eval ARCH := $$(subst .,,$$(suffix $$(DEST))))
	mkdir -p $(DEST)
	( cd $(DEST) && ../$(SRCDIR)/configure \
		--prefix=$(ARIA2_PREFIX) \
		--bindir=$(PWD)/$(DEST) \
		--sysconfdir=/etc \
		--with-cppunit-prefix=$(PWD)/$(ARCH) \
		$(ARIA2_CONFFLAGS) \
		CFLAGS="$(CFLAGS) $(LTO_FLAGS) -arch $(ARCH) -I$(PWD)/$(ARCH)/include" \
		CXXFLAGS="$(CXXFLAGS) $(LTO_FLAGS) -arch $(ARCH) -I$(PWD)/$(ARCH)/include" \
		LDFLAGS="$(LDFLAGS) $(CXXFLAGS) $(LTO_FLAGS) -L$(PWD)/$(ARCH)/lib" \
		PKG_CONFIG_PATH=$(PWD)/$(ARCH)/lib/pkgconfig \
		)
	$(MAKE) -C $(DEST) -sj$(CPUS)
	$(MAKE) -C $(DEST) -sj$(CPUS) check
	# Check that the resulting executable is Position-independent (PIE)
	otool -hv $(DEST)/src/aria2c | grep -q PIE
	$(MAKE) -C $(DEST) -sj$(CPUS) install-strip
	touch $@

aria2.build: aria2.x86_64.build
	mkdir -p $(ARIA2_PREFIX)/bin
	cp -f aria2.x86_64/aria2c $(ARIA2_PREFIX)/bin/aria2c
	arch -64 $(ARIA2_PREFIX)/bin/aria2c -v
	touch $@

aria2.universal.build: aria2.x86_64.build aria2.i686.build
	mkdir -p $(ARIA2_PREFIX)/bin
	# Got two binaries now. Merge them into one universal binary and remove
	# the old ones.
	lipo \
		-arch x86_64 aria2.x86_64/aria2c \
		-arch i686 aria2.i686/aria2c \
		-create -output $(ARIA2_PREFIX)/bin/aria2c
	# Basic sanity check
	arch -64 $(ARIA2_PREFIX)/bin/aria2c -v
	arch -32 $(ARIA2_PREFIX)/bin/aria2c -v
	touch $@

$(ARIA2_CHANGELOG): aria2.x86_64.build
	git log --pretty=fuller --date=short $(PREV_TAG)..HEAD > $@

$(ARIA2_DOCS): aria2.x86_64.build
	cp -av $(SRCDIR)/$(@F) $@

$(ARIA2_DIST).tar.bz2: aria2.build $(ARIA2_DOCS) $(ARIA2_CHANGELOG)
	find $(ARIA2_PREFIX) -exec touch "{}" \;
	tar -cf $@ \
		--use-compress-program=bzip2 \
		--options='compression-level=9' \
		$(ARIA2)

$(ARIA2_DIST).universal.tar.bz2: aria2.universal.build $(ARIA2_DOCS) $(ARIA2_CHANGELOG)
	find $(ARIA2_PREFIX) -exec touch "{}" \;
	tar -cf $@ \
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
		--root $(SRCDIR)/osx-package/etc \
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

$(ARIA2_DIST).universal.pkg: aria2.universal.build $(ARIA2_DOCS) $(ARIA2_CHANGELOG)
	find $(ARIA2_PREFIX) -exec touch "{}" \;
	pkgbuild \
		--root $(ARIA2) \
		--identifier aria2 \
		--version $(VERSION) \
		--install-location /usr/local/aria2 \
		--ownership recommended \
		out.pkg
	pkgbuild \
		--root $(SRCDIR)/osx-package/etc \
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
	cp $(SRCDIR)/osx-package/DS_Store dmg/.DS_Store
	hdiutil create $@.uncompressed \
		-srcfolder dmg \
		-volname "aria2 $(VERSION) Intel" \
		-ov
	hdiutil convert -format UDBZ -o $@ $@.uncompressed.dmg
	hdiutil flatten $@
	rm -rf $@.uncompressed.dmg dmg

$(ARIA2_DIST).universal.dmg: $(ARIA2_DIST).universal.pkg
	-rm -rf dmg
	mkdir -p dmg/Docs
	cp -av $(ARIA2_DIST).universal.pkg dmg/aria2.pkg
	find $(ARIA2_PREFIX)/share/doc/aria2 -type f -depth 1 -exec cp -av "{}" dmg/Docs \;
	rm -rf dmg/Docs/README dmg/Docs/README.rst
	cp $(SRCDIR)/osx-package/DS_Store dmg/.DS_Store
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

dist.universal.build: $(ARIA2_DIST).universal.tar.bz2 $(ARIA2_DIST).universal.pkg $(ARIA2_DIST).universal.dmg
	echo 'Build success: $(ARIA2_DIST)'
	touch $@

all:: dist.build

universal:: dist.universal.build

multi: all universal

clean-dist:
	rm -rf $(ARIA2_DIST).tar.bz2 $(ARIA2_DIST).pkg $(ARIA2_DIST).dmg

clean: clean-dist
	rm -rf *aria2*

cleaner: clean
	rm -rf *.build *.check *.stamp $(ARCHLIBS) $(NONARCHLIBS) *x86_64* *i686*

really-clean: cleaner
	rm -rf *.tar.*


.PHONY: all universal multi clean-dist clean cleaner really-clean
