/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2006 Tatsuhiro Tsujikawa
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 * In addition, as a special exception, the copyright holders give
 * permission to link the code of portions of this program with the
 * OpenSSL library under certain conditions as described in each
 * individual source file, and distribute linked combinations
 * including the two.
 * You must obey the GNU General Public License in all respects
 * for all of the code used other than OpenSSL.  If you modify
 * file(s) with this exception, you may extend this exception to your
 * version of the file(s), but you are not obligated to do so.  If you
 * do not wish to do so, delete this exception statement from your
 * version.  If you delete this exception statement from all source
 * files in the program, then also delete it here.
 */
/* copyright --> */
#include "prefs.h"

#include <cassert>
#include <vector>
#include <map>

namespace aria2 {

Pref::Pref(const char* k, size_t i):k(k), i(i) {}

namespace {

class PrefFactory {
public:
  PrefFactory():count_(0)
  {
    // We add special null pref whose ID is 0.
    makePref("");
  }
  size_t nextId()
  {
    return count_++;
  }
  Pref* makePref(const char* key)
  {
    size_t id = nextId();
    Pref* pref = new Pref(key, id);
    i2p_.push_back(pref);
    k2p_[key] = pref;
    return pref;
  }
  size_t getCount() const
  {
    return count_;
  }
  const Pref* i2p(size_t id) const
  {
    assert(id < count_);
    return i2p_[id];
  }
  const Pref* k2p(const std::string& k) const
  {
    std::map<std::string, const Pref*>::const_iterator i = k2p_.find(k);
    if(i == k2p_.end()) {
      return i2p_[0];
    } else {
      return (*i).second;
    }
  }
private:
  size_t count_;
  std::vector<const Pref*> i2p_;
  std::map<std::string, const Pref*> k2p_;
};

PrefFactory* getPrefFactory()
{
  static PrefFactory* pf = new PrefFactory();
  return pf;
}

Pref* makePref(const char* key)
{
  return getPrefFactory()->makePref(key);
}

} // namespace

namespace option {

size_t countOption()
{
  return getPrefFactory()->getCount();
}

const Pref* i2p(size_t id)
{
  return getPrefFactory()->i2p(id);
}

const Pref* k2p(const std::string& key)
{
  return getPrefFactory()->k2p(key);
}

} // namespace option

/**
 * Constants
 */
const std::string A2_V_TRUE("true");
const std::string A2_V_FALSE("false");
const std::string A2_V_DEFAULT("default");
const std::string V_NONE("none");
const std::string V_MEM("mem");
const std::string V_ALL("all");
const std::string A2_V_FULL("full");
const std::string A2_V_GEOM("geom");
const std::string V_PREALLOC("prealloc");
const std::string V_FALLOC("falloc");
const std::string V_TRUNC("trunc");
const std::string V_DEBUG("debug");
const std::string V_INFO("info");
const std::string V_NOTICE("notice");
const std::string V_WARN("warn");
const std::string V_ERROR("error");
const std::string V_INORDER("inorder");
const std::string V_FEEDBACK("feedback");
const std::string V_ADAPTIVE("adaptive");
const std::string V_EPOLL("epoll");
const std::string V_KQUEUE("kqueue");
const std::string V_PORT("port");
const std::string V_POLL("poll");
const std::string V_SELECT("select");
const std::string V_BINARY("binary");
const std::string V_ASCII("ascii");
const std::string V_GET("get");
const std::string V_TUNNEL("tunnel");
const std::string V_PLAIN("plain");
const std::string V_ARC4("arc4");
const std::string V_HTTP("http");
const std::string V_HTTPS("https");
const std::string V_FTP("ftp");

const Pref* PREF_VERSION = makePref("version");
const Pref* PREF_HELP = makePref("help");

/**
 * General preferences
 */
// values: 1*digit
const Pref* PREF_TIMEOUT = makePref("timeout");
// values: 1*digit
const Pref* PREF_DNS_TIMEOUT = makePref("dns-timeout");
// values: 1*digit
const Pref* PREF_CONNECT_TIMEOUT = makePref("connect-timeout");
// values: 1*digit
const Pref* PREF_MAX_TRIES = makePref("max-tries");
// values: 1*digit
const Pref* PREF_AUTO_SAVE_INTERVAL = makePref("auto-save-interval");
// values: a string that your file system recognizes as a file name.
const Pref* PREF_LOG = makePref("log");
// values: a string that your file system recognizes as a directory.
const Pref* PREF_DIR = makePref("dir");
// values: a string that your file system recognizes as a file name.
const Pref* PREF_OUT = makePref("out");
// values: 1*digit
const Pref* PREF_SPLIT = makePref("split");
// value: true | false
const Pref* PREF_DAEMON = makePref("daemon");
// value: a string
const Pref* PREF_REFERER = makePref("referer");
// value: 1*digit
const Pref* PREF_LOWEST_SPEED_LIMIT = makePref("lowest-speed-limit");
// value: 1*digit
const Pref* PREF_PIECE_LENGTH = makePref("piece-length");
// value: 1*digit
const Pref* PREF_MAX_OVERALL_DOWNLOAD_LIMIT = makePref("max-overall-download-limit");
// value: 1*digit
const Pref* PREF_MAX_DOWNLOAD_LIMIT = makePref("max-download-limit");
// value: 1*digit
const Pref* PREF_STARTUP_IDLE_TIME = makePref("startup-idle-time");
// value: prealloc | fallc | none
const Pref* PREF_FILE_ALLOCATION = makePref("file-allocation");
// value: 1*digit
const Pref* PREF_NO_FILE_ALLOCATION_LIMIT = makePref("no-file-allocation-limit");
// value: true | false
const Pref* PREF_ALLOW_OVERWRITE = makePref("allow-overwrite");
// value: true | false
const Pref* PREF_REALTIME_CHUNK_CHECKSUM = makePref("realtime-chunk-checksum");
// value: true | false
const Pref* PREF_CHECK_INTEGRITY = makePref("check-integrity");
// value: string that your file system recognizes as a file name.
const Pref* PREF_NETRC_PATH = makePref("netrc-path");
// value:
const Pref* PREF_CONTINUE = makePref("continue");
// value:
const Pref* PREF_NO_NETRC = makePref("no-netrc");
// value: 1*digit
const Pref* PREF_MAX_DOWNLOADS = makePref("max-downloads");
// value: string that your file system recognizes as a file name.
const Pref* PREF_INPUT_FILE = makePref("input-file");
// value: true | false
const Pref* PREF_DEFERRED_INPUT = makePref("deferred-input");
// value: 1*digit
const Pref* PREF_MAX_CONCURRENT_DOWNLOADS = makePref("max-concurrent-downloads");
// value: true | false
const Pref* PREF_FORCE_SEQUENTIAL = makePref("force-sequential");
// value: true | false
const Pref* PREF_AUTO_FILE_RENAMING = makePref("auto-file-renaming");
// value: true | false
const Pref* PREF_PARAMETERIZED_URI = makePref("parameterized-uri");
// value: true | false
const Pref* PREF_ENABLE_DIRECT_IO = makePref("enable-direct-io");
// value: true | false
const Pref* PREF_ALLOW_PIECE_LENGTH_CHANGE = makePref("allow-piece-length-change");
// value: true | false
const Pref* PREF_NO_CONF = makePref("no-conf");
// value: string
const Pref* PREF_CONF_PATH = makePref("conf-path");
// value: 1*digit
const Pref* PREF_STOP = makePref("stop");
// value: true | false
const Pref* PREF_QUIET = makePref("quiet");
// value: true | false
const Pref* PREF_ASYNC_DNS = makePref("async-dns");
// value: 1*digit
const Pref* PREF_SUMMARY_INTERVAL = makePref("summary-interval");
// value: debug, info, notice, warn, error
const Pref* PREF_LOG_LEVEL = makePref("log-level");
// value: inorder | feedback | adaptive
const Pref* PREF_URI_SELECTOR = makePref("uri-selector");
// value: 1*digit
const Pref* PREF_SERVER_STAT_TIMEOUT = makePref("server-stat-timeout");
// value: string that your file system recognizes as a file name.
const Pref* PREF_SERVER_STAT_IF = makePref("server-stat-if");
// value: string that your file system recognizes as a file name.
const Pref* PREF_SERVER_STAT_OF = makePref("server-stat-of");
// value: true | false
const Pref* PREF_REMOTE_TIME = makePref("remote-time");
// value: 1*digit
const Pref* PREF_MAX_FILE_NOT_FOUND = makePref("max-file-not-found");
// value: epoll | select
const Pref* PREF_EVENT_POLL = makePref("event-poll");
// value: true | false
const Pref* PREF_ENABLE_RPC = makePref("enable-rpc");
// value: 1*digit
const Pref* PREF_RPC_LISTEN_PORT = makePref("rpc-listen-port");
// value: string
const Pref* PREF_RPC_USER = makePref("rpc-user");
// value: string
const Pref* PREF_RPC_PASSWD = makePref("rpc-passwd");
// value: 1*digit
const Pref* PREF_RPC_MAX_REQUEST_SIZE = makePref("rpc-max-request-size");
// value: true | false
const Pref* PREF_RPC_LISTEN_ALL = makePref("rpc-listen-all");
// value: true | false
const Pref* PREF_RPC_ALLOW_ORIGIN_ALL = makePref("rpc-allow-origin-all");
// value: true | false
const Pref* PREF_DRY_RUN = makePref("dry-run");
// value: true | false
const Pref* PREF_REUSE_URI = makePref("reuse-uri");
// value: string
const Pref* PREF_ON_DOWNLOAD_START = makePref("on-download-start");
const Pref* PREF_ON_DOWNLOAD_PAUSE = makePref("on-download-pause");
const Pref* PREF_ON_DOWNLOAD_STOP = makePref("on-download-stop");
const Pref* PREF_ON_DOWNLOAD_COMPLETE = makePref("on-download-complete");
const Pref* PREF_ON_DOWNLOAD_ERROR = makePref("on-download-error");
// value: string
const Pref* PREF_INTERFACE = makePref("interface");
// value: true | false
const Pref* PREF_DISABLE_IPV6 = makePref("disable-ipv6");
// value: true | false
const Pref* PREF_HUMAN_READABLE = makePref("human-readable");
// value: true | false
const Pref* PREF_REMOVE_CONTROL_FILE = makePref("remove-control-file");
// value: true | false
const Pref* PREF_ALWAYS_RESUME = makePref("always-resume");
// value: 1*digit
const Pref* PREF_MAX_RESUME_FAILURE_TRIES = makePref("max-resume-failure-tries");
// value: string that your file system recognizes as a file name.
const Pref* PREF_SAVE_SESSION = makePref("save-session");
// value: 1*digit
const Pref* PREF_MAX_CONNECTION_PER_SERVER = makePref("max-connection-per-server");
// value: 1*digit
const Pref* PREF_MIN_SPLIT_SIZE = makePref("min-split-size");
// value: true | false
const Pref* PREF_CONDITIONAL_GET = makePref("conditional-get");
// value: true | false
const Pref* PREF_SELECT_LEAST_USED_HOST = makePref("select-least-used-host");
// value: true | false
const Pref* PREF_ENABLE_ASYNC_DNS6 = makePref("enable-async-dns6");
// value: 1*digit
const Pref* PREF_MAX_DOWNLOAD_RESULT = makePref("max-download-result");
// value: 1*digit
const Pref* PREF_RETRY_WAIT = makePref("retry-wait");
// value: string
const Pref* PREF_ASYNC_DNS_SERVER = makePref("async-dns-server");
// value: true | false
const Pref* PREF_SHOW_CONSOLE_READOUT = makePref("show-console-readout");
// value: default | inorder
const Pref* PREF_STREAM_PIECE_SELECTOR = makePref("stream-piece-selector");
// value: true | false
const Pref* PREF_TRUNCATE_CONSOLE_READOUT = makePref("truncate-console-readout");
// value: true | false
const Pref* PREF_PAUSE = makePref("pause");
// value: default | full
const Pref* PREF_DOWNLOAD_RESULT = makePref("download-result");
// value: true | false
const Pref* PREF_HASH_CHECK_ONLY = makePref("hash-check-only");
// values: hashType=digest
const Pref* PREF_CHECKSUM = makePref("checksum");
// value: pid
const Pref* PREF_STOP_WITH_PROCESS = makePref("stop-with-process");
// value: true | false
const Pref* PREF_ENABLE_MMAP = makePref("enable-mmap");

/**
 * FTP related preferences
 */
const Pref* PREF_FTP_USER = makePref("ftp-user");
const Pref* PREF_FTP_PASSWD = makePref("ftp-passwd");
// values: binary | ascii
const Pref* PREF_FTP_TYPE = makePref("ftp-type");
// values: true | false
const Pref* PREF_FTP_PASV = makePref("ftp-pasv");
// values: true | false
const Pref* PREF_FTP_REUSE_CONNECTION = makePref("ftp-reuse-connection");

/**
 * HTTP related preferences
 */
const Pref* PREF_HTTP_USER = makePref("http-user");
const Pref* PREF_HTTP_PASSWD = makePref("http-passwd");
// values: string
const Pref* PREF_USER_AGENT = makePref("user-agent");
// value: string that your file system recognizes as a file name.
const Pref* PREF_LOAD_COOKIES = makePref("load-cookies");
// value: string that your file system recognizes as a file name.
const Pref* PREF_SAVE_COOKIES = makePref("save-cookies");
// values: true | false
const Pref* PREF_ENABLE_HTTP_KEEP_ALIVE = makePref("enable-http-keep-alive");
// values: true | false
const Pref* PREF_ENABLE_HTTP_PIPELINING = makePref("enable-http-pipelining");
// value: 1*digit
const Pref* PREF_MAX_HTTP_PIPELINING = makePref("max-http-pipelining");
// value: string
const Pref* PREF_HEADER = makePref("header");
// value: string that your file system recognizes as a file name.
const Pref* PREF_CERTIFICATE = makePref("certificate");
// value: string that your file system recognizes as a file name.
const Pref* PREF_PRIVATE_KEY = makePref("private-key");
// value: string that your file system recognizes as a file name.
const Pref* PREF_CA_CERTIFICATE = makePref("ca-certificate");
// value: true | false
const Pref* PREF_CHECK_CERTIFICATE = makePref("check-certificate");
// value: true | false
const Pref* PREF_USE_HEAD = makePref("use-head");
// value: true | false
const Pref* PREF_HTTP_AUTH_CHALLENGE = makePref("http-auth-challenge");
// value: true | false
const Pref* PREF_HTTP_NO_CACHE = makePref("http-no-cache");
// value: true | false
const Pref* PREF_HTTP_ACCEPT_GZIP = makePref("http-accept-gzip");

/** 
 * Proxy related preferences
 */
const Pref* PREF_HTTP_PROXY = makePref("http-proxy");
const Pref* PREF_HTTPS_PROXY = makePref("https-proxy");
const Pref* PREF_FTP_PROXY = makePref("ftp-proxy");
const Pref* PREF_ALL_PROXY = makePref("all-proxy");
// values: comma separeted hostname or domain
const Pref* PREF_NO_PROXY = makePref("no-proxy");
// values: get | tunnel
const Pref* PREF_PROXY_METHOD = makePref("proxy-method");
const Pref* PREF_HTTP_PROXY_USER = makePref("http-proxy-user");
const Pref* PREF_HTTP_PROXY_PASSWD = makePref("http-proxy-passwd");
const Pref* PREF_HTTPS_PROXY_USER = makePref("https-proxy-user");
const Pref* PREF_HTTPS_PROXY_PASSWD = makePref("https-proxy-passwd");
const Pref* PREF_FTP_PROXY_USER = makePref("ftp-proxy-user");
const Pref* PREF_FTP_PROXY_PASSWD = makePref("ftp-proxy-passwd");
const Pref* PREF_ALL_PROXY_USER = makePref("all-proxy-user");
const Pref* PREF_ALL_PROXY_PASSWD = makePref("all-proxy-passwd");

/**
 * BitTorrent related preferences
 */
// values: 1*digit
const Pref* PREF_PEER_CONNECTION_TIMEOUT = makePref("peer-connection-timeout");
// values: 1*digit
const Pref* PREF_BT_TIMEOUT = makePref("bt-timeout");
// values: 1*digit
const Pref* PREF_BT_REQUEST_TIMEOUT = makePref("bt-request-timeout");
// values: true | false
const Pref* PREF_SHOW_FILES = makePref("show-files");
// values: 1*digit
const Pref* PREF_MAX_OVERALL_UPLOAD_LIMIT = makePref("max-overall-upload-limit");
// values: 1*digit
const Pref* PREF_MAX_UPLOAD_LIMIT = makePref("max-upload-limit");
// values: a string that your file system recognizes as a file name.
const Pref* PREF_TORRENT_FILE = makePref("torrent-file");
// values: 1*digit
const Pref* PREF_LISTEN_PORT = makePref("listen-port");
// values: true | false | mem
const Pref* PREF_FOLLOW_TORRENT = makePref("follow-torrent");
// values: 1*digit * = makePref(  = makePref(,|-) 1*digit);
const Pref* PREF_SELECT_FILE = makePref("select-file");
// values: 1*digit
const Pref* PREF_SEED_TIME = makePref("seed-time");
// values: 1*digit ['.' [ 1*digit ] ]
const Pref* PREF_SEED_RATIO = makePref("seed-ratio");
// values: 1*digit
const Pref* PREF_BT_KEEP_ALIVE_INTERVAL = makePref("bt-keep-alive-interval");
// values: a string, less than or equals to 20 bytes length
const Pref* PREF_PEER_ID_PREFIX = makePref("peer-id-prefix");
// values: true | false
const Pref* PREF_ENABLE_PEER_EXCHANGE = makePref("enable-peer-exchange");
// values: true | false
const Pref* PREF_ENABLE_DHT = makePref("enable-dht");
// values: a string
const Pref* PREF_DHT_LISTEN_ADDR = makePref("dht-listen-addr");
// values: 1*digit
const Pref* PREF_DHT_LISTEN_PORT = makePref("dht-listen-port");
// values: a string
const Pref* PREF_DHT_ENTRY_POINT_HOST = makePref("dht-entry-point-host");
// values: 1*digit
const Pref* PREF_DHT_ENTRY_POINT_PORT = makePref("dht-entry-point-port");
// values: a string  = makePref(hostname:port);
const Pref* PREF_DHT_ENTRY_POINT = makePref("dht-entry-point");
// values: a string
const Pref* PREF_DHT_FILE_PATH = makePref("dht-file-path");
// values: true | false
const Pref* PREF_ENABLE_DHT6 = makePref("enable-dht6");
// values: a string
const Pref* PREF_DHT_LISTEN_ADDR6 = makePref("dht-listen-addr6");
// values: a string
const Pref* PREF_DHT_ENTRY_POINT_HOST6 = makePref("dht-entry-point-host6");
// values: 1*digit
const Pref* PREF_DHT_ENTRY_POINT_PORT6 = makePref("dht-entry-point-port6");
// values: a string  = makePref(hostname:port)
const Pref* PREF_DHT_ENTRY_POINT6 = makePref("dht-entry-point6");
// values: a string
const Pref* PREF_DHT_FILE_PATH6 = makePref("dht-file-path6");
// values: plain | arc4
const Pref* PREF_BT_MIN_CRYPTO_LEVEL = makePref("bt-min-crypto-level");
// values:: true | false
const Pref* PREF_BT_REQUIRE_CRYPTO = makePref("bt-require-crypto");
// values: 1*digit
const Pref* PREF_BT_REQUEST_PEER_SPEED_LIMIT = makePref("bt-request-peer-speed-limit");
// values: 1*digit
const Pref* PREF_BT_MAX_OPEN_FILES = makePref("bt-max-open-files");
// values: true | false
const Pref* PREF_BT_SEED_UNVERIFIED = makePref("bt-seed-unverified");
// values: true | false
const Pref* PREF_BT_HASH_CHECK_SEED = makePref("bt-hash-check-seed");
// values: 1*digit
const Pref* PREF_BT_MAX_PEERS = makePref("bt-max-peers");
// values: a string  = makePref(IP address)
const Pref* PREF_BT_EXTERNAL_IP = makePref("bt-external-ip");
// values: 1*digit '=' a string that your file system recognizes as a file name.
const Pref* PREF_INDEX_OUT = makePref("index-out");
// values: 1*digit
const Pref* PREF_BT_TRACKER_INTERVAL = makePref("bt-tracker-interval");
// values: 1*digit
const Pref* PREF_BT_STOP_TIMEOUT = makePref("bt-stop-timeout");
// values: head[=SIZE]|tail[=SIZE], ...
const Pref* PREF_BT_PRIORITIZE_PIECE = makePref("bt-prioritize-piece");
// values: true | false
const Pref* PREF_BT_SAVE_METADATA = makePref("bt-save-metadata");
// values: true | false
const Pref* PREF_BT_METADATA_ONLY = makePref("bt-metadata-only");
// values: true | false
const Pref* PREF_BT_ENABLE_LPD = makePref("bt-enable-lpd");
// values: string
const Pref* PREF_BT_LPD_INTERFACE = makePref("bt-lpd-interface");
// values: 1*digit
const Pref* PREF_BT_TRACKER_TIMEOUT = makePref("bt-tracker-timeout");
// values: 1*digit
const Pref* PREF_BT_TRACKER_CONNECT_TIMEOUT = makePref("bt-tracker-connect-timeout");
// values: 1*digit
const Pref* PREF_DHT_MESSAGE_TIMEOUT = makePref("dht-message-timeout");
// values: string
const Pref* PREF_ON_BT_DOWNLOAD_COMPLETE = makePref("on-bt-download-complete");
// values: string
const Pref* PREF_BT_TRACKER = makePref("bt-tracker");
// values: string
const Pref* PREF_BT_EXCLUDE_TRACKER = makePref("bt-exclude-tracker");
// values: true | false
const Pref* PREF_BT_REMOVE_UNSELECTED_FILE =
  makePref("bt-remove-unselected-file");

/**
 * Metalink related preferences
 */
// values: a string that your file system recognizes as a file name.
const Pref* PREF_METALINK_FILE = makePref("metalink-file");
// values: a string
const Pref* PREF_METALINK_VERSION = makePref("metalink-version");
// values: a string
const Pref* PREF_METALINK_LANGUAGE = makePref("metalink-language");
// values: a string
const Pref* PREF_METALINK_OS = makePref("metalink-os");
// values: a string
const Pref* PREF_METALINK_LOCATION = makePref("metalink-location");
// values: 1*digit
const Pref* PREF_METALINK_SERVERS = makePref("metalink-servers");
// values: true | false | mem
const Pref* PREF_FOLLOW_METALINK = makePref("follow-metalink");
// values: http | https | ftp | none
const Pref* PREF_METALINK_PREFERRED_PROTOCOL = makePref("metalink-preferred-protocol");
// values: true | false
const Pref* PREF_METALINK_ENABLE_UNIQUE_PROTOCOL = makePref("metalink-enable-unique-protocol");
const Pref* PREF_METALINK_BASE_URI = makePref("metalink-base-uri");

} // namespace aria2
