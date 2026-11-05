#!/bin/sh
# Native glibc Linux build with a pinned libcurl-impersonate dependency.
set -eu

source_dir=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
jobs=${1:-4}
case "$jobs" in
  *[!0-9]*|"") printf '%s\n' 'Expected a positive numeric job count.' >&2; exit 1 ;;
esac
if [ "$#" -gt 1 ] || [ "$jobs" -eq 0 ]; then
  printf 'Usage: sh %s [jobs > 0]\n' "$0" >&2
  exit 1
fi
if [ "$(uname -s)" != Linux ] || ! getconf GNU_LIBC_VERSION >/dev/null 2>&1; then
  printf '%s\n' 'This helper requires glibc Linux; see doc/impersonation.rst for manual builds.' >&2
  exit 1
fi
case "$(uname -m)" in
  x86_64)
    platform=x86_64-linux-gnu
    checksum=7716be1ec87f13ecc776c211003bd1122c345287829b1f18a494a5467c90ab1e
    ;;
  aarch64)
    platform=aarch64-linux-gnu
    checksum=1cb74a3b5103df020c252f9efef5a779c87380ee6244b365a90a0e4f7086864e
    ;;
  *) printf '%s\n' 'This helper supports x86_64 and aarch64 Linux.' >&2; exit 1 ;;
esac
for tool in autoreconf autopoint make pkg-config curl sha256sum tar patchelf readelf; do
  if ! command -v "$tool" >/dev/null 2>&1; then
    printf 'Missing build tool: %s (see doc/impersonation.rst)\n' "$tool" >&2
    exit 1
  fi
done
pkg-config --exists openssl expat zlib || {
  printf '%s\n' 'Missing development libraries; see the Linux prerequisites in doc/impersonation.rst.' >&2
  exit 1
}

ca_bundle=${CA_BUNDLE:-}
if [ -z "$ca_bundle" ]; then
  for candidate in /etc/ssl/certs/ca-certificates.crt /etc/pki/tls/certs/ca-bundle.crt /etc/ssl/ca-bundle.pem; do
    if [ -r "$candidate" ]; then ca_bundle=$candidate; break; fi
  done
fi
if [ -z "$ca_bundle" ] || [ ! -r "$ca_bundle" ]; then
  printf '%s\n' 'Install ca-certificates or set CA_BUNDLE to the system CA bundle.' >&2
  exit 1
fi

version=v2.3.2
dependency_dir="$source_dir/.deps/curl-impersonate/$version/$platform"
build_dir="$source_dir/.build/chrome-$platform"
prefix_dir="$source_dir/dist/linux-$platform"
archive="$dependency_dir/libcurl-impersonate-$version.$platform.tar.gz"
log_file=
trap 'result=$?; if [ "$result" -ne 0 ] && [ -n "$log_file" ]; then printf "Build failed; see %s\n" "$log_file" >&2; tail -n 40 "$log_file" >&2; fi' 0
mkdir -p "$dependency_dir" "$build_dir"
if [ ! -f "$archive" ]; then
  curl --fail --location --retry 3 --output "$archive.part" \
    "https://github.com/lexiforest/curl-impersonate/releases/download/$version/libcurl-impersonate-$version.$platform.tar.gz"
  printf '%s  %s\n' "$checksum" "$archive.part" | sha256sum --check
  mv "$archive.part" "$archive"
fi
printf '%s  %s\n' "$checksum" "$archive" | sha256sum --check
tar -xzf "$archive" -C "$dependency_dir"

cd "$source_dir"
printf '%s\n' 'Regenerating the build system...'
log_file="$build_dir/autoreconf.log"
autoreconf -fi > "$log_file" 2>&1
cd "$build_dir"
printf '%s\n' 'Configuring Chrome impersonation and the normal OpenSSL transport...'
log_file="$build_dir/configure.log"
CURL_IMPERSONATE_CFLAGS="-I$dependency_dir/include" \
CURL_IMPERSONATE_LIBS="-L$dependency_dir -Wl,-rpath,$dependency_dir -lcurl-impersonate" \
CXXFLAGS="${CXXFLAGS:--O2 -g}" \
"$source_dir/configure" \
  --prefix="$prefix_dir" \
  --with-curl-impersonate --with-ca-bundle="$ca_bundle" \
  --with-openssl --without-gnutls --without-libnettle --without-libgmp \
  --without-libgcrypt --without-sqlite3 --without-libcares \
  --without-libssh2 --without-libxml2 \
  --disable-nls > "$log_file" 2>&1
printf 'Compiling with %s jobs...\n' "$jobs"
log_file="$build_dir/build.log"
make -j "$jobs" > "$log_file" 2>&1
printf '%s\n' 'Installing the local bundle...'
log_file="$build_dir/install.log"
make install > "$log_file" 2>&1
log_file=

# Keep the TLS libraries with the binary. The C/C++ runtime, zlib and Expat
# still come from the target distribution: build on the oldest target system.
mkdir -p "$prefix_dir/bin" "$prefix_dir/share/licenses/curl-impersonate" \
  "$prefix_dir/share/licenses/aria2"
cp -L "$dependency_dir/libcurl-impersonate.so.4" "$prefix_dir/bin/"
patchelf --set-rpath '$ORIGIN' "$prefix_dir/bin/libcurl-impersonate.so.4"
# Remove the old layout's copy when updating an existing local bundle.
rm -f "$prefix_dir/lib/libcurl-impersonate.so.4"
openssl_libdir=$(pkg-config --variable=libdir openssl)
for library in libssl.so libcrypto.so; do
  library_path="$openssl_libdir/$library"
  soname=$(readelf -d "$library_path" | sed -n 's/.*(SONAME).*\[\(.*\)\].*/\1/p')
  if [ -z "$soname" ]; then
    printf 'Cannot determine the SONAME of %s\n' "$library_path" >&2
    exit 1
  fi
  cp -L "$library_path" "$prefix_dir/bin/$soname"
  patchelf --set-rpath '$ORIGIN' "$prefix_dir/bin/$soname"
  rm -f "$prefix_dir/lib/$soname"
done
rmdir "$prefix_dir/lib" 2>/dev/null || :
patchelf --set-rpath '$ORIGIN' "$prefix_dir/bin/aria2c"
cp "$dependency_dir"/LICENSE* "$prefix_dir/share/licenses/curl-impersonate/"
cp "$source_dir/COPYING" "$prefix_dir/share/licenses/aria2/"
cp "$source_dir/LICENSE.OpenSSL" "$prefix_dir/share/licenses/aria2/"
openssl_license=${OPENSSL_LICENSE:-/usr/share/doc/libssl-dev/copyright}
if [ ! -r "$openssl_license" ]; then
  printf '%s\n' 'Set OPENSSL_LICENSE to the copyright/license file for the bundled OpenSSL installation.' >&2
  exit 1
fi
mkdir -p "$prefix_dir/share/licenses/openssl"
cp "$openssl_license" "$prefix_dir/share/licenses/openssl/COPYRIGHT"
cp "$source_dir/doc/impersonation.rst" "$prefix_dir/IMPERSONATION.rst"
printf 'Built: %s\n' "$prefix_dir/bin/aria2c"
"$prefix_dir/bin/aria2c" --version
printf '\nC++ tests: make -C "%s" check\n' "$build_dir"
printf 'Integration tests: python3 "%s/test/impersonation_integration_test.py" "%s/bin/aria2c"\n' "$source_dir" "$prefix_dir"
