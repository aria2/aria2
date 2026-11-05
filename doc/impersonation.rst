Chrome impersonation
====================

This optional transport sends HTTP(S) requests through libcurl-impersonate,
including its Chrome TLS ClientHello, HTTP/2 settings, pseudo-header ordering,
and browser headers. It is linked into aria2; no Python runtime, external proxy,
TLS interception, or additional trusted CA is involved.

Usage
-----

Build with the feature enabled, then run::

    aria2c --impersonate=chrome --referer=https://example.org/ \
        -x4 -s4 https://example.org/file.bin

The default is **--impersonate=none**, which selects the original aria2
transport. **chrome** is a reproducible alias for **chrome136**, rather than
silently changing with the installed library. Other explicit Chrome profile
names supported by the linked library may be supplied. Unsupported profiles
fail with a diagnostic.

The option also works in aria2.conf, input-file options, and initial/reserved
RPC download options. The library's Chrome User-Agent replaces the default
aria2 User-Agent; an explicit custom User-Agent is preserved. Referer, Origin,
authorization, cookies, and custom headers still come from aria2/the caller.
Impersonation does not run JavaScript or solve browser challenges.

Building on macOS
-----------------

Install the build tools::

    brew install autoconf automake libtool pkgconf gettext cppunit

Run the helper from the repository::

    sh contrib/build-macos-impersonate.sh

It downloads the official libcurl-impersonate v2.3.2 macOS archive, verifies
its pinned SHA-256, builds in .build/chrome, and installs in dist.
The helper supports native arm64 and x86_64 builds. The executable is
**dist/bin/aria2c**. Keep the accompanying **dist/lib** directory with it.
The bundle includes libcurl-impersonate and, when detected, c-ares.
It does not overwrite any system aria2 installation.

The resulting binary is built for the local macOS SDK; it is not a universal
binary or a promise of compatibility with older macOS releases.

Building on Linux
-----------------

The Linux helper supports native x86_64 and aarch64 systems using glibc.
On Debian/Ubuntu, install the prerequisites::

    sudo apt-get update
    sudo apt-get install build-essential autoconf automake libtool autopoint \
        gettext pkg-config curl ca-certificates patchelf python3 openssl \
        libssl-dev libexpat1-dev zlib1g-dev libcppunit-dev

Choose a build environment no newer than the oldest system that must run the
binary. For example, a Debian 9 target needs its glibc 2.24 and GCC 6 C++
runtime baseline. Building on Debian 12 does not produce a Debian 9-compatible
executable, even when both machines are x86_64. Matching the CPU architecture
alone is insufficient. Debian 9's packages are now in Debian's archive; use
``archive.debian.org`` repositories when preparing an isolated Debian 9 build
environment.

From this modified source tree, run::

    sh contrib/build-linux-impersonate.sh 4

The helper verifies the pinned SHA-256 of libcurl-impersonate v2.3.2 and uses
separate dependency, build, and install directories for each architecture.
On x86_64 the executable is::

    dist/linux-x86_64-linux-gnu/bin/aria2c --impersonate=chrome \
        https://example.org/file.bin

For aarch64, replace ``x86_64`` with ``aarch64`` in the paths. The executable
loads ``libcurl-impersonate.so.4`` and the bundled OpenSSL libraries from the
same directory as ``aria2c`` (``bin`` in the installed bundle). The executable
and all three bundled libraries use ``$ORIGIN`` as their library search path.
Keep ``aria2c`` and all three bundled shared libraries together when moving
them; no separate ``lib`` directory is needed.
The C/C++ runtime, zlib, and Expat come from the Linux distribution; this is
not a fully static binary. The helper uses OpenSSL for ordinary aria2 HTTPS
requests and libcurl-impersonate for requests with ``--impersonate`` enabled.
SQLite cookie-database import, c-ares asynchronous DNS, GnuTLS, nettle/GMP,
and SFTP are disabled. Normal HTTP cookies, BitTorrent, gzip, Metalink,
hashing, and XML-RPC remain enabled. This matches the base features of the
Debian 9 OpenSSL build without introducing a GnuTLS dependency chain.

