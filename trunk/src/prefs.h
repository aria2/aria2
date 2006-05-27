/* <!-- copyright */
/*
 * aria2 - a simple utility for downloading files faster
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
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
#define PREF_MAX_TRIES "max_try"
// values: 1*digit
#define PREF_MIN_SEGMENT_SIZE "min_segment_size"
// values: 1*digit
#define PREF_AUTO_SAVE_INTERVAL "auto_save_interval"

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
#define PREF_UPLOAD_LIMIT "upload_limit"

#endif // _D_PREFS_H_
