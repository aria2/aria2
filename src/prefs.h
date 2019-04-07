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
#ifndef D_PREFS_H
#define D_PREFS_H

#include "common.h"
#include <string>

namespace aria2 {

struct Pref {
  Pref(const char* k, size_t i);
  // Keyword, aka Option Name
  const char* k;
  // Option ID
  size_t i;
};

typedef const Pref* PrefPtr;

namespace option {

// Returns the number of options.
size_t countOption();

// Returns Pref whose ID is id. id must be less than countOption().
PrefPtr i2p(size_t id);

// Returns Pref whose keyword is k. If no such Pref is found, returns
// special null Pref whose ID is 0.
PrefPtr k2p(const std::string& k);

// Deletes resources allocated for preferences. Call this function at
// the end of the program only once.
void deletePrefResource();

} // namespace option

/**
 * Constants
 */
extern const std::string A2_V_TRUE;
extern const std::string A2_V_FALSE;
extern const std::string A2_V_DEFAULT;
extern const std::string V_NONE;
extern const std::string V_MEM;
extern const std::string V_ALL;
extern const std::string A2_V_FULL;
extern const std::string A2_V_HIDE;
extern const std::string A2_V_GEOM;
extern const std::string V_PREALLOC;
extern const std::string V_FALLOC;
extern const std::string V_TRUNC;
extern const std::string V_DEBUG;
extern const std::string V_INFO;
extern const std::string V_NOTICE;
extern const std::string V_WARN;
extern const std::string V_ERROR;
extern const std::string V_INORDER;
extern const std::string A2_V_RANDOM;
extern const std::string V_FEEDBACK;
extern const std::string V_ADAPTIVE;
extern const std::string V_LIBUV;
extern const std::string V_EPOLL;
extern const std::string V_KQUEUE;
extern const std::string V_PORT;
extern const std::string V_POLL;
extern const std::string V_SELECT;
extern const std::string V_BINARY;
extern const std::string V_ASCII;
extern const std::string V_GET;
extern const std::string V_TUNNEL;
extern const std::string V_PLAIN;
extern const std::string V_ARC4;
extern const std::string V_HTTP;
extern const std::string V_HTTPS;
extern const std::string V_FTP;
extern const std::string A2_V_TLS11;
extern const std::string A2_V_TLS12;
extern const std::string A2_V_TLS13;

extern PrefPtr PREF_VERSION;
extern PrefPtr PREF_HELP;

/**
 * General preferences
 */
// values: 1*digit
extern PrefPtr PREF_TIMEOUT;
// values: 1*digit
extern PrefPtr PREF_DNS_TIMEOUT;
// values: 1*digit
extern PrefPtr PREF_CONNECT_TIMEOUT;
// values: 1*digit
extern PrefPtr PREF_MAX_TRIES;
// values: 1*digit
extern PrefPtr PREF_AUTO_SAVE_INTERVAL;
// values: a string that your file system recognizes as a file name.
extern PrefPtr PREF_LOG;
// values: a string that your file system recognizes as a directory.
extern PrefPtr PREF_DIR;
// values: a string that your file system recognizes as a file name.
extern PrefPtr PREF_OUT;
// values: 1*digit
extern PrefPtr PREF_SPLIT;
// value: true | false
extern PrefPtr PREF_DAEMON;
// value: a string
extern PrefPtr PREF_REFERER;
// value: 1*digit
extern PrefPtr PREF_LOWEST_SPEED_LIMIT;
// value: 1*digit
extern PrefPtr PREF_PIECE_LENGTH;
// value: 1*digit
extern PrefPtr PREF_MAX_DOWNLOAD_LIMIT;
// value: 1*digit
extern PrefPtr PREF_STARTUP_IDLE_TIME;
// value: prealloc | falloc | none
extern PrefPtr PREF_FILE_ALLOCATION;
// value: 1*digit
extern PrefPtr PREF_NO_FILE_ALLOCATION_LIMIT;
// value: true | false
extern PrefPtr PREF_ALLOW_OVERWRITE;
// value: true | false
extern PrefPtr PREF_REALTIME_CHUNK_CHECKSUM;
// value: true | false
extern PrefPtr PREF_CHECK_INTEGRITY;
// value: string that your file system recognizes as a file name.
extern PrefPtr PREF_NETRC_PATH;
// value:
extern PrefPtr PREF_CONTINUE;
// value:
extern PrefPtr PREF_NO_NETRC;
// value: 1*digit
extern PrefPtr PREF_MAX_OVERALL_DOWNLOAD_LIMIT;
// value: 1*digit
extern PrefPtr PREF_MAX_DOWNLOADS;
// value: string that your file system recognizes as a file name.
extern PrefPtr PREF_INPUT_FILE;
// value: true | false
extern PrefPtr PREF_DEFERRED_INPUT;
// value: 1*digit
extern PrefPtr PREF_MAX_CONCURRENT_DOWNLOADS;
// value: true | false
extern PrefPtr PREF_OPTIMIZE_CONCURRENT_DOWNLOADS;
// value: 1*digit ['.' [ 1*digit ] ]
extern PrefPtr PREF_OPTIMIZE_CONCURRENT_DOWNLOADS_COEFFA;
// value: 1*digit ['.' [ 1*digit ] ]
extern PrefPtr PREF_OPTIMIZE_CONCURRENT_DOWNLOADS_COEFFB;
// value: true | false
extern PrefPtr PREF_FORCE_SEQUENTIAL;
// value: true | false
extern PrefPtr PREF_AUTO_FILE_RENAMING;
// value: true | false
extern PrefPtr PREF_PARAMETERIZED_URI;
// value: true | false
extern PrefPtr PREF_ALLOW_PIECE_LENGTH_CHANGE;
// value: true | false
extern PrefPtr PREF_NO_CONF;
// value: string
extern PrefPtr PREF_CONF_PATH;
// value: 1*digit
extern PrefPtr PREF_STOP;
// value: true | false
extern PrefPtr PREF_QUIET;
// value: true | false
extern PrefPtr PREF_ASYNC_DNS;
// value: 1*digit
extern PrefPtr PREF_SUMMARY_INTERVAL;
// value: debug, info, notice, warn, error
extern PrefPtr PREF_LOG_LEVEL;
// value: debug, info, notice, warn, error
extern PrefPtr PREF_CONSOLE_LOG_LEVEL;
// value: inorder | feedback | adaptive
extern PrefPtr PREF_URI_SELECTOR;
// value: 1*digit
extern PrefPtr PREF_SERVER_STAT_TIMEOUT;
// value: string that your file system recognizes as a file name.
extern PrefPtr PREF_SERVER_STAT_IF;
// value: string that your file system recognizes as a file name.
extern PrefPtr PREF_SERVER_STAT_OF;
// value: true | false
extern PrefPtr PREF_REMOTE_TIME;
// value: 1*digit
extern PrefPtr PREF_MAX_FILE_NOT_FOUND;
// value: epoll | select
extern PrefPtr PREF_EVENT_POLL;
// value: true | false
extern PrefPtr PREF_ENABLE_RPC;
// value: 1*digit
extern PrefPtr PREF_RPC_LISTEN_PORT;
// value: string
extern PrefPtr PREF_RPC_USER;
// value: string
extern PrefPtr PREF_RPC_PASSWD;
// value: 1*digit
extern PrefPtr PREF_RPC_MAX_REQUEST_SIZE;
// value: true | false
extern PrefPtr PREF_RPC_LISTEN_ALL;
// value: true | false
extern PrefPtr PREF_RPC_ALLOW_ORIGIN_ALL;
// value: string that your file system recognizes as a file name.
extern PrefPtr PREF_RPC_CERTIFICATE;
// value: string that your file system recognizes as a file name.
extern PrefPtr PREF_RPC_PRIVATE_KEY;
// value: true | false
extern PrefPtr PREF_RPC_SECURE;
// value: true | false
extern PrefPtr PREF_RPC_SAVE_UPLOAD_METADATA;
// value: true | false
extern PrefPtr PREF_DRY_RUN;
// value: true | false
extern PrefPtr PREF_REUSE_URI;
// value: string
extern PrefPtr PREF_ON_DOWNLOAD_START;
extern PrefPtr PREF_ON_DOWNLOAD_PAUSE;
extern PrefPtr PREF_ON_DOWNLOAD_STOP;
extern PrefPtr PREF_ON_DOWNLOAD_COMPLETE;
extern PrefPtr PREF_ON_DOWNLOAD_ERROR;
// value: string
extern PrefPtr PREF_INTERFACE;
// value: string
extern PrefPtr PREF_MULTIPLE_INTERFACE;
// value: true | false
extern PrefPtr PREF_DISABLE_IPV6;
// value: true | false
extern PrefPtr PREF_HUMAN_READABLE;
// value: true | false
extern PrefPtr PREF_REMOVE_CONTROL_FILE;
// value: true | false
extern PrefPtr PREF_ALWAYS_RESUME;
// value: 1*digit
extern PrefPtr PREF_MAX_RESUME_FAILURE_TRIES;
// value: string that your file system recognizes as a file name.
extern PrefPtr PREF_SAVE_SESSION;
// value: 1*digit
extern PrefPtr PREF_MAX_CONNECTION_PER_SERVER;
// value: 1*digit
extern PrefPtr PREF_MIN_SPLIT_SIZE;
// value: true | false
extern PrefPtr PREF_CONDITIONAL_GET;
// value: true | false
extern PrefPtr PREF_SELECT_LEAST_USED_HOST;
// value: true | false
extern PrefPtr PREF_ENABLE_ASYNC_DNS6;
// value: 1*digit
extern PrefPtr PREF_MAX_DOWNLOAD_RESULT;
// value: 1*digit
extern PrefPtr PREF_RETRY_WAIT;
// value: string
extern PrefPtr PREF_ASYNC_DNS_SERVER;
// value: true | false
extern PrefPtr PREF_SHOW_CONSOLE_READOUT;
// value: default | inorder | geom
extern PrefPtr PREF_STREAM_PIECE_SELECTOR;
// value: true | false
extern PrefPtr PREF_TRUNCATE_CONSOLE_READOUT;
// value: true | false
extern PrefPtr PREF_PAUSE;
// value: default | full | hide
extern PrefPtr PREF_DOWNLOAD_RESULT;
// value: true | false
extern PrefPtr PREF_HASH_CHECK_ONLY;
// values: hashType=digest
extern PrefPtr PREF_CHECKSUM;
// value: pid
extern PrefPtr PREF_STOP_WITH_PROCESS;
// value: true | false
extern PrefPtr PREF_ENABLE_MMAP;
// value: true | false
extern PrefPtr PREF_FORCE_SAVE;
// value: true | false
extern PrefPtr PREF_SAVE_NOT_FOUND;
// value: 1*digit
extern PrefPtr PREF_DISK_CACHE;
// value: string
extern PrefPtr PREF_GID;
// values: 1*digit
extern PrefPtr PREF_SAVE_SESSION_INTERVAL;
// value: true |false
extern PrefPtr PREF_ENABLE_COLOR;
// value: string
extern PrefPtr PREF_RPC_SECRET;
// values: 1*digit
extern PrefPtr PREF_DSCP;
// values: true | false
extern PrefPtr PREF_PAUSE_METADATA;
// values: 1*digit
extern PrefPtr PREF_RLIMIT_NOFILE;
// values: SSLv3 | TLSv1 | TLSv1.1 | TLSv1.2
extern PrefPtr PREF_MIN_TLS_VERSION;
// value: 1*digit
extern PrefPtr PREF_SOCKET_RECV_BUFFER_SIZE;
// value: 1*digit
extern PrefPtr PREF_MAX_MMAP_LIMIT;
// value: true | false
extern PrefPtr PREF_STDERR;
// value: true | false
extern PrefPtr PREF_KEEP_UNFINISHED_DOWNLOAD_RESULT;

/**
 * FTP related preferences
 */
extern PrefPtr PREF_FTP_USER;
extern PrefPtr PREF_FTP_PASSWD;
// values: binary | ascii
extern PrefPtr PREF_FTP_TYPE;
// values: true | false
extern PrefPtr PREF_FTP_PASV;
// values: true | false
extern PrefPtr PREF_FTP_REUSE_CONNECTION;
// values: hashType=digest
extern PrefPtr PREF_SSH_HOST_KEY_MD;

/**
 * HTTP related preferences
 */
extern PrefPtr PREF_HTTP_USER;
extern PrefPtr PREF_HTTP_PASSWD;
// values: string
extern PrefPtr PREF_USER_AGENT;
// value: string that your file system recognizes as a file name.
extern PrefPtr PREF_LOAD_COOKIES;
// value: string that your file system recognizes as a file name.
extern PrefPtr PREF_SAVE_COOKIES;
// values: true | false
extern PrefPtr PREF_ENABLE_HTTP_KEEP_ALIVE;
// values: true | false
extern PrefPtr PREF_ENABLE_HTTP_PIPELINING;
// value: 1*digit
extern PrefPtr PREF_MAX_HTTP_PIPELINING;
// value: string
extern PrefPtr PREF_HEADER;
// value: string that your file system recognizes as a file name.
extern PrefPtr PREF_CERTIFICATE;
// value: string that your file system recognizes as a file name.
extern PrefPtr PREF_PRIVATE_KEY;
// value: string that your file system recognizes as a file name.
extern PrefPtr PREF_CA_CERTIFICATE;
// value: true | false
extern PrefPtr PREF_CHECK_CERTIFICATE;
// value: true | false
extern PrefPtr PREF_USE_HEAD;
// value: true | false
extern PrefPtr PREF_HTTP_AUTH_CHALLENGE;
// value: true | false
extern PrefPtr PREF_HTTP_NO_CACHE;
// value: true | false
extern PrefPtr PREF_HTTP_ACCEPT_GZIP;
// value: true | false
extern PrefPtr PREF_CONTENT_DISPOSITION_DEFAULT_UTF8;

/**;
 * Proxy related preferences
 */
extern PrefPtr PREF_HTTP_PROXY;
extern PrefPtr PREF_HTTPS_PROXY;
extern PrefPtr PREF_FTP_PROXY;
extern PrefPtr PREF_ALL_PROXY;
// values: comma separated hostname or domain
extern PrefPtr PREF_NO_PROXY;
// values: get | tunnel
extern PrefPtr PREF_PROXY_METHOD;
extern PrefPtr PREF_HTTP_PROXY_USER;
extern PrefPtr PREF_HTTP_PROXY_PASSWD;
extern PrefPtr PREF_HTTPS_PROXY_USER;
extern PrefPtr PREF_HTTPS_PROXY_PASSWD;
extern PrefPtr PREF_FTP_PROXY_USER;
extern PrefPtr PREF_FTP_PROXY_PASSWD;
extern PrefPtr PREF_ALL_PROXY_USER;
extern PrefPtr PREF_ALL_PROXY_PASSWD;

/**
 * BitTorrent related preferences
 */
// values: 1*digit
extern PrefPtr PREF_PEER_CONNECTION_TIMEOUT;
// values: 1*digit
extern PrefPtr PREF_BT_TIMEOUT;
// values: 1*digit
extern PrefPtr PREF_BT_REQUEST_TIMEOUT;
// values: true | false
extern PrefPtr PREF_SHOW_FILES;
// values: 1*digit
extern PrefPtr PREF_MAX_OVERALL_UPLOAD_LIMIT;
// values: 1*digit
extern PrefPtr PREF_MAX_UPLOAD_LIMIT;
// values: a string that your file system recognizes as a file name.
extern PrefPtr PREF_TORRENT_FILE;
// values: 1*digit
extern PrefPtr PREF_LISTEN_PORT;
// values: true | false | mem
extern PrefPtr PREF_FOLLOW_TORRENT;
// values: 1*digit *( (,|-) 1*digit)
extern PrefPtr PREF_SELECT_FILE;
// values: 1*digit ['.' [ 1*digit ] ]
extern PrefPtr PREF_SEED_TIME;
// values: 1*digit ['.' [ 1*digit ] ]
extern PrefPtr PREF_SEED_RATIO;
// values: 1*digit
extern PrefPtr PREF_BT_KEEP_ALIVE_INTERVAL;
// values: a string, less than or equals to 20 bytes length
extern PrefPtr PREF_PEER_ID_PREFIX;
// values: a string representing the extended BT handshake peer user agent
extern PrefPtr PREF_PEER_AGENT;
// values: true | false
extern PrefPtr PREF_ENABLE_PEER_EXCHANGE;
// values: true | false
extern PrefPtr PREF_ENABLE_DHT;
// values: a string
extern PrefPtr PREF_DHT_LISTEN_ADDR;
// values: 1*digit
extern PrefPtr PREF_DHT_LISTEN_PORT;
// values: a string
extern PrefPtr PREF_DHT_ENTRY_POINT_HOST;
// values: 1*digit
extern PrefPtr PREF_DHT_ENTRY_POINT_PORT;
// values: a string (hostname:port)
extern PrefPtr PREF_DHT_ENTRY_POINT;
// values: a string
extern PrefPtr PREF_DHT_FILE_PATH;
// values: true | false
extern PrefPtr PREF_ENABLE_DHT6;
// values: a string
extern PrefPtr PREF_DHT_LISTEN_ADDR6;
// values: a string
extern PrefPtr PREF_DHT_ENTRY_POINT_HOST6;
// values: 1*digit
extern PrefPtr PREF_DHT_ENTRY_POINT_PORT6;
// values: a string (hostname:port)
extern PrefPtr PREF_DHT_ENTRY_POINT6;
// values: a string
extern PrefPtr PREF_DHT_FILE_PATH6;
// values: plain | arc4
extern PrefPtr PREF_BT_MIN_CRYPTO_LEVEL;
// values:: true | false
extern PrefPtr PREF_BT_REQUIRE_CRYPTO;
// values: 1*digit
extern PrefPtr PREF_BT_REQUEST_PEER_SPEED_LIMIT;
// values: 1*digit
extern PrefPtr PREF_BT_MAX_OPEN_FILES;
// values: true | false
extern PrefPtr PREF_BT_SEED_UNVERIFIED;
// values: true | false
extern PrefPtr PREF_BT_HASH_CHECK_SEED;
// values: 1*digit
extern PrefPtr PREF_BT_MAX_PEERS;
// values: a string (IP address)
extern PrefPtr PREF_BT_EXTERNAL_IP;
// values: 1*digit '=' a string that your file system recognizes as a file name.
extern PrefPtr PREF_INDEX_OUT;
// values: 1*digit
extern PrefPtr PREF_BT_TRACKER_INTERVAL;
// values: 1*digit
extern PrefPtr PREF_BT_STOP_TIMEOUT;
// values: head[=SIZE]|tail[=SIZE], ...
extern PrefPtr PREF_BT_PRIORITIZE_PIECE;
// values: true | false
extern PrefPtr PREF_BT_SAVE_METADATA;
// values: true | false
extern PrefPtr PREF_BT_METADATA_ONLY;
// values: true | false
extern PrefPtr PREF_BT_ENABLE_LPD;
// values: string
extern PrefPtr PREF_BT_LPD_INTERFACE;
// values: 1*digit
extern PrefPtr PREF_BT_TRACKER_TIMEOUT;
// values: 1*digit
extern PrefPtr PREF_BT_TRACKER_CONNECT_TIMEOUT;
// values: 1*digit
extern PrefPtr PREF_DHT_MESSAGE_TIMEOUT;
// values: string
extern PrefPtr PREF_ON_BT_DOWNLOAD_COMPLETE;
// values: string
extern PrefPtr PREF_BT_TRACKER;
// values: string
extern PrefPtr PREF_BT_EXCLUDE_TRACKER;
// values: true | false
extern PrefPtr PREF_BT_REMOVE_UNSELECTED_FILE;
// values: true |false
extern PrefPtr PREF_BT_DETACH_SEED_ONLY;
// values: true | false
extern PrefPtr PREF_BT_FORCE_ENCRYPTION;
// values: true | false
extern PrefPtr PREF_BT_ENABLE_HOOK_AFTER_HASH_CHECK;
// values: true | false
extern PrefPtr PREF_BT_LOAD_SAVED_METADATA;

/**
 * Metalink related preferences
 */
// values: a string that your file system recognizes as a file name.
extern PrefPtr PREF_METALINK_FILE;
// values: a string
extern PrefPtr PREF_METALINK_VERSION;
// values: a string
extern PrefPtr PREF_METALINK_LANGUAGE;
// values: a string
extern PrefPtr PREF_METALINK_OS;
// values: a string
extern PrefPtr PREF_METALINK_LOCATION;
// values: true | false | mem
extern PrefPtr PREF_FOLLOW_METALINK;
// values: http | https | ftp | none
extern PrefPtr PREF_METALINK_PREFERRED_PROTOCOL;
// values: true | false
extern PrefPtr PREF_METALINK_ENABLE_UNIQUE_PROTOCOL;
// values: a string
extern PrefPtr PREF_METALINK_BASE_URI;

} // namespace aria2

#endif // D_PREFS_H
