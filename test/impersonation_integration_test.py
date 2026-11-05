#!/usr/bin/env python3
"""Exercise the real aria2 scheduler through the impersonating transport.

Usage: python3 test/impersonation_integration_test.py /path/to/aria2c
Uses only localhost fixtures and Python's standard library; openssl is needed
to generate a temporary TLS certificate. No system trust settings are changed.
"""
import base64
import gzip
import http.server
import os
from pathlib import Path
import select
import socket
import socketserver
import ssl
import subprocess
import sys
import tempfile
import threading
import time
import unittest

BINARY = str(Path(sys.argv.pop(1)).resolve())
DATA = bytes(range(256)) * (8 * 1024 * 1024 // 256)
SMALL = DATA[:32123]


class Handler(http.server.BaseHTTPRequestHandler):
    protocol_version = "HTTP/1.1"
    records = []
    lock = threading.Lock()

    def log_message(self, *args):
        pass

    def record(self):
        with self.lock:
            self.records.append((self.command, self.path, dict(self.headers)))

    def do_CONNECT(self):
        self.record()
        host, port = self.path.rsplit(":", 1)
        with socket.create_connection((host, int(port)), timeout=5) as remote:
            self.send_response(200)
            self.end_headers()
            while True:
                ready, _, _ = select.select([self.connection, remote], [], [], 5)
                if not ready:
                    return
                for source in ready:
                    data = source.recv(65536)
                    if not data:
                        return
                    target = remote if source is self.connection else self.connection
                    target.sendall(data)

    def do_HEAD(self):
        self.do_GET()

    def do_GET(self):
        self.record()
        try:
            self.respond()
        except (BrokenPipeError, ConnectionResetError, ssl.SSLError):
            pass  # aria2 cancels metadata transfers and superseded ranges.

    def headers_for(self, status, length=None, **headers):
        self.send_response(status)
        if length is not None:
            self.send_header("Content-Length", str(length))
        for key, value in headers.items():
            self.send_header(key.replace("_", "-"), value)
        self.end_headers()

    def respond(self):
        if self.path == "/redirect":
            self.headers_for(302, 0, Location="/small", Set_Cookie="fixture=yes; Path=/")
            return
        if self.path == "/auth":
            expected = "Basic " + base64.b64encode(b"fixture:fixture").decode()
            if self.headers.get("Authorization") != expected:
                self.headers_for(401, 0, WWW_Authenticate='Basic realm="fixture"')
                return
        if self.path == "/chunked":
            self.headers_for(200, Transfer_Encoding="chunked")
            for offset in range(0, len(SMALL), 997):
                chunk = SMALL[offset:offset + 997]
                self.wfile.write(("%x\r\n" % len(chunk)).encode() + chunk + b"\r\n")
            self.wfile.write(b"0\r\nX-Fixture-Trailer: ignored\r\n\r\n")
            return
        if self.path == "/close":
            self.headers_for(200, Connection="close")
            self.wfile.write(SMALL)
            self.close_connection = True
            return
        if self.path == "/gzip":
            body = gzip.compress(SMALL)
            self.headers_for(200, len(body), Content_Encoding="gzip")
            self.wfile.write(body)
            return
        if self.path == "/truncated":
            self.headers_for(200, len(DATA), Connection="close")
            self.wfile.write(DATA[:1024])
            self.close_connection = True
            return
        if self.path == "/stall":
            self.headers_for(200, len(DATA))
            self.wfile.flush()
            time.sleep(10)
            return
        data = DATA if self.path == "/file" else SMALL
        if self.path == "/empty":
            data = b""
        first, last, status = 0, len(data) - 1, 200
        extra = {"Accept_Ranges": "bytes"}
        if self.headers.get("Range"):
            span = self.headers["Range"].split("=", 1)[1].split("-")
            first = int(span[0])
            last = min(int(span[1]) if span[1] else last, last)
            status = 206
            extra["Content_Range"] = "bytes {}-{}/{}".format(first, last, len(data))
        self.headers_for(status, last - first + 1, **extra)
        if self.command == "HEAD":
            return
        for offset in range(first, last + 1, 16384):
            self.wfile.write(data[offset:min(offset + 16384, last + 1)])
            if self.path == "/file":
                time.sleep(0.001)


class Server(socketserver.ThreadingMixIn, http.server.HTTPServer):
    daemon_threads = True

    def handle_error(self, request, client_address):
        pass  # Connection cancellation is expected in these tests.


class Integration(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.root = tempfile.TemporaryDirectory(prefix="aria2-impersonation-test-")
        cls.directory = Path(cls.root.name)
        cls.cert = cls.directory / "cert.pem"
        key = cls.directory / "key.pem"
        config = cls.directory / "openssl.cnf"
        config.write_text(
            "[req]\nprompt=no\ndistinguished_name=dn\nx509_extensions=ext\n"
            "[dn]\nCN=localhost\n[ext]\nsubjectAltName=DNS:localhost\n"
            "basicConstraints=critical,CA:TRUE\n"
            "keyUsage=critical,digitalSignature,keyEncipherment,keyCertSign\n"
        )
        subprocess.run(
            ["openssl", "req", "-x509", "-newkey", "rsa:2048", "-nodes",
             "-days", "1", "-keyout", str(key), "-out", str(cls.cert),
             "-config", str(config)],
            check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL,
        )
        cls.http = Server(("127.0.0.1", 0), Handler)
        cls.https = Server(("127.0.0.1", 0), Handler)
        if hasattr(ssl, "TLSVersion"):
            context = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
            context.maximum_version = ssl.TLSVersion.TLSv1_2
        else:
            # Python 3.5 / OpenSSL 1.1.0 on Debian 9.
            context = ssl.SSLContext(ssl.PROTOCOL_TLSv1_2)
        context.load_cert_chain(str(cls.cert), str(key))
        context.set_alpn_protocols(["http/1.1"])
        cls.https.socket = context.wrap_socket(cls.https.socket, server_side=True)
        cls.proxy = Server(("127.0.0.1", 0), Handler)
        for server in (cls.http, cls.https, cls.proxy):
            threading.Thread(target=server.serve_forever, daemon=True).start()
        cls.base = "http://127.0.0.1:{}".format(cls.http.server_port)
        cls.tls_base = "https://localhost:{}".format(cls.https.server_port)

    @classmethod
    def tearDownClass(cls):
        for server in (cls.http, cls.https, cls.proxy):
            server.shutdown()
            server.server_close()
        cls.root.cleanup()

    def setUp(self):
        self.target = self.directory / (self._testMethodName + ".bin")
        with Handler.lock:
            Handler.records.clear()

    def command(self, path, *options):
        url = path if "://" in path else self.base + path
        return [
            BINARY, "--no-conf=true", "--impersonate=chrome",
            "--console-log-level=error", "--download-result=hide",
            "--summary-interval=0", "--enable-color=false",
            "--file-allocation=none", "--max-tries=1",
            "--connect-timeout=5", "--timeout=5",
            "--auto-file-renaming=false", "--allow-overwrite=true",
            "--dir=" + str(self.directory), "--out=" + self.target.name,
            *options, url,
        ]

    def environment(self):
        return {k: v for k, v in os.environ.items()
                if k.lower() not in ("http_proxy", "https_proxy", "all_proxy", "no_proxy")}

    def download(self, path, *options, success=True, expected=SMALL):
        result = subprocess.run(self.command(path, *options),
                                env=self.environment(), stdout=subprocess.PIPE,
                                stderr=subprocess.PIPE, universal_newlines=True,
                                timeout=30)
        if success:
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            if expected is not None:
                self.assertEqual(self.target.read_bytes(), expected)
        else:
            self.assertNotEqual(result.returncode, 0, result.stdout + result.stderr)
        return result

    def test_default_transport(self):
        self.download("/small", "--impersonate=none")

    def test_profile_and_headers(self):
        self.download("/small", "--referer=https://example.org/",
                      "--header=X-Fixture: present")
        headers = Handler.records[-1][2]
        self.assertIn("Chrome/136.", headers["User-Agent"])
        self.assertEqual(headers["Referer"], "https://example.org/")
        self.assertEqual(headers["X-Fixture"], "present")
        self.assertEqual(headers["Accept-Encoding"], "identity")

    def test_custom_user_agent(self):
        self.download("/small", "--user-agent=FixtureClient/1")
        self.assertEqual(Handler.records[-1][2]["User-Agent"], "FixtureClient/1")

    def test_segmented_download(self):
        self.download("/file", "--split=4", "--max-connection-per-server=4",
                      "--min-split-size=1M", expected=DATA)
        ranges = [h["Range"] for _, _, h in Handler.records if "Range" in h]
        self.assertGreaterEqual(len(ranges), 2, Handler.records)

    def test_resume(self):
        self.target.write_bytes(DATA[:1024 * 1024])
        self.download("/file", "--continue=true", "--split=1", expected=DATA)
        self.assertTrue(any(h.get("Range", "").startswith("bytes=1048576-")
                            for _, _, h in Handler.records), Handler.records)

    def test_redirect_and_cookies(self):
        self.download("/redirect")
        self.assertIn("fixture=yes", Handler.records[-1][2].get("Cookie", ""))

    def test_authentication_challenge(self):
        self.download("/auth", "--http-user=fixture", "--http-passwd=fixture",
                      "--http-auth-challenge=true")
        self.assertIn("Authorization", Handler.records[-1][2])

    def test_chunked_with_trailers(self):
        self.download("/chunked")

    def test_empty_file(self):
        self.download("/empty", expected=b"")

    def test_connection_close_framing(self):
        self.download("/close")

    def test_gzip(self):
        self.download("/gzip", "--http-accept-gzip=true")

    def test_head(self):
        self.download("/small", "--dry-run=true", expected=None)
        self.assertEqual(Handler.records[-1][0], "HEAD")

    def test_truncation_is_error(self):
        self.download("/truncated", success=False)

    def test_trusted_tls(self):
        self.download(self.tls_base + "/small", "--ca-certificate=" + str(self.cert))

    def test_minimum_tls_version_enforced(self):
        self.download(self.tls_base + "/small", "--ca-certificate=" + str(self.cert),
                      "--min-tls-version=TLSv1.3", success=False)

    def test_untrusted_tls_is_error(self):
        self.download(self.tls_base + "/small", success=False)

    def test_wrong_hostname_is_error(self):
        self.download(self.tls_base.replace("localhost", "127.0.0.1") + "/small",
                      "--ca-certificate=" + str(self.cert), success=False)

    def test_connect_proxy(self):
        self.download(
            self.tls_base + "/small", "--ca-certificate=" + str(self.cert),
            "--all-proxy=http://127.0.0.1:{}".format(self.proxy.server_port),
            "--all-proxy-user=fixture", "--all-proxy-passwd=fixture",
        )
        connect = next(h for method, _, h in Handler.records if method == "CONNECT")
        expected = "Basic " + base64.b64encode(b"fixture:fixture").decode()
        self.assertEqual(connect.get("Proxy-Authorization"), expected)
        origin = next(h for method, path, h in Handler.records if path == "/small")
        self.assertNotIn("Proxy-Authorization", origin)

    def test_no_proxy(self):
        self.download(
            self.tls_base + "/small", "--ca-certificate=" + str(self.cert),
            "--all-proxy=http://127.0.0.1:1", "--no-proxy=localhost",
        )
        self.assertFalse(any(method == "CONNECT" for method, _, _ in Handler.records))

    def test_unknown_profile_is_error(self):
        result = self.download("/small", "--impersonate=chrome99999", success=False)
        self.assertIn("Unsupported Chrome profile", result.stdout + result.stderr)

    def test_cancellation_does_not_wait_for_worker(self):
        process = subprocess.Popen(self.command("/stall"), env=self.environment(),
                                   stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        try:
            deadline = time.monotonic() + 5
            while time.monotonic() < deadline:
                if any(path == "/stall" for _, path, _ in Handler.records):
                    break
                time.sleep(0.02)
            process.terminate()
            process.wait(timeout=3)
        finally:
            if process.poll() is None:
                process.kill()
                process.wait()


if __name__ == "__main__":
    unittest.main(verbosity=2)
