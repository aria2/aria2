#!/bin/sh
# Native macOS build with a pinned, verified libcurl-impersonate dependency.
set -eu

source_dir=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
dependency_dir="$source_dir/.deps/curl-impersonate"
build_dir="$source_dir/.build/chrome"
prefix_dir="$source_dir/dist"
trap 'result=$?; if [ "$result" -ne 0 ]; then printf "%s\n" "Build failed; see $build_dir/*.log" >&2; fi' 0
jobs=4
if [ "$#" -gt 0 ]; then jobs=$1; fi
case "$jobs" in *[!0-9]*|"") printf '%s\n' 'Expected a numeric job count.' >&2; exit 1;; esac

if [ "$(uname -s)" != Darwin ]; then
  printf '%s\n' 'This helper builds on macOS; see doc/impersonation.rst for POSIX builds.' >&2
  exit 1
fi
for tool in autoreconf make pkg-config curl shasum python3; do
  if ! command -v "$tool" >/dev/null 2>&1; then
    printf 'Missing build tool: %s\n' "$tool" >&2
    printf '%s\n' 'Install: brew install autoconf automake libtool pkgconf gettext cppunit' >&2
    exit 1
  fi
done

case "$(uname -m)" in
  arm64)
    platform=arm64-macos
    checksum=6c0dc841a8f6b7a8f81a0f55197e8241405fb7dcd97df180e66fc6f796c42bd5
    ;;
  x86_64)
    platform=x86_64-macos
    checksum=eed3342d8768065ab7a9106f4286d9101959b7e9e1b2b969eb8d8be1a52422cd
    ;;
  *) printf '%s\n' 'Unsupported macOS architecture.' >&2; exit 1 ;;
esac

version=v2.3.2
archive="$dependency_dir/libcurl-impersonate-$version.$platform.tar.gz"
mkdir -p "$dependency_dir" "$build_dir"
if [ ! -f "$archive" ]; then
  curl --fail --location --retry 3 --output "$archive" \
    "https://github.com/lexiforest/curl-impersonate/releases/download/$version/libcurl-impersonate-$version.$platform.tar.gz"
fi
printf '%s  %s\n' "$checksum" "$archive" | shasum -a 256 --check
tar -xzf "$archive" -C "$dependency_dir"

cd "$source_dir"
autoreconf -fi > "$build_dir/autoreconf.log" 2>&1
cd "$build_dir"
CURL_IMPERSONATE_CFLAGS="-I$dependency_dir/include" \
CURL_IMPERSONATE_LIBS="-L$dependency_dir -Wl,-rpath,$dependency_dir -lcurl-impersonate" \
CXXFLAGS="-O2 -g" \
"$source_dir/configure" \
  --prefix="$prefix_dir" \
  --with-curl-impersonate \
  --with-ca-bundle=/etc/ssl/cert.pem \
  --without-gnutls --without-openssl --without-libssh2 --without-libxml2 \
  --disable-nls > configure.log 2>&1
make -j "$jobs" > build.log 2>&1
make install > install.log 2>&1

# Bundle non-system dylibs and remove the build-directory dependency.
python3 - "$source_dir" "$dependency_dir" "$prefix_dir" <<'PY'
from pathlib import Path
import shutil
import subprocess
import sys

source, dependency, prefix = map(Path, sys.argv[1:])
binary = prefix / "bin/aria2c"
libdir = prefix / "lib"
licenses = prefix / "share/licenses"
libdir.mkdir(parents=True, exist_ok=True)
curl = libdir / "libcurl-impersonate.4.dylib"
shutil.copy2((dependency / curl.name).resolve(), curl)
queue = [binary, curl]
seen = set()

def dependencies(path):
    output = subprocess.check_output(["otool", "-L", str(path)], text=True)
    return [line.strip().split(" (compatibility", 1)[0]
            for line in output.splitlines()[1:]]

while queue:
    path = queue.pop(0)
    if path in seen:
        continue
    seen.add(path)
    for name in dependencies(path):
        leaf = Path(name).name
        if name.startswith(("/usr/lib/", "/System/Library/")):
            continue
        if path != binary and leaf == path.name:
            continue  # A dylib's own install name.
        destination = libdir / leaf
        if name.startswith(("/opt/homebrew/", "/usr/local/")):
            original = Path(name).resolve()
            if destination not in seen:
                shutil.copy2(original, destination)
            queue.append(destination)
            if leaf.startswith("libcares"):
                for filename in ("LICENSE.md", "LICENSE", "COPYING"):
                    license_file = original.parent.parent / filename
                    if license_file.is_file():
                        target = licenses / "c-ares"
                        target.mkdir(parents=True, exist_ok=True)
                        shutil.copy2(license_file, target / filename)
        elif leaf != curl.name:
            raise RuntimeError("Unresolved bundle dependency: " + name)
        replacement = ("@executable_path/../lib/" if path == binary
                       else "@loader_path/") + leaf
        subprocess.run(["install_name_tool", "-change", name, replacement, str(path)],
                       check=True)

subprocess.run(["install_name_tool", "-delete_rpath", str(dependency), str(binary)],
               check=True)
for path in seen:
    subprocess.run(["codesign", "--force", "--sign", "-", str(path)],
                   check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)

curl_licenses = licenses / "curl-impersonate"
curl_licenses.mkdir(parents=True, exist_ok=True)
for item in dependency.glob("LICENSE*"):
    shutil.copy2(item, curl_licenses / item.name)
aria_license = licenses / "aria2"
aria_license.mkdir(parents=True, exist_ok=True)
shutil.copy2(source / "COPYING", aria_license / "COPYING")
shutil.copy2(source / "doc/impersonation.rst", prefix / "IMPERSONATION.rst")
print("Built:", binary)
PY
"$prefix_dir/bin/aria2c" --version
