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
#define V_TRUE "true"
#define V_FALSE "false"

/**
 * General preferences
 */
// values: 1*digit
#define PREF_RETRY_WAIT "retry_wait"
// values: 1*digit
#define PREF_TIMEOUT "timeout"
// values: 1*digit
#define PREF_DNS_TIMEOUT "dns_timeout"
// values: 1*digit
#define PREF_MAX_TRIES "max_tries"
// values: 1*digit
#define PREF_MIN_SEGMENT_SIZE "min_segment_size"
// values: 1*digit
#define PREF_AUTO_SAVE_INTERVAL "auto_save_interval"
// values: true | false
#define PREF_STDOUT_LOG "stdout_log"
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
#define PREF_LOWEST_SPEED_LIMIT "lowest_speed_limit"
// value: 1*digit
#define PREF_SEGMENT_SIZE "segment_size"
// value: 1*digit
#define PREF_MAX_DOWNLOAD_LIMIT "max_download_limit"
// value: 1*digit
#define PREF_STARTUP_IDLE_TIME "startup_idle_time"

/**
 * FTP related preferences
 */
#define PREF_FTP_USER "ftp_user"
#define PREF_FTP_PASSWD "ftp_passwd"
// values: binary | ascii
#define PREF_FTP_TYPE "ftp_type"
#  define V_BINARY "binary"
#  define V_ASCII "ascii"
// values: get | tunnel
#define PREF_FTP_VIA_HTTP_PROXY "ftp_via_http_proxy"
#  define V_GET "get"
#  define V_TUNNEL "tunnel"
// values: true | false
#define PREF_FTP_PASV_ENABLED "ftp_pasv_enabled"

/**
 * HTTP related preferences
 */
#define PREF_HTTP_USER "http_user"
#define PREF_HTTP_PASSWD "http_passwd"
// values: basic
#define PREF_HTTP_AUTH_SCHEME "http_auth_scheme"
#  define V_BASIC "basic"
// values: true | false
#define PREF_HTTP_AUTH_ENABLED "http_auth_enabled"
// values: true | false
#define PREF_HTTP_KEEP_ALIVE "http_keep_alive"

/** 
 * HTTP proxy related preferences
 */
#define PREF_HTTP_PROXY_USER "http_proxy_user"
#define PREF_HTTP_PROXY_PASSWD "http_proxy_passwd"
#define PREF_HTTP_PROXY_HOST "http_proxy_host"
#define PREF_HTTP_PROXY_PORT "http_proxy_port"
// values: get | tunnel
#define PREF_HTTP_PROXY_METHOD "http_proxy_method"
// values: true | false
#define PREF_HTTP_PROXY_ENABLED "http_proxy_enabled"
// values: true | false
#define PREF_HTTP_PROXY_AUTH_ENABLED "http_proxy_auth_enabled"

/**
 * BitTorrent related preferences
 */
// values: 1*digit
#define PREF_PEER_CONNECTION_TIMEOUT "peer_connection_timeout"
// values: true | false
#define PREF_SHOW_FILES "show_files"
// values: true | false
#define PREF_NO_PREALLOCATION "no_preallocation"
// values: true | false
#define PREF_DIRECT_FILE_MAPPING "direct_file_mapping"
// values: 1*digit
#define PREF_MAX_UPLOAD_LIMIT "max_upload_limit"
// values: a string that your file system recognizes as a file name.
#define PREF_TORRENT_FILE "torrent_file"
// values: 1*digit
#define PREF_LISTEN_PORT "listen_port"
// values: true | false
#define PREF_FOLLOW_TORRENT "follow_torrent"
// values: 1*digit *( (,|-) 1*digit)
#define PREF_SELECT_FILE "select_file"
// values: 1*digit
#define PREF_SEED_TIME "seed_time"
// values: 1*digit ['.' [ 1*digit ] ]
#define PREF_SEED_RATIO "seed_ratio"
// values: 1*digit
#define PREF_TRACKER_MAX_TRIES "tracker_max_tries"

/**
 * Metalink related preferences
 */
// values: a string that your file system recognizes as a file name.
#define PREF_METALINK_FILE "metalink_file"
// values: a string
#define PREF_METALINK_VERSION "metalink_version"
// values: a string
#define PREF_METALINK_LANGUAGE "metalink_language"
// values: a string
#define PREF_METALINK_OS "metalink_os"
// values: a string
#define PREF_METALINK_LOCATION "metalink_location"
// values: 1*digit
#define PREF_METALINK_SERVERS "metalink_servers"
// values: true | false
#define PREF_FOLLOW_METALINK "follow_metalink"

#endif // _D_PREFS_H_
