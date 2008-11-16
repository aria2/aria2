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
#include "OptionHandlerFactory.h"
#include "prefs.h"
#include "OptionHandlerImpl.h"
#include "array_fun.h"
#include "usage_text.h"
#include "A2STR.h"
#include "Util.h"
#include "help_tags.h"
#include "StringFormat.h"

namespace aria2 {

OptionHandlers OptionHandlerFactory::createOptionHandlers()
{
  OptionHandlers handlers;
  // General Options
  {
    SharedHandle<OptionHandler> op(new BooleanOptionHandler
				   (PREF_ALLOW_OVERWRITE,
				    TEXT_ALLOW_OVERWRITE,
				    V_FALSE));
    op->addTag(TAG_ADVANCED);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new BooleanOptionHandler
				   (PREF_ALLOW_PIECE_LENGTH_CHANGE,
				    TEXT_ALLOW_PIECE_LENGTH_CHANGE,
				    V_FALSE));
    op->addTag(TAG_ADVANCED);
    handlers.push_back(op);
  } 
  {
    SharedHandle<OptionHandler> op(new BooleanOptionHandler
				   (PREF_ASYNC_DNS,
				    TEXT_ASYNC_DNS,
				    V_TRUE));
    op->addTag(TAG_ADVANCED);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new BooleanOptionHandler
				   (PREF_AUTO_FILE_RENAMING,
				    TEXT_AUTO_FILE_RENAMING,
				    V_TRUE));
    op->addTag(TAG_ADVANCED);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new NumberOptionHandler
				   (PREF_AUTO_SAVE_INTERVAL,
				    TEXT_AUTO_SAVE_INTERVAL,
				    "60",
				    0, 600));
    op->addTag(TAG_ADVANCED);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new BooleanOptionHandler
				   (PREF_CHECK_INTEGRITY,
				    TEXT_CHECK_INTEGRITY,
				    V_FALSE));
    op->addTag(TAG_BASIC);
    op->addTag(TAG_BITTORRENT);
    op->addTag(TAG_METALINK);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new DefaultOptionHandler
				   (PREF_CONF_PATH,
				    TEXT_CONF_PATH,
				    Util::getHomeDir()+"/.aria2/aria2.conf"));
    op->addTag(TAG_ADVANCED);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new BooleanOptionHandler
				   (PREF_CONTINUE,
				    TEXT_CONTINUE,
				    V_FALSE)); // TODO ommit?
    op->addTag(TAG_BASIC);
    op->addTag(TAG_FTP);
    op->addTag(TAG_HTTP);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new BooleanOptionHandler
				   (PREF_DAEMON,
				    TEXT_DAEMON,
				    V_FALSE)); // TODO ommit?
    op->addTag(TAG_ADVANCED);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new DefaultOptionHandler
				   (PREF_DIR,
				    TEXT_DIR,
				    "."));
    op->addTag(TAG_BASIC);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new NumberOptionHandler
				   (PREF_DNS_TIMEOUT,
				    NO_DESCRIPTION,
				    "30",
				    1, 60,
				    true));
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new BooleanOptionHandler
				   (PREF_ENABLE_DIRECT_IO,
				    TEXT_ENABLE_DIRECT_IO,
				    V_TRUE));
    op->addTag(TAG_ADVANCED);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new ParameterOptionHandler
				   (PREF_FILE_ALLOCATION,
				    TEXT_FILE_ALLOCATION,
				    V_PREALLOC,
				    V_NONE, V_PREALLOC));
    op->addTag(TAG_BASIC);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new BooleanOptionHandler
				   (PREF_FORCE_SEQUENTIAL,
				    TEXT_FORCE_SEQUENTIAL,
				    V_FALSE));
    op->addTag(TAG_BASIC);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new DefaultOptionHandler
				   (PREF_INPUT_FILE,
				    TEXT_INPUT_FILE,
				    NO_DEFAULT_VALUE,
				    "FILENAME,-"));
    op->addTag(TAG_BASIC);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new DefaultOptionHandler
				   (PREF_LOG,
				    TEXT_LOG,
				    NO_DEFAULT_VALUE,
				    "FILENAME,-"));
    op->addTag(TAG_BASIC);
    handlers.push_back(op);
  }
  {
    const std::string params[] = { V_DEBUG, V_INFO, V_NOTICE, V_WARN, V_ERROR };
    SharedHandle<OptionHandler> op(new ParameterOptionHandler
				   (PREF_LOG_LEVEL,
				    TEXT_LOG_LEVEL,
				    V_DEBUG,
				    std::deque<std::string>
				    (&params[0],
				     &params[arrayLength(params)])));
    op->addTag(TAG_ADVANCED);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new NumberOptionHandler
				   (PREF_MAX_CONCURRENT_DOWNLOADS,
				    TEXT_MAX_CONCURRENT_DOWNLOADS,
				    "5",
				    1, 45));
    op->addTag(TAG_BASIC);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new UnitNumberOptionHandler
				   (PREF_MAX_DOWNLOAD_LIMIT,
				    TEXT_MAX_DOWNLOAD_LIMIT,
				    "0",
				    0));
    op->addTag(TAG_BITTORRENT);
    op->addTag(TAG_FTP);
    op->addTag(TAG_HTTP);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new BooleanOptionHandler
				   (PREF_NO_CONF,
				    TEXT_NO_CONF,
				    V_FALSE));
    op->addTag(TAG_ADVANCED);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new UnitNumberOptionHandler
				   (PREF_NO_FILE_ALLOCATION_LIMIT,
				    TEXT_NO_FILE_ALLOCATION_LIMIT,
				    "5M",
				    0));
    op->addTag(TAG_ADVANCED);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new BooleanOptionHandler
				   (PREF_PARAMETERIZED_URI,
				    TEXT_PARAMETERIZED_URI,
				    V_FALSE));  
    op->addTag(TAG_ADVANCED);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new BooleanOptionHandler
				   (PREF_QUIET,
				    TEXT_QUIET,
				    V_FALSE));
    op->addTag(TAG_ADVANCED);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new BooleanOptionHandler
				   (PREF_REALTIME_CHUNK_CHECKSUM,
				    TEXT_REALTIME_CHUNK_CHECKSUM,
				    V_TRUE));
    op->addTag(TAG_METALINK);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new NumberOptionHandler
				   (PREF_STOP,
				    TEXT_STOP,
				    "0",
				    0, INT32_MAX));
    op->addTag(TAG_ADVANCED);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new NumberOptionHandler
				   (PREF_SUMMARY_INTERVAL,
				    TEXT_SUMMARY_INTERVAL,
				    "60",
				    0, INT32_MAX));
    op->addTag(TAG_ADVANCED);
    handlers.push_back(op);
  }
  // HTTP/FTP options
  {
    SharedHandle<OptionHandler> op(new NumberOptionHandler
				   (PREF_CONNECT_TIMEOUT,
				    TEXT_CONNECT_TIMEOUT,
				    "60",
				    1, 600));
    op->addTag(TAG_FTP);
    op->addTag(TAG_HTTP);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new UnitNumberOptionHandler
				   (PREF_LOWEST_SPEED_LIMIT,
				    TEXT_LOWEST_SPEED_LIMIT,
				    "0",
				    0));
    op->addTag(TAG_FTP);
    op->addTag(TAG_HTTP);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new NumberOptionHandler
				   (PREF_MAX_FILE_NOT_FOUND,
				    TEXT_MAX_FILE_NOT_FOUND,
				    "0",
				    0));
    op->addTag(TAG_FTP);
    op->addTag(TAG_HTTP);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new NumberOptionHandler
				   (PREF_MAX_TRIES,
				    TEXT_MAX_TRIES,
				    "5",
				    0));
    op->addTag(TAG_FTP);
    op->addTag(TAG_HTTP);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new DefaultOptionHandler
				   (PREF_OUT,
				    TEXT_OUT,
				    NO_DEFAULT_VALUE,
				    "FILENAME"));
    op->addTag(TAG_BASIC);
    op->addTag(TAG_FTP);
    op->addTag(TAG_HTTP);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new BooleanOptionHandler
				   (PREF_REMOTE_TIME,
				    TEXT_REMOTE_TIME,
				    V_FALSE));
    op->addTag(TAG_FTP);
    op->addTag(TAG_HTTP);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new NumberOptionHandler
				   (PREF_RETRY_WAIT,
				    TEXT_RETRY_WAIT,
				    "5",
				    0, 60));
    op->addTag(TAG_FTP);
    op->addTag(TAG_HTTP);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new UnitNumberOptionHandler
				   (PREF_SEGMENT_SIZE,
				    NO_DESCRIPTION,
				    "1M",
				    1024, -1,
				    true));
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new DefaultOptionHandler
				   (PREF_SERVER_STAT_IF,
				    TEXT_SERVER_STAT_IF,
				    NO_DEFAULT_VALUE,
				    "FILENAME"));
    op->addTag(TAG_FTP);
    op->addTag(TAG_HTTP);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new DefaultOptionHandler
				   (PREF_SERVER_STAT_OF,
				    TEXT_SERVER_STAT_OF,
				    NO_DEFAULT_VALUE,
				    "FILENAME"));
    op->addTag(TAG_FTP);
    op->addTag(TAG_HTTP);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new NumberOptionHandler
				   (PREF_SERVER_STAT_TIMEOUT,
				    TEXT_SERVER_STAT_TIMEOUT,
				    "86400",
				    0, INT32_MAX));
    op->addTag(TAG_FTP);
    op->addTag(TAG_HTTP);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new NumberOptionHandler
				   (PREF_SPLIT,
				    TEXT_SPLIT,
				    "5",
				    1, 16));
    op->addTag(TAG_BASIC);
    op->addTag(TAG_FTP);
    op->addTag(TAG_HTTP);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new NumberOptionHandler
				   (PREF_STARTUP_IDLE_TIME,
				    NO_DESCRIPTION,
				    "10",
				    1, 60,
				    true));
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new NumberOptionHandler
				   (PREF_TIMEOUT,
				    TEXT_TIMEOUT,
				    "60",
				    1, 600));
    op->addTag(TAG_FTP);
    op->addTag(TAG_HTTP);
    handlers.push_back(op);
  }
  {
    const std::string params[] = { V_INORDER, V_FEEDBACK };
    SharedHandle<OptionHandler> op(new ParameterOptionHandler
				   (PREF_URI_SELECTOR,
				    TEXT_URI_SELECTOR,
				    V_INORDER,
				    std::deque<std::string>
				    (&params[0], &params[arrayLength(params)])));
    op->addTag(TAG_FTP);
    op->addTag(TAG_HTTP);
    handlers.push_back(op);
  }
  // HTTP Specific Options
  {
    SharedHandle<OptionHandler> op(new DefaultOptionHandler
				   (PREF_CA_CERTIFICATE,
				    TEXT_CA_CERTIFICATE));
    op->addTag(TAG_HTTP);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new DefaultOptionHandler
				   (PREF_CERTIFICATE,
				    TEXT_CERTIFICATE));
    op->addTag(TAG_HTTP);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new BooleanOptionHandler
				   (PREF_CHECK_CERTIFICATE,
				    TEXT_CHECK_CERTIFICATE,
				    V_FALSE));
    op->addTag(TAG_HTTP);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new BooleanOptionHandler
				   (PREF_ENABLE_HTTP_KEEP_ALIVE,
				    TEXT_ENABLE_HTTP_KEEP_ALIVE,
				    V_TRUE));
    op->addTag(TAG_HTTP);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new BooleanOptionHandler
				   (PREF_ENABLE_HTTP_PIPELINING,
				    TEXT_ENABLE_HTTP_PIPELINING,
				    V_FALSE));
    op->addTag(TAG_HTTP);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new CumulativeOptionHandler
				   (PREF_HEADER,
				    TEXT_HEADER,
				    NO_DEFAULT_VALUE,
				    "\n"));
    op->addTag(TAG_HTTP);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new ParameterOptionHandler
				   (PREF_HTTP_AUTH_SCHEME,
				    TEXT_HTTP_AUTH_SCHEME,
				    V_BASIC,
				    V_BASIC));
    op->addTag(TAG_HTTP);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new DefaultOptionHandler
				   (PREF_HTTP_PASSWD,
				    TEXT_HTTP_PASSWD));
    op->addTag(TAG_BASIC);
    op->addTag(TAG_HTTP);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new DefaultOptionHandler
				   (PREF_HTTP_USER,
				    TEXT_HTTP_USER));
    op->addTag(TAG_BASIC);
    op->addTag(TAG_HTTP);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new DefaultOptionHandler
				   (PREF_LOAD_COOKIES,
				    TEXT_LOAD_COOKIES,
				    NO_DEFAULT_VALUE,
				    "FILENAME"));
    op->addTag(TAG_BASIC);
    op->addTag(TAG_HTTP);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new NumberOptionHandler
				   (PREF_MAX_HTTP_PIPELINING,
				    NO_DESCRIPTION,
				    "2",
				    1, 8,
				    true));
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new DefaultOptionHandler
				   (PREF_PRIVATE_KEY,
				    TEXT_PRIVATE_KEY));
    op->addTag(TAG_HTTP);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new DefaultOptionHandler
				   (PREF_REFERER,
				    TEXT_REFERER));
    op->addTag(TAG_HTTP);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new DefaultOptionHandler
				   (PREF_USER_AGENT,
				    TEXT_USER_AGENT,
				    "aria2"));
    op->addTag(TAG_HTTP);
    handlers.push_back(op);
  }
  // FTP Specific Options
  {
    SharedHandle<OptionHandler> op(new DefaultOptionHandler
				   (PREF_FTP_PASSWD,
				    TEXT_FTP_PASSWD));
    op->addTag(TAG_BASIC);
    op->addTag(TAG_FTP);
    handlers.push_back(op);
  }    
  {
    SharedHandle<OptionHandler> op(new BooleanOptionHandler
				   (PREF_FTP_PASV,
				    TEXT_FTP_PASV,
				    V_TRUE));
    op->addTag(TAG_FTP);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new BooleanOptionHandler
				   (PREF_FTP_REUSE_CONNECTION,
				    TEXT_FTP_REUSE_CONNECTION,
				    V_TRUE));
    op->addTag(TAG_FTP);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new ParameterOptionHandler
				   (PREF_FTP_TYPE,
				    TEXT_FTP_TYPE,
				    V_BINARY,
				    V_BINARY, V_ASCII));
    op->addTag(TAG_FTP);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new DefaultOptionHandler
				   (PREF_FTP_USER,
				    TEXT_FTP_USER));
    op->addTag(TAG_BASIC);
    op->addTag(TAG_FTP);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new DefaultOptionHandler
				   (PREF_NETRC_PATH,
				    NO_DESCRIPTION,
				    Util::getHomeDir()+"/.netrc",
				    "/PATH/TO/NETRC",
				    true));
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new BooleanOptionHandler
				   (PREF_NO_NETRC,
				    TEXT_NO_NETRC,
				    V_FALSE)); // TODO ommit?
    op->addTag(TAG_FTP);
    op->addTag(TAG_HTTP);
    handlers.push_back(op);
  }
  // Proxy options
  {
    SharedHandle<OptionHandler> op(new HttpProxyOptionHandler
				   (PREF_HTTP_PROXY,
				    TEXT_HTTP_PROXY,
				    NO_DEFAULT_VALUE));
    op->addTag(TAG_HTTP);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new HttpProxyOptionHandler
				   (PREF_HTTPS_PROXY,
				    TEXT_HTTPS_PROXY,
				    NO_DEFAULT_VALUE));
    op->addTag(TAG_HTTP);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new HttpProxyOptionHandler
				   (PREF_FTP_PROXY,
				    TEXT_FTP_PROXY,
				    NO_DEFAULT_VALUE));
    op->addTag(TAG_FTP);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new HttpProxyOptionHandler
				   (PREF_ALL_PROXY,
				    TEXT_ALL_PROXY,
				    NO_DEFAULT_VALUE));
    op->addTag(TAG_FTP);
    op->addTag(TAG_HTTP);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new DefaultOptionHandler
				   (PREF_NO_PROXY,
				    TEXT_NO_PROXY));
    op->addTag(TAG_FTP);
    op->addTag(TAG_HTTP);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new ParameterOptionHandler
				   (PREF_PROXY_METHOD,
				    TEXT_PROXY_METHOD,
				    V_TUNNEL,
				    V_GET, V_TUNNEL));
    op->addTag(TAG_FTP);
    op->addTag(TAG_HTTP);
    handlers.push_back(op);
  }
  // BitTorrent/Metalink Options
  {
    SharedHandle<OptionHandler> op(new IntegerRangeOptionHandler
				   (PREF_SELECT_FILE,
				    TEXT_SELECT_FILE,
				    NO_DEFAULT_VALUE,
				    1, INT32_MAX));
    op->addTag(TAG_BITTORRENT);
    op->addTag(TAG_METALINK);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new BooleanOptionHandler
				   (PREF_SHOW_FILES,
				    TEXT_SHOW_FILES,
				    V_FALSE)); // TODO ommit?
    op->addTag(TAG_BASIC);
    op->addTag(TAG_BITTORRENT);
    op->addTag(TAG_METALINK);
    handlers.push_back(op);
  }
  // BitTorrent Specific Options
  {
    SharedHandle<OptionHandler> op(new NumberOptionHandler
				   (PREF_BT_KEEP_ALIVE_INTERVAL,
				    NO_DESCRIPTION,
				    "120",
				    1, 120,
				    true));
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new NumberOptionHandler
				   (PREF_BT_MAX_OPEN_FILES,
				    TEXT_BT_MAX_OPEN_FILES,
				    "100",
				    1));
    op->addTag(TAG_BITTORRENT);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new ParameterOptionHandler
				   (PREF_BT_MIN_CRYPTO_LEVEL,
				    TEXT_BT_MIN_CRYPTO_LEVEL,
				    V_PLAIN,
				    V_PLAIN, V_ARC4));
    op->addTag(TAG_BITTORRENT);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new UnitNumberOptionHandler
				   (PREF_BT_REQUEST_PEER_SPEED_LIMIT,
				    TEXT_BT_REQUEST_PEER_SPEED_LIMIT,
				    "50K",
				    0));
    op->addTag(TAG_BITTORRENT);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new BooleanOptionHandler
				   (PREF_BT_REQUIRE_CRYPTO,
				    TEXT_BT_REQUIRE_CRYPTO,
				    V_FALSE));
    op->addTag(TAG_BITTORRENT);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new NumberOptionHandler
				   (PREF_BT_REQUEST_TIMEOUT,
				    NO_DESCRIPTION,
				    "60",
				    1, 600,
				    true));
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new BooleanOptionHandler
				   (PREF_BT_SEED_UNVERIFIED,
				    TEXT_BT_SEED_UNVERIFIED,
				    V_FALSE));
    op->addTag(TAG_BITTORRENT);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new NumberOptionHandler
				   (PREF_BT_TIMEOUT,
				    NO_DESCRIPTION,
				    "180",
				    1, 600,
				    true));
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new BooleanOptionHandler
				   (PREF_DIRECT_FILE_MAPPING,
				    TEXT_DIRECT_FILE_MAPPING,
				    V_TRUE));
    op->addTag(TAG_BITTORRENT);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new HostPortOptionHandler
				   (PREF_DHT_ENTRY_POINT,
				    TEXT_DHT_ENTRY_POINT,
				    NO_DEFAULT_VALUE,
				    PREF_DHT_ENTRY_POINT_HOST,
				    PREF_DHT_ENTRY_POINT_PORT));
    op->addTag(TAG_BITTORRENT);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new DefaultOptionHandler
				   (PREF_DHT_FILE_PATH,
				    TEXT_DHT_FILE_PATH,
				    Util::getHomeDir()+"/.aria2/dht.dat",
				    "/PATH/TO/DHT_DAT"));
    op->addTag(TAG_BITTORRENT);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new IntegerRangeOptionHandler
				   (PREF_DHT_LISTEN_PORT,
				    TEXT_DHT_LISTEN_PORT,
				    "6881-6999",
				    1024, UINT16_MAX));
    op->addTag(TAG_BASIC);
    op->addTag(TAG_BITTORRENT);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new BooleanOptionHandler
				   (PREF_ENABLE_DHT,
				    TEXT_ENABLE_DHT,
				    V_FALSE));
    op->addTag(TAG_BASIC);
    op->addTag(TAG_BITTORRENT);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new BooleanOptionHandler
				   (PREF_ENABLE_PEER_EXCHANGE,
				    TEXT_ENABLE_PEER_EXCHANGE,
				    V_TRUE));
    op->addTag(TAG_BITTORRENT);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new ParameterOptionHandler
				   (PREF_FOLLOW_TORRENT,
				    TEXT_FOLLOW_TORRENT,
				    V_TRUE,
				    V_TRUE, V_MEM, V_FALSE));
    op->addTag(TAG_BITTORRENT);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new IntegerRangeOptionHandler
				   (PREF_LISTEN_PORT,
				    TEXT_LISTEN_PORT,
				    "6881-6999",
				    1024, UINT16_MAX));
    op->addTag(TAG_BASIC);
    op->addTag(TAG_BITTORRENT);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new UnitNumberOptionHandler
				   (PREF_MAX_UPLOAD_LIMIT,
				    TEXT_MAX_UPLOAD_LIMIT,
				    "0",
				    0));
    op->addTag(TAG_BASIC);
    op->addTag(TAG_BITTORRENT);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new NumberOptionHandler
				   (PREF_PEER_CONNECTION_TIMEOUT,
				    NO_DESCRIPTION,
				    "20",
				    1, 600,
				    true));
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new DefaultOptionHandler
				   (PREF_PEER_ID_PREFIX,
				    TEXT_PEER_ID_PREFIX,
				    "-aria2-"));
    op->addTag(TAG_BITTORRENT);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new NumberOptionHandler
				   (PREF_SEED_TIME,
				    TEXT_SEED_TIME,
				    NO_DEFAULT_VALUE,
				    0));
    op->addTag(TAG_BITTORRENT);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new FloatNumberOptionHandler
				   (PREF_SEED_RATIO,
				    TEXT_SEED_RATIO,
				    "1.0",
				    0.0));
    op->addTag(TAG_BITTORRENT);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new DefaultOptionHandler
				   (PREF_TORRENT_FILE,
				    TEXT_TORRENT_FILE));
    op->addTag(TAG_BASIC);
    op->addTag(TAG_BITTORRENT);
    handlers.push_back(op);
  }
  // Metalink Specific Options
  {
    SharedHandle<OptionHandler> op(new ParameterOptionHandler
				   (PREF_FOLLOW_METALINK,
				    TEXT_FOLLOW_METALINK,
				    V_TRUE,
				    V_TRUE, V_MEM, V_FALSE));
    op->addTag(TAG_METALINK);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new BooleanOptionHandler
				   (PREF_METALINK_ENABLE_UNIQUE_PROTOCOL,
				    TEXT_METALINK_ENABLE_UNIQUE_PROTOCOL,
				    V_TRUE));
    op->addTag(TAG_METALINK);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new DefaultOptionHandler
				   (PREF_METALINK_FILE,
				    TEXT_METALINK_FILE));
    op->addTag(TAG_BASIC);
    op->addTag(TAG_METALINK);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new DefaultOptionHandler
				   (PREF_METALINK_LANGUAGE,
				    TEXT_METALINK_LANGUAGE));
    op->addTag(TAG_METALINK);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new DefaultOptionHandler
				   (PREF_METALINK_LOCATION,
				    TEXT_METALINK_LOCATION));
    op->addTag(TAG_METALINK);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new DefaultOptionHandler
				   (PREF_METALINK_OS,
				    TEXT_METALINK_OS));
    op->addTag(TAG_METALINK);
    handlers.push_back(op);
  }
  {
    const std::string params[] = { V_HTTP, V_HTTPS, V_FTP, V_NONE };
    SharedHandle<OptionHandler> op(new ParameterOptionHandler
				   (PREF_METALINK_PREFERRED_PROTOCOL,
				    TEXT_METALINK_PREFERRED_PROTOCOL,
				    V_NONE,
				    std::deque<std::string>
				    (&params[0], &params[arrayLength(params)])));
    op->addTag(TAG_METALINK);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new NumberOptionHandler
				   (PREF_METALINK_SERVERS,
				    TEXT_METALINK_SERVERS,
				    "5",
				    1));
    op->addTag(TAG_METALINK);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new DefaultOptionHandler
				   (PREF_METALINK_VERSION,
				    TEXT_METALINK_VERSION));
    op->addTag(TAG_METALINK);
    handlers.push_back(op);
  }
  // Help Option
  {
    SharedHandle<OptionHandler> op(new DefaultOptionHandler
				   ("help",
				    TEXT_HELP,
				    TAG_BASIC,
				    StringFormat("%s,%s,%s,%s,%s,%s,%s,all",
						 TAG_BASIC,
						 TAG_ADVANCED,
						 TAG_HTTP,
						 TAG_FTP,
						 TAG_METALINK,
						 TAG_BITTORRENT,
						 TAG_HELP).str()));
    op->addTag(TAG_BASIC);
    op->addTag(TAG_HELP);
    handlers.push_back(op);
  }
  
  return handlers;
}

} // namespace aria2
