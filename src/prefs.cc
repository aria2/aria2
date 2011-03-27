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

namespace aria2 {

/**
 * Constants
 */
const std::string A2_V_TRUE("true");
const std::string A2_V_FALSE("false");
const std::string V_NONE("none");
const std::string V_MEM("mem");
const std::string V_ALL("all");
/**
 * General preferences
 */
// values: 1*digit
const std::string PREF_TIMEOUT("timeout");
// values: 1*digit
const std::string PREF_DNS_TIMEOUT("dns-timeout");
// values: 1*digit
const std::string PREF_CONNECT_TIMEOUT("connect-timeout");
// values: 1*digit
const std::string PREF_MAX_TRIES("max-tries");
// values: 1*digit
const std::string PREF_AUTO_SAVE_INTERVAL("auto-save-interval");
// values: a string that your file system recognizes as a file name.
const std::string PREF_LOG("log");
// values: a string that your file system recognizes as a directory.
const std::string PREF_DIR("dir");
// values: a string that your file system recognizes as a file name.
const std::string PREF_OUT("out");
// values: 1*digit
const std::string PREF_SPLIT("split");
// value: true | false
const std::string PREF_DAEMON("daemon");
// value: a string
const std::string PREF_REFERER("referer");
// value: 1*digit
const std::string PREF_LOWEST_SPEED_LIMIT("lowest-speed-limit");
// value: 1*digit
const std::string PREF_SEGMENT_SIZE("segment-size");
// value: 1*digit
const std::string PREF_MAX_OVERALL_DOWNLOAD_LIMIT("max-overall-download-limit");
// value: 1*digit
const std::string PREF_MAX_DOWNLOAD_LIMIT("max-download-limit");
// value: 1*digit
const std::string PREF_STARTUP_IDLE_TIME("startup-idle-time");
// value: prealloc | fallc | none
const std::string PREF_FILE_ALLOCATION("file-allocation");
const std::string V_PREALLOC("prealloc");
const std::string V_FALLOC("falloc");
// value: 1*digit
const std::string PREF_NO_FILE_ALLOCATION_LIMIT("no-file-allocation-limit");
// value: true | false
const std::string PREF_ALLOW_OVERWRITE("allow-overwrite");
// value: true | false
const std::string PREF_REALTIME_CHUNK_CHECKSUM("realtime-chunk-checksum");
// value: true | false
const std::string PREF_CHECK_INTEGRITY("check-integrity");
// value: string that your file system recognizes as a file name.
const std::string PREF_NETRC_PATH("netrc-path");
// value:
const std::string PREF_CONTINUE("continue");
// value:
const std::string PREF_NO_NETRC("no-netrc");
// value: 1*digit
const std::string PREF_MAX_DOWNLOADS("max-downloads");
// value: string that your file system recognizes as a file name.
const std::string PREF_INPUT_FILE("input-file");
// value: 1*digit
const std::string PREF_MAX_CONCURRENT_DOWNLOADS("max-concurrent-downloads");
// value: true | false
const std::string PREF_FORCE_SEQUENTIAL("force-sequential");
// value: true | false
const std::string PREF_AUTO_FILE_RENAMING("auto-file-renaming");
// value: true | false
const std::string PREF_PARAMETERIZED_URI("parameterized-uri");
// value: true | false
const std::string PREF_ENABLE_DIRECT_IO("enable-direct-io");
// value: true | false
const std::string PREF_ALLOW_PIECE_LENGTH_CHANGE("allow-piece-length-change");
// value: true | false
const std::string PREF_NO_CONF("no-conf");
// value: string
const std::string PREF_CONF_PATH("conf-path");
// value: 1*digit
const std::string PREF_STOP("stop");
// value: true | false
const std::string PREF_QUIET("quiet");
// value: true | false
const std::string PREF_ASYNC_DNS("async-dns");
// value: 1*digit
const std::string PREF_SUMMARY_INTERVAL("summary-interval");
// value: debug, info, notice, warn, error
const std::string PREF_LOG_LEVEL("log-level");
const std::string V_DEBUG("debug");
const std::string V_INFO("info");
const std::string V_NOTICE("notice");
const std::string V_WARN("warn");
const std::string V_ERROR("error");
// value: inorder | feedback | adaptive
const std::string PREF_URI_SELECTOR("uri-selector");
const std::string V_INORDER("inorder");
const std::string V_FEEDBACK("feedback");
const std::string V_ADAPTIVE("adaptive");
// value: 1*digit
const std::string PREF_SERVER_STAT_TIMEOUT("server-stat-timeout");
// value: string that your file system recognizes as a file name.
const std::string PREF_SERVER_STAT_IF("server-stat-if");
// value: string that your file system recognizes as a file name.
const std::string PREF_SERVER_STAT_OF("server-stat-of");
// value: true | false
const std::string PREF_REMOTE_TIME("remote-time");
// value: 1*digit
const std::string PREF_MAX_FILE_NOT_FOUND("max-file-not-found");
// value: epoll | select
const std::string PREF_EVENT_POLL("event-poll");
const std::string V_EPOLL("epoll");
const std::string V_KQUEUE("kqueue");
const std::string V_PORT("port");
const std::string V_POLL("poll");
const std::string V_SELECT("select");
// value: true | false
const std::string PREF_ENABLE_RPC("enable-rpc");
// value: 1*digit
const std::string PREF_RPC_LISTEN_PORT("rpc-listen-port");
// value: string
const std::string PREF_RPC_USER("rpc-user");
// value: string
const std::string PREF_RPC_PASSWD("rpc-passwd");
// value: 1*digit
const std::string PREF_RPC_MAX_REQUEST_SIZE("rpc-max-request-size");
// value: true | false
const std::string PREF_RPC_LISTEN_ALL("rpc-listen-all");
// value: true | false
const std::string PREF_ENABLE_XML_RPC("enable-xml-rpc");
// value: 1*digit
const std::string PREF_XML_RPC_LISTEN_PORT("xml-rpc-listen-port");
// value: string
const std::string PREF_XML_RPC_USER("xml-rpc-user");
// value: string
const std::string PREF_XML_RPC_PASSWD("xml-rpc-passwd");
// value: 1*digit
const std::string PREF_XML_RPC_MAX_REQUEST_SIZE("xml-rpc-max-request-size");
// value: true | false
const std::string PREF_XML_RPC_LISTEN_ALL("xml-rpc-listen-all");
// value: true | false
const std::string PREF_DRY_RUN("dry-run");
// value: true | false
const std::string PREF_REUSE_URI("reuse-uri");
// value: string
const std::string PREF_ON_DOWNLOAD_START("on-download-start");
const std::string PREF_ON_DOWNLOAD_PAUSE("on-download-pause");
const std::string PREF_ON_DOWNLOAD_STOP("on-download-stop");
const std::string PREF_ON_DOWNLOAD_COMPLETE("on-download-complete");
const std::string PREF_ON_DOWNLOAD_ERROR("on-download-error");
// value: string
const std::string PREF_INTERFACE("interface");
// value: true | false
const std::string PREF_DISABLE_IPV6("disable-ipv6");
// value: true | false
const std::string PREF_HUMAN_READABLE("human-readable");
// value: true | false
const std::string PREF_REMOVE_CONTROL_FILE("remove-control-file");
// value: true | false
const std::string PREF_ALWAYS_RESUME("always-resume");
// value: 1*digit
const std::string PREF_MAX_RESUME_FAILURE_TRIES("max-resume-failure-tries");
// value: string that your file system recognizes as a file name.
const std::string PREF_SAVE_SESSION("save-session");
// value: 1*digit
const std::string PREF_MAX_CONNECTION_PER_SERVER("max-connection-per-server");
// value: 1*digit
const std::string PREF_MIN_SPLIT_SIZE("min-split-size");
// value: true | false
const std::string PREF_CONDITIONAL_GET("conditional-get");
// value: true | false
const std::string PREF_SELECT_LEAST_USED_HOST("select-least-used-host");
// value: true | false
const std::string PREF_ENABLE_ASYNC_DNS6("enable-async-dns6");
// value: 1*digit
const std::string PREF_MAX_DOWNLOAD_RESULT("max-download-result");
// value: 1*digit
const std::string PREF_RETRY_WAIT("retry-wait");
// value: string
const std::string PREF_ASYNC_DNS_SERVER("async-dns-server");
// value: true | false
const std::string PREF_SHOW_CONSOLE_READOUT("show-console-readout");

/**
 * FTP related preferences
 */
const std::string PREF_FTP_USER("ftp-user");
const std::string PREF_FTP_PASSWD("ftp-passwd");
// values: binary | ascii
const std::string PREF_FTP_TYPE("ftp-type");
const std::string V_BINARY("binary");
const std::string V_ASCII("ascii");
// values: true | false
const std::string PREF_FTP_PASV("ftp-pasv");
// values: true | false
const std::string PREF_FTP_REUSE_CONNECTION("ftp-reuse-connection");

/**
 * HTTP related preferences
 */
const std::string PREF_HTTP_USER("http-user");
const std::string PREF_HTTP_PASSWD("http-passwd");
// values: string
const std::string PREF_USER_AGENT("user-agent");
// value: string that your file system recognizes as a file name.
const std::string PREF_LOAD_COOKIES("load-cookies");
// value: string that your file system recognizes as a file name.
const std::string PREF_SAVE_COOKIES("save-cookies");
// values: true | false
const std::string PREF_ENABLE_HTTP_KEEP_ALIVE("enable-http-keep-alive");
// values: true | false
const std::string PREF_ENABLE_HTTP_PIPELINING("enable-http-pipelining");
// value: 1*digit
const std::string PREF_MAX_HTTP_PIPELINING("max-http-pipelining");
// value: string
const std::string PREF_HEADER("header");
// value: string that your file system recognizes as a file name.
const std::string PREF_CERTIFICATE("certificate");
// value: string that your file system recognizes as a file name.
const std::string PREF_PRIVATE_KEY("private-key");
// value: string that your file system recognizes as a file name.
const std::string PREF_CA_CERTIFICATE("ca-certificate");
// value: true | false
const std::string PREF_CHECK_CERTIFICATE("check-certificate");
// value: true | false
const std::string PREF_USE_HEAD("use-head");
// value: true | false
const std::string PREF_HTTP_AUTH_CHALLENGE("http-auth-challenge");
// value: true | false
const std::string PREF_HTTP_NO_CACHE("http-no-cache");
// value: true | false
const std::string PREF_HTTP_ACCEPT_GZIP("http-accept-gzip");

/** 
 * Proxy related preferences
 */
const std::string PREF_HTTP_PROXY("http-proxy");
const std::string PREF_HTTPS_PROXY("https-proxy");
const std::string PREF_FTP_PROXY("ftp-proxy");
const std::string PREF_ALL_PROXY("all-proxy");
// values: comma separeted hostname or domain
const std::string PREF_NO_PROXY("no-proxy");
// values: get | tunnel
const std::string PREF_PROXY_METHOD("proxy-method");
const std::string V_GET("get");
const std::string V_TUNNEL("tunnel");
const std::string PREF_HTTP_PROXY_USER("http-proxy-user");
const std::string PREF_HTTP_PROXY_PASSWD("http-proxy-passwd");
const std::string PREF_HTTPS_PROXY_USER("https-proxy-user");
const std::string PREF_HTTPS_PROXY_PASSWD("https-proxy-passwd");
const std::string PREF_FTP_PROXY_USER("ftp-proxy-user");
const std::string PREF_FTP_PROXY_PASSWD("ftp-proxy-passwd");
const std::string PREF_ALL_PROXY_USER("all-proxy-user");
const std::string PREF_ALL_PROXY_PASSWD("all-proxy-passwd");

/**
 * BitTorrent related preferences
 */
// values: 1*digit
const std::string PREF_PEER_CONNECTION_TIMEOUT("peer-connection-timeout");
// values: 1*digit
const std::string PREF_BT_TIMEOUT("bt-timeout");
// values: 1*digit
const std::string PREF_BT_REQUEST_TIMEOUT("bt-request-timeout");
// values: true | false
const std::string PREF_SHOW_FILES("show-files");
// values: 1*digit
const std::string PREF_MAX_OVERALL_UPLOAD_LIMIT("max-overall-upload-limit");
// values: 1*digit
const std::string PREF_MAX_UPLOAD_LIMIT("max-upload-limit");
// values: a string that your file system recognizes as a file name.
const std::string PREF_TORRENT_FILE("torrent-file");
// values: 1*digit
const std::string PREF_LISTEN_PORT("listen-port");
// values: true | false | mem
const std::string PREF_FOLLOW_TORRENT("follow-torrent");
// values: 1*digit *( (,|-) 1*digit);
const std::string PREF_SELECT_FILE("select-file");
// values: 1*digit
const std::string PREF_SEED_TIME("seed-time");
// values: 1*digit ['.' [ 1*digit ] ]
const std::string PREF_SEED_RATIO("seed-ratio");
// values: 1*digit
const std::string PREF_BT_KEEP_ALIVE_INTERVAL("bt-keep-alive-interval");
// values: a string, less than or equals to 20 bytes length
const std::string PREF_PEER_ID_PREFIX("peer-id-prefix");
// values: true | false
const std::string PREF_ENABLE_PEER_EXCHANGE("enable-peer-exchange");
// values: true | false
const std::string PREF_ENABLE_DHT("enable-dht");
// values: a string
const std::string PREF_DHT_LISTEN_ADDR("dht-listen-addr");
// values: 1*digit
const std::string PREF_DHT_LISTEN_PORT("dht-listen-port");
// values: a string
const std::string PREF_DHT_ENTRY_POINT_HOST("dht-entry-point-host");
// values: 1*digit
const std::string PREF_DHT_ENTRY_POINT_PORT("dht-entry-point-port");
// values: a string (hostname:port);
const std::string PREF_DHT_ENTRY_POINT("dht-entry-point");
// values: a string
const std::string PREF_DHT_FILE_PATH("dht-file-path");
// values: true | false
const std::string PREF_ENABLE_DHT6("enable-dht6");
// values: a string
const std::string PREF_DHT_LISTEN_ADDR6("dht-listen-addr6");
// values: a string
const std::string PREF_DHT_ENTRY_POINT_HOST6("dht-entry-point-host6");
// values: 1*digit
const std::string PREF_DHT_ENTRY_POINT_PORT6("dht-entry-point-port6");
// values: a string (hostname:port)
const std::string PREF_DHT_ENTRY_POINT6("dht-entry-point6");
// values: a string
const std::string PREF_DHT_FILE_PATH6("dht-file-path6");
// values: plain | arc4
const std::string PREF_BT_MIN_CRYPTO_LEVEL("bt-min-crypto-level");
const std::string V_PLAIN("plain");
const std::string V_ARC4("arc4");
// values:: true | false
const std::string PREF_BT_REQUIRE_CRYPTO("bt-require-crypto");
// values: 1*digit
const std::string PREF_BT_REQUEST_PEER_SPEED_LIMIT("bt-request-peer-speed-limit");
// values: 1*digit
const std::string PREF_BT_MAX_OPEN_FILES("bt-max-open-files");
// values: true | false
const std::string PREF_BT_SEED_UNVERIFIED("bt-seed-unverified");
// values: true | false
const std::string PREF_BT_HASH_CHECK_SEED("bt-hash-check-seed");
// values: 1*digit
const std::string PREF_BT_MAX_PEERS("bt-max-peers");
// values: a string (IP address)
const std::string PREF_BT_EXTERNAL_IP("bt-external-ip");
// values: 1*digit '=' a string that your file system recognizes as a file name.
const std::string PREF_INDEX_OUT("index-out");
// values: 1*digit
const std::string PREF_BT_TRACKER_INTERVAL("bt-tracker-interval");
// values: 1*digit
const std::string PREF_BT_STOP_TIMEOUT("bt-stop-timeout");
// values: head[=SIZE]|tail[=SIZE], ...
const std::string PREF_BT_PRIORITIZE_PIECE("bt-prioritize-piece");
// values: true | false
const std::string PREF_BT_SAVE_METADATA("bt-save-metadata");
// values: true | false
const std::string PREF_BT_METADATA_ONLY("bt-metadata-only");
// values: true | false
const std::string PREF_BT_ENABLE_LPD("bt-enable-lpd");
// values: string
const std::string PREF_BT_LPD_INTERFACE("bt-lpd-interface");
// values: 1*digit
const std::string PREF_BT_TRACKER_TIMEOUT("bt-tracker-timeout");
// values: 1*digit
const std::string PREF_BT_TRACKER_CONNECT_TIMEOUT("bt-tracker-connect-timeout");
// values: 1*digit
const std::string PREF_DHT_MESSAGE_TIMEOUT("dht-message-timeout");
// values: string
const std::string PREF_ON_BT_DOWNLOAD_COMPLETE("on-bt-download-complete");
// values: string
const std::string PREF_BT_TRACKER("bt-tracker");
// values: string
const std::string PREF_BT_EXCLUDE_TRACKER("bt-exclude-tracker");

/**
 * Metalink related preferences
 */
// values: a string that your file system recognizes as a file name.
const std::string PREF_METALINK_FILE("metalink-file");
// values: a string
const std::string PREF_METALINK_VERSION("metalink-version");
// values: a string
const std::string PREF_METALINK_LANGUAGE("metalink-language");
// values: a string
const std::string PREF_METALINK_OS("metalink-os");
// values: a string
const std::string PREF_METALINK_LOCATION("metalink-location");
// values: 1*digit
const std::string PREF_METALINK_SERVERS("metalink-servers");
// values: true | false | mem
const std::string PREF_FOLLOW_METALINK("follow-metalink");
// values: http | https | ftp | none
const std::string PREF_METALINK_PREFERRED_PROTOCOL("metalink-preferred-protocol");
const std::string V_HTTP("http");
const std::string V_HTTPS("https");
const std::string V_FTP("ftp");
// values: true | false
const std::string PREF_METALINK_ENABLE_UNIQUE_PROTOCOL("metalink-enable-unique-protocol");

} // namespace aria2
