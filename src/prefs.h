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

namespace option {

// Returns the number of options.
size_t countOption();

// Returns Pref whose ID is id. id must be less than countOption().
const Pref* i2p(size_t id);

// Returns Pref whose keyword is k. If no such Pref is found, returns
// special null Pref whose ID is 0.
const Pref* k2p(const std::string& k);

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
extern const std::string V_FEEDBACK;
extern const std::string V_ADAPTIVE;
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

extern const Pref* PREF_VERSION;
extern const Pref* PREF_HELP;

/**
 * General preferences
 */
// values: 1*digit
extern const Pref* PREF_TIMEOUT;
// values: 1*digit
extern const Pref* PREF_DNS_TIMEOUT;
// values: 1*digit
extern const Pref* PREF_CONNECT_TIMEOUT;
// values: 1*digit
extern const Pref* PREF_MAX_TRIES;
// values: 1*digit
extern const Pref* PREF_AUTO_SAVE_INTERVAL;
// values: a string that your file system recognizes as a file name.
extern const Pref* PREF_LOG;
// values: a string that your file system recognizes as a directory.
extern const Pref* PREF_DIR;
// values: a string that your file system recognizes as a file name.
extern const Pref* PREF_OUT;
// values: 1*digit
extern const Pref* PREF_SPLIT;
// value: true | false
extern const Pref* PREF_DAEMON;
// value: a string
extern const Pref* PREF_REFERER;
// value: 1*digit
extern const Pref* PREF_LOWEST_SPEED_LIMIT;
// value: 1*digit
extern const Pref* PREF_PIECE_LENGTH;
// value: 1*digit
extern const Pref* PREF_MAX_DOWNLOAD_LIMIT;
// value: 1*digit
extern const Pref* PREF_STARTUP_IDLE_TIME;
// value: prealloc | falloc | none
extern const Pref* PREF_FILE_ALLOCATION;
// value: 1*digit
extern const Pref* PREF_NO_FILE_ALLOCATION_LIMIT;
// value: true | false
extern const Pref* PREF_ALLOW_OVERWRITE;
// value: true | false
extern const Pref* PREF_REALTIME_CHUNK_CHECKSUM;
// value: true | false
extern const Pref* PREF_CHECK_INTEGRITY;
// value: string that your file system recognizes as a file name.
extern const Pref* PREF_NETRC_PATH;
// value:
extern const Pref* PREF_CONTINUE;
// value:
extern const Pref* PREF_NO_NETRC;
// value: 1*digit
extern const Pref* PREF_MAX_OVERALL_DOWNLOAD_LIMIT;
// value: 1*digit
extern const Pref* PREF_MAX_DOWNLOADS;
// value: string that your file system recognizes as a file name.
extern const Pref* PREF_INPUT_FILE;
// value: true | false
extern const Pref* PREF_DEFERRED_INPUT;
// value: 1*digit
extern const Pref* PREF_MAX_CONCURRENT_DOWNLOADS;
// value: true | false
extern const Pref* PREF_FORCE_SEQUENTIAL;
// value: true | false
extern const Pref* PREF_AUTO_FILE_RENAMING;
// value: true | false
extern const Pref* PREF_PARAMETERIZED_URI;
// value: true | false
extern const Pref* PREF_ENABLE_DIRECT_IO;
// value: true | false
extern const Pref* PREF_ALLOW_PIECE_LENGTH_CHANGE;
// value: true | false
extern const Pref* PREF_NO_CONF;
// value: string
extern const Pref* PREF_CONF_PATH;
// value: 1*digit
extern const Pref* PREF_STOP;
// value: true | false
extern const Pref* PREF_QUIET;
// value: true | false
extern const Pref* PREF_ASYNC_DNS;
// value: 1*digit
extern const Pref* PREF_SUMMARY_INTERVAL;
// value: debug, info, notice, warn, error
extern const Pref* PREF_LOG_LEVEL;
// value: inorder | feedback | adaptive
extern const Pref* PREF_URI_SELECTOR;
// value: 1*digit
extern const Pref* PREF_SERVER_STAT_TIMEOUT;
// value: string that your file system recognizes as a file name.
extern const Pref* PREF_SERVER_STAT_IF;
// value: string that your file system recognizes as a file name.
extern const Pref* PREF_SERVER_STAT_OF;
// value: true | false
extern const Pref* PREF_REMOTE_TIME;
// value: 1*digit
extern const Pref* PREF_MAX_FILE_NOT_FOUND;
// value: epoll | select
extern const Pref* PREF_EVENT_POLL;
// value: true | false
extern const Pref* PREF_ENABLE_RPC;
// value: 1*digit
extern const Pref* PREF_RPC_LISTEN_PORT;
// value: string
extern const Pref* PREF_RPC_USER;
// value: string
extern const Pref* PREF_RPC_PASSWD;
// value: 1*digit
extern const Pref* PREF_RPC_MAX_REQUEST_SIZE;
// value: true | false
extern const Pref* PREF_RPC_LISTEN_ALL;
// value: true | false
extern const Pref* PREF_RPC_ALLOW_ORIGIN_ALL;
// value: true | false
extern const Pref* PREF_DRY_RUN;
// value: true | false
extern const Pref* PREF_REUSE_URI;
// value: string
extern const Pref* PREF_ON_DOWNLOAD_START;
extern const Pref* PREF_ON_DOWNLOAD_PAUSE;
extern const Pref* PREF_ON_DOWNLOAD_STOP;
extern const Pref* PREF_ON_DOWNLOAD_COMPLETE;
extern const Pref* PREF_ON_DOWNLOAD_ERROR;
// value: string
extern const Pref* PREF_INTERFACE;
// value: true | false
extern const Pref* PREF_DISABLE_IPV6;
// value: true | false
extern const Pref* PREF_HUMAN_READABLE;
// value: true | false
extern const Pref* PREF_REMOVE_CONTROL_FILE;
// value: true | false
extern const Pref* PREF_ALWAYS_RESUME;
// value: 1*digit
extern const Pref* PREF_MAX_RESUME_FAILURE_TRIES;
// value: string that your file system recognizes as a file name.
extern const Pref* PREF_SAVE_SESSION;
// value: 1*digit
extern const Pref* PREF_MAX_CONNECTION_PER_SERVER;
// value: 1*digit
extern const Pref* PREF_MIN_SPLIT_SIZE;
// value: true | false
extern const Pref* PREF_CONDITIONAL_GET;
// value: true | false
extern const Pref* PREF_SELECT_LEAST_USED_HOST;
// value: true | false
extern const Pref* PREF_ENABLE_ASYNC_DNS6;
// value: 1*digit
extern const Pref* PREF_MAX_DOWNLOAD_RESULT;
// value: 1*digit
extern const Pref* PREF_RETRY_WAIT;
// value: string
extern const Pref* PREF_ASYNC_DNS_SERVER;
// value: true | false
extern const Pref* PREF_SHOW_CONSOLE_READOUT;
// value: default | inorder | geom
extern const Pref* PREF_STREAM_PIECE_SELECTOR;
// value: true | false
extern const Pref* PREF_TRUNCATE_CONSOLE_READOUT;
// value: true | false
extern const Pref* PREF_PAUSE;
// value: default | full
extern const Pref* PREF_DOWNLOAD_RESULT;
// value: true | false
extern const Pref* PREF_HASH_CHECK_ONLY;
// values: hashType=digest
extern const Pref* PREF_CHECKSUM;
// value: pid
extern const Pref* PREF_STOP_WITH_PROCESS;
// value: true | false
extern const Pref* PREF_ENABLE_MMAP;

/**
 * FTP related preferences
 */
extern const Pref* PREF_FTP_USER;
extern const Pref* PREF_FTP_PASSWD;
// values: binary | ascii
extern const Pref* PREF_FTP_TYPE;
// values: true | false
extern const Pref* PREF_FTP_PASV;
// values: true | false
extern const Pref* PREF_FTP_REUSE_CONNECTION;

/**
 * HTTP related preferences
 */
extern const Pref* PREF_HTTP_USER;
extern const Pref* PREF_HTTP_PASSWD;
// values: string
extern const Pref* PREF_USER_AGENT;
// value: string that your file system recognizes as a file name.
extern const Pref* PREF_LOAD_COOKIES;
// value: string that your file system recognizes as a file name.
extern const Pref* PREF_SAVE_COOKIES;
// values: true | false
extern const Pref* PREF_ENABLE_HTTP_KEEP_ALIVE;
// values: true | false
extern const Pref* PREF_ENABLE_HTTP_PIPELINING;
// value: 1*digit
extern const Pref* PREF_MAX_HTTP_PIPELINING;
// value: string
extern const Pref* PREF_HEADER;
// value: string that your file system recognizes as a file name.
extern const Pref* PREF_CERTIFICATE;
// value: string that your file system recognizes as a file name.
extern const Pref* PREF_PRIVATE_KEY;
// value: string that your file system recognizes as a file name.
extern const Pref* PREF_CA_CERTIFICATE;
// value: true | false
extern const Pref* PREF_CHECK_CERTIFICATE;
// value: true | false
extern const Pref* PREF_USE_HEAD;
// value: true | false
extern const Pref* PREF_HTTP_AUTH_CHALLENGE;
// value: true | false
extern const Pref* PREF_HTTP_NO_CACHE;
// value: true | false
extern const Pref* PREF_HTTP_ACCEPT_GZIP;

/**;
 * Proxy related preferences
 */
extern const Pref* PREF_HTTP_PROXY;
extern const Pref* PREF_HTTPS_PROXY;
extern const Pref* PREF_FTP_PROXY;
extern const Pref* PREF_ALL_PROXY;
// values: comma separeted hostname or domain
extern const Pref* PREF_NO_PROXY;
// values: get | tunnel
extern const Pref* PREF_PROXY_METHOD;
extern const Pref* PREF_HTTP_PROXY_USER;
extern const Pref* PREF_HTTP_PROXY_PASSWD;
extern const Pref* PREF_HTTPS_PROXY_USER;
extern const Pref* PREF_HTTPS_PROXY_PASSWD;
extern const Pref* PREF_FTP_PROXY_USER;
extern const Pref* PREF_FTP_PROXY_PASSWD;
extern const Pref* PREF_ALL_PROXY_USER;
extern const Pref* PREF_ALL_PROXY_PASSWD;

/**
 * BitTorrent related preferences
 */
// values: 1*digit
extern const Pref* PREF_PEER_CONNECTION_TIMEOUT;
// values: 1*digit
extern const Pref* PREF_BT_TIMEOUT;
// values: 1*digit
extern const Pref* PREF_BT_REQUEST_TIMEOUT;
// values: true | false
extern const Pref* PREF_SHOW_FILES;
// values: 1*digit
extern const Pref* PREF_MAX_OVERALL_UPLOAD_LIMIT;
// values: 1*digit
extern const Pref* PREF_MAX_UPLOAD_LIMIT;
// values: a string that your file system recognizes as a file name.
extern const Pref* PREF_TORRENT_FILE;
// values: 1*digit
extern const Pref* PREF_LISTEN_PORT;
// values: true | false | mem
extern const Pref* PREF_FOLLOW_TORRENT;
// values: 1*digit *( (,|-) 1*digit)
extern const Pref* PREF_SELECT_FILE;
// values: 1*digit
extern const Pref* PREF_SEED_TIME;
// values: 1*digit ['.' [ 1*digit ] ]
extern const Pref* PREF_SEED_RATIO;
// values: 1*digit
extern const Pref* PREF_BT_KEEP_ALIVE_INTERVAL;
// values: a string, less than or equals to 20 bytes length
extern const Pref* PREF_PEER_ID_PREFIX;
// values: true | false
extern const Pref* PREF_ENABLE_PEER_EXCHANGE;
// values: true | false
extern const Pref* PREF_ENABLE_DHT;
// values: a string
extern const Pref* PREF_DHT_LISTEN_ADDR;
// values: 1*digit
extern const Pref* PREF_DHT_LISTEN_PORT;
// values: a string
extern const Pref* PREF_DHT_ENTRY_POINT_HOST;
// values: 1*digit
extern const Pref* PREF_DHT_ENTRY_POINT_PORT;
// values: a string (hostname:port)
extern const Pref* PREF_DHT_ENTRY_POINT;
// values: a string
extern const Pref* PREF_DHT_FILE_PATH;
// values: true | false
extern const Pref* PREF_ENABLE_DHT6;
// values: a string
extern const Pref* PREF_DHT_LISTEN_ADDR6;
// values: a string
extern const Pref* PREF_DHT_ENTRY_POINT_HOST6;
// values: 1*digit
extern const Pref* PREF_DHT_ENTRY_POINT_PORT6;
// values: a string (hostname:port)
extern const Pref* PREF_DHT_ENTRY_POINT6;
// values: a string
extern const Pref* PREF_DHT_FILE_PATH6;
// values: plain | arc4
extern const Pref* PREF_BT_MIN_CRYPTO_LEVEL;
// values:: true | false
extern const Pref* PREF_BT_REQUIRE_CRYPTO;
// values: 1*digit
extern const Pref* PREF_BT_REQUEST_PEER_SPEED_LIMIT;
// values: 1*digit
extern const Pref* PREF_BT_MAX_OPEN_FILES;
// values: true | false
extern const Pref* PREF_BT_SEED_UNVERIFIED;
// values: true | false
extern const Pref* PREF_BT_HASH_CHECK_SEED;
// values: 1*digit
extern const Pref* PREF_BT_MAX_PEERS;
// values: a string (IP address)
extern const Pref* PREF_BT_EXTERNAL_IP;
// values: 1*digit '=' a string that your file system recognizes as a file name.
extern const Pref* PREF_INDEX_OUT;
// values: 1*digit
extern const Pref* PREF_BT_TRACKER_INTERVAL;
// values: 1*digit
extern const Pref* PREF_BT_STOP_TIMEOUT;
// values: head[=SIZE]|tail[=SIZE], ...
extern const Pref* PREF_BT_PRIORITIZE_PIECE;
// values: true | false
extern const Pref* PREF_BT_SAVE_METADATA;
// values: true | false
extern const Pref* PREF_BT_METADATA_ONLY;
// values: true | false
extern const Pref* PREF_BT_ENABLE_LPD;
// values: string
extern const Pref* PREF_BT_LPD_INTERFACE;
// values: 1*digit
extern const Pref* PREF_BT_TRACKER_TIMEOUT;
// values: 1*digit
extern const Pref* PREF_BT_TRACKER_CONNECT_TIMEOUT;
// values: 1*digit
extern const Pref* PREF_DHT_MESSAGE_TIMEOUT;
// values: string
extern const Pref* PREF_ON_BT_DOWNLOAD_COMPLETE;
// values: string
extern const Pref* PREF_BT_TRACKER;
// values: string
extern const Pref* PREF_BT_EXCLUDE_TRACKER;
// values: true | false
extern const Pref* PREF_BT_REMOVE_UNSELECTED_FILE;

/**
 * Metalink related preferences
 */
// values: a string that your file system recognizes as a file name.
extern const Pref* PREF_METALINK_FILE;
// values: a string
extern const Pref* PREF_METALINK_VERSION;
// values: a string
extern const Pref* PREF_METALINK_LANGUAGE;
// values: a string
extern const Pref* PREF_METALINK_OS;
// values: a string
extern const Pref* PREF_METALINK_LOCATION;
// values: 1*digit
extern const Pref* PREF_METALINK_SERVERS;
// values: true | false | mem
extern const Pref* PREF_FOLLOW_METALINK;
// values: http | https | ftp | none
extern const Pref* PREF_METALINK_PREFERRED_PROTOCOL;
// values: true | false
extern const Pref* PREF_METALINK_ENABLE_UNIQUE_PROTOCOL;
// values: a string
extern const Pref* PREF_METALINK_BASE_URI;

} // namespace aria2

#endif // D_PREFS_H
