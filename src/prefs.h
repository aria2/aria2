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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
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
#ifndef _D_PREFS_H_
#define _D_PREFS_H_

#include "common.h"

/**
 * Constants
 */
#undef V_TRUE
#define V_TRUE "true"
#undef V_FALSE
#define V_FALSE "false"
#undef V_NONE
#define V_NONE "none"
#define V_MEM "mem"
#define V_ALL "all"
/**
 * General preferences
 */
// values: 1*digit
#define PREF_RETRY_WAIT "retry-wait"
// values: 1*digit
#define PREF_TIMEOUT "timeout"
// values: 1*digit
#define PREF_DNS_TIMEOUT "dns-timeout"
// values: 1*digit
#define PREF_MAX_TRIES "max-tries"
// values: 1*digit
#define PREF_MIN_SEGMENT_SIZE "min-segment-size"
// values: 1*digit
#define PREF_AUTO_SAVE_INTERVAL "auto-save-interval"
// values: true | false
#define PREF_STDOUT_LOG "stdout-log"
// values: a string that your file system recognizes as a file name.
#define PREF_LOG "log"
// values: a string that your file system recognizes as a directory.
#define PREF_DIR "dir"
// values: a string that your file system recognizes as a file name.
#define PREF_OUT "out"
// values: 1*digit
#define PREF_SPLIT "split"
// value: true | false
#define PREF_DAEMON "daemon"
// value: a string
#define PREF_REFERER "referer"
// value: 1*digit
#define PREF_LOWEST_SPEED_LIMIT "lowest-speed-limit"
// value: 1*digit
#define PREF_SEGMENT_SIZE "segment-size"
// value: 1*digit
#define PREF_MAX_DOWNLOAD_LIMIT "max-download-limit"
// value: 1*digit
#define PREF_STARTUP_IDLE_TIME "startup-idle-time"
// value: prealloc | none
#define PREF_FILE_ALLOCATION "file-allocation"
#  define V_PREALLOC "prealloc"
#// value: 1*digit
#define PREF_NO_FILE_ALLOCATION_LIMIT "no-file-allocation-limit"
// value: true | false
#define PREF_ALLOW_OVERWRITE "allow-overwrite"
// value: true | false
#define PREF_REALTIME_CHUNK_CHECKSUM "realtime-chunk-checksum"
// value: true | false
#define PREF_CHECK_INTEGRITY "check-integrity"
// value: string that your file system recognizes as a file name.
#define PREF_NETRC_PATH "netrc-path"
// value:
#define PREF_CONTINUE "continue"
// value:
#define PREF_NO_NETRC "no-netrc"
// value: 1*digit
#define PREF_MAX_DOWNLOADS "max-downloads"
// value: string that your file system recognizes as a file name.
#define PREF_INPUT_FILE "input-file"
// value: 1*digit
#define PREF_MAX_CONCURRENT_DOWNLOADS "max-concurrent-downloads"
// value: 1*digit
#define PREF_DIRECT_DOWNLOAD_TIMEOUT "direct-download-timeout"
// value: true | false
#define PREF_FORCE_SEQUENTIAL "force-sequential"
// value: true | false
#define PREF_AUTO_FILE_RENAMING "auto-file-renaming"
// value: true | false
#define PREF_PARAMETERIZED_URI "parameterized-uri"
// value: true | false
#define PREF_ENABLE_DIRECT_IO "enable-direct-io"
// value: true | false
#define PREF_ALLOW_PIECE_LENGTH_CHANGE "allow-piece-length-change"
// value: true | false
#define PREF_NO_CONF "no-conf"
// value: string
#define PREF_CONF_PATH "conf-path"
// value: 1*digit
#define PREF_STOP "stop"
// value: true | false
#define PREF_QUIET "quiet"

/**
 * FTP related preferences
 */
#define PREF_FTP_USER "ftp-user"
#define PREF_FTP_PASSWD "ftp-passwd"
// values: binary | ascii
#define PREF_FTP_TYPE "ftp-type"
#  define V_BINARY "binary"
#  define V_ASCII "ascii"
// values: get | tunnel
#define PREF_FTP_VIA_HTTP_PROXY "ftp-via-http-proxy"
#  define V_GET "get"
#  define V_TUNNEL "tunnel"
// values: true | false
#define PREF_FTP_PASV "ftp-pasv"

/**
 * HTTP related preferences
 */