The helper copies OpenSSL's copyright file from the Debian/Ubuntu development
package. On other distributions, set ``OPENSSL_LICENSE`` to the corresponding
license file for the OpenSSL installation being bundled.

Build logs are in ``.build/chrome-x86_64-linux-gnu``. Check the result with::

    make -C .build/chrome-x86_64-linux-gnu -j4 check
    python3 test/impersonation_integration_test.py \
        dist/linux-x86_64-linux-gnu/bin/aria2c

No root access is needed after installing the prerequisites, and the helper
does not replace a system aria2 installation. ``CA_BUNDLE`` can override the
detected system trust bundle. The helper expects glibc; Alpine/musl and other
architectures require a matching libcurl-impersonate build and the manual
configuration below.

Manual POSIX build
------------------

Install libcurl-impersonate (ordinary libcurl lacks the required API).
Then, after autoreconf -i, configure using its headers and library::

    CURL_IMPERSONATE_CFLAGS="-I/path/to/impersonate/include" \
    CURL_IMPERSONATE_LIBS="-L/path/to/impersonate -lcurl-impersonate" \
    ./configure --with-curl-impersonate \
        --with-ca-bundle=/path/to/system-ca-bundle.pem
    make -j4

Ensure the dynamic loader can find that library, for example using a build
rpath. The feature is disabled unless explicitly requested at configure time.
The implementation currently requires POSIX socketpair and C++ threads;
Windows builds are rejected when this feature is requested.

Transport behavior and limits
-----------------------------

Aria2 continues to generate requests and process responses, including ranges,
resume, redirects, authentication challenges, cookies, checksums, file naming,
rate limits, and download/RPC scheduling. Redirects and cookie jars are not
independently managed by libcurl, so existing origin/authentication boundaries
remain in aria2.

Each connection uses one worker and a private socketpair. The worker sends the
request using libcurl and translates its response into the HTTP/1.1 framing
understood by aria2. Bodies stream through bounded socket buffers, rather than
being buffered in memory in full. Cancellation closes the socket and signals
libcurl; workers hold no engine or download state.

This first implementation supports GET and HEAD. It uses one upstream
connection per request and disables aria2 HTTP keep-alive/pipelining for
impersonated requests. Segmented downloads still work; HTTP/2 multiplexing
across aria2 ranges and upstream connection pooling are not implemented.
Active transfers follow aria2's connection limits; cancelled workers finish
asynchronously without blocking the engine.

By default, Accept-Encoding is identity, making range offsets stable. With
--http-accept-gzip=true, aria2's gzip/deflate request and decoding are preserved.
Arbitrary brotli/zstd responses requested through custom headers are not
decoded by aria2. libcurl removes upstream transfer framing; the adapter
preserves Content-Length when present and otherwise supplies chunked framing.

Certificate and hostname verification remain enabled by default.
--ca-certificate, PEM --certificate/--private-key, --check-certificate,
--min-tls-version, --interface, --disable-ipv6, HTTP proxy credentials and
--no-proxy are forwarded/respected. With c-ares support in the linked libcurl,
--async-dns-server is forwarded too. Name resolution and transport-specific
tuning otherwise belong to libcurl; aria2's multi-interface selection,
resolver timing/cache settings, and TCP tuning are not reproduced.

Verification
------------

Run the existing C++ suite from the build directory::

    make check

Run the network integration fixtures from the repository::

    python3 test/impersonation_integration_test.py .build/chrome/src/aria2c

These tests support Python 3.5 and newer, including Debian 9's Python.
They use localhost HTTP/HTTPS and a CONNECT proxy. They cover the
normal transport, browser/custom headers, splitting, resume, authentication,
cookie redirects, gzip, chunked and close-delimited bodies, HEAD, empty and
truncated files, certificate trust/hostname validation, minimum TLS version,
proxying, unsupported profiles, and cancellation.