#define PREF_HTTP_USER "http-user"
#define PREF_HTTP_PASSWD "http-passwd"
// values: basic
#define PREF_HTTP_AUTH_SCHEME "http-auth-scheme"
#  define V_BASIC "basic"
// values: true | false
#define PREF_HTTP_AUTH_ENABLED "http-auth-enabled"
// values: string
#define PREF_USER_AGENT "user-agent"
// value: string that your file system recognizes as a file name.
#define PREF_LOAD_COOKIES "load-cookies"
// values: true | false
#define PREF_ENABLE_HTTP_KEEP_ALIVE "enable-http-keep-alive"
// values: true | false
#define PREF_ENABLE_HTTP_PIPELINING "enable-http-pipelining"
// value: 1*digit
#define PREF_MAX_HTTP_PIPELINING "max-http-pipelining"
// value: string
#define PREF_HEADER "header"

/** 
 * HTTP proxy related preferences
 */
#define PREF_HTTP_PROXY "http-proxy"
#define PREF_HTTP_PROXY_USER "http-proxy-user"
#define PREF_HTTP_PROXY_PASSWD "http-proxy-passwd"
#define PREF_HTTP_PROXY_HOST "http-proxy-host"
#define PREF_HTTP_PROXY_PORT "http-proxy-port"
// values: get | tunnel
#define PREF_HTTP_PROXY_METHOD "http-proxy-method"
// values: true | false
#define PREF_HTTP_PROXY_ENABLED "http-proxy-enabled"
// values: true | false
#define PREF_HTTP_PROXY_AUTH_ENABLED "http-proxy-auth-enabled"

/**
 * BitTorrent related preferences
 */
// values: 1*digit
#define PREF_PEER_CONNECTION_TIMEOUT "peer-connection-timeout"
// values: 1*digit
#define PREF_BT_TIMEOUT "bt-timeout"
// values: 1*digit
#define PREF_BT_REQUEST_TIMEOUT "bt-request-timeout"
// values: true | false
#define PREF_SHOW_FILES "show-files"
// values: true | false
#define PREF_NO_PREALLOCATION "no-preallocation"
// values: true | false
#define PREF_DIRECT_FILE_MAPPING "direct-file-mapping"
// values: 1*digit
#define PREF_MAX_UPLOAD_LIMIT "max-upload-limit"
// values: a string that your file system recognizes as a file name.
#define PREF_TORRENT_FILE "torrent-file"
// values: 1*digit
#define PREF_LISTEN_PORT "listen-port"
// values: true | false | mem
#define PREF_FOLLOW_TORRENT "follow-torrent"
// values: 1*digit *( (,|-) 1*digit)
#define PREF_SELECT_FILE "select-file"
// values: 1*digit
#define PREF_SEED_TIME "seed-time"
// values: 1*digit ['.' [ 1*digit ] ]
#define PREF_SEED_RATIO "seed-ratio"
// values: 1*digit
#define PREF_TRACKER_MAX_TRIES "tracker-max-tries"
// values: 1*digit
#define PREF_BT_KEEP_ALIVE_INTERVAL "bt-keep-alive-interval"
// values: a string, less than or equals to 20 bytes length
#define PREF_PEER_ID_PREFIX "peer-id-prefix"
// values: true | false
#define PREF_ENABLE_PEER_EXCHANGE "enable-peer-exchange"
// values: true | false
#define PREF_ENABLE_DHT "enable-dht"
// values: 1*digit
#define PREF_DHT_LISTEN_PORT "dht-listen-port"
// values: a string
#define PREF_DHT_ENTRY_POINT_HOST "dht-entry-point-host"
// values: 1*digit
#define PREF_DHT_ENTRY_POINT_PORT "dht-entry-point-port"
// values: a string (hostname:port)
#define PREF_DHT_ENTRY_POINT "dht-entry-point"
// values: a string
#define PREF_DHT_FILE_PATH "dht-file-path"
// values: plain | arc4
#define PREF_BT_MIN_CRYPTO_LEVEL "bt-min-crypto-level"
#  define V_PLAIN "plain"
#  define V_ARC4 "arc4"
// values:: true | false
#define PREF_BT_REQUIRE_CRYPTO "bt-require-crypto"

/**
 * Metalink related preferences
 */
// values: a string that your file system recognizes as a file name.
#define PREF_METALINK_FILE "metalink-file"
// values: a string
#define PREF_METALINK_VERSION "metalink-version"
// values: a string
#define PREF_METALINK_LANGUAGE "metalink-language"
// values: a string
#define PREF_METALINK_OS "metalink-os"
// values: a string
#define PREF_METALINK_LOCATION "metalink-location"
// values: 1*digit
#define PREF_METALINK_SERVERS "metalink-servers"
// values: true | false | mem
#define PREF_FOLLOW_METALINK "follow-metalink"
// values: http | https | ftp | none
#define PREF_METALINK_PREFERRED_PROTOCOL "metalink-preferred-protocol"
#  define V_HTTP "http"
#  define V_HTTPS "https"
#  define V_FTP "ftp"
// values: true | false
#define PREF_METALINK_ENABLE_UNIQUE_PROTOCOL "metalink-enable-unique-protocol"

#endif // _D_PREFS_H_
