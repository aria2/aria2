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

#define SH(X) SharedHandle<OptionHandler>(X)

namespace aria2 {

OptionHandlers OptionHandlerFactory::createOptionHandlers()
{
  OptionHandlers handlers;


  handlers.push_back(SH(new HttpProxyOptionHandler(PREF_HTTP_PROXY,
						   PREF_HTTP_PROXY_HOST,
						   PREF_HTTP_PROXY_PORT)));
  handlers.push_back(SH(new DefaultOptionHandler(PREF_HTTP_USER)));
  handlers.push_back(SH(new DefaultOptionHandler(PREF_HTTP_PASSWD)));
  handlers.push_back(SH(new DefaultOptionHandler(PREF_HTTP_PROXY_USER)));
  handlers.push_back(SH(new DefaultOptionHandler(PREF_HTTP_PROXY_PASSWD)));
  handlers.push_back(SH(new ParameterOptionHandler(PREF_HTTP_AUTH_SCHEME, V_BASIC)));
  handlers.push_back(SH(new DefaultOptionHandler(PREF_REFERER)));
  handlers.push_back(SH(new NumberOptionHandler(PREF_RETRY_WAIT, 0, 60)));
  handlers.push_back(SH(new DefaultOptionHandler(PREF_FTP_USER)));
  handlers.push_back(SH(new DefaultOptionHandler(PREF_FTP_PASSWD)));
  handlers.push_back(SH(new ParameterOptionHandler(PREF_FTP_TYPE, V_BINARY, V_ASCII)));
  handlers.push_back(SH(new ParameterOptionHandler(PREF_FTP_VIA_HTTP_PROXY,
						V_GET, V_TUNNEL)));
  handlers.push_back(SH(new UnitNumberOptionHandler(PREF_MIN_SEGMENT_SIZE, 1024)));
  handlers.push_back(SH(new ParameterOptionHandler(PREF_HTTP_PROXY_METHOD,
						V_GET, V_TUNNEL)));
  handlers.push_back(SH(new IntegerRangeOptionHandler(PREF_LISTEN_PORT, 1024, UINT16_MAX)));
  handlers.push_back(SH(new ParameterOptionHandler(PREF_FOLLOW_TORRENT, V_TRUE, V_MEM, V_FALSE)));
  handlers.push_back(SH(new BooleanOptionHandler(PREF_NO_PREALLOCATION)));
  handlers.push_back(SH(new BooleanOptionHandler(PREF_DIRECT_FILE_MAPPING)));
  handlers.push_back(SH(new IntegerRangeOptionHandler(PREF_SELECT_FILE, 1, INT32_MAX)));
  handlers.push_back(SH(new NumberOptionHandler(PREF_SEED_TIME, 0)));
  handlers.push_back(SH(new FloatNumberOptionHandler(PREF_SEED_RATIO, 0.0)));
  handlers.push_back(SH(new UnitNumberOptionHandler(PREF_MAX_UPLOAD_LIMIT, 0)));
  handlers.push_back(SH(new DefaultOptionHandler(PREF_METALINK_VERSION)));
  handlers.push_back(SH(new DefaultOptionHandler(PREF_METALINK_LANGUAGE)));
  handlers.push_back(SH(new DefaultOptionHandler(PREF_METALINK_OS)));
  handlers.push_back(SH(new ParameterOptionHandler(PREF_FOLLOW_METALINK, V_TRUE, V_MEM, V_FALSE)));
  handlers.push_back(SH(new DefaultOptionHandler(PREF_METALINK_LOCATION)));
  handlers.push_back(SH(new UnitNumberOptionHandler(PREF_LOWEST_SPEED_LIMIT, 0)));
  handlers.push_back(SH(new UnitNumberOptionHandler(PREF_MAX_DOWNLOAD_LIMIT, 0)));
  handlers.push_back(SH(new BooleanOptionHandler(PREF_ALLOW_OVERWRITE)));
  handlers.push_back(SH(new BooleanOptionHandler(PREF_CHECK_INTEGRITY)));
  handlers.push_back(SH(new BooleanOptionHandler(PREF_REALTIME_CHUNK_CHECKSUM)));
  handlers.push_back(SH(new BooleanOptionHandler(PREF_DAEMON)));
  handlers.push_back(SH(new DefaultOptionHandler(PREF_DIR)));
  handlers.push_back(SH(new DefaultOptionHandler(PREF_OUT)));
  handlers.push_back(SH(new LogOptionHandler(PREF_LOG)));
  handlers.push_back(SH(new NumberOptionHandler(PREF_SPLIT, 1, 16)));
  handlers.push_back(SH(new NumberOptionHandler(PREF_TIMEOUT, 1, 600)));
  handlers.push_back(SH(new NumberOptionHandler(PREF_MAX_TRIES, 0)));
  handlers.push_back(SH(new BooleanOptionHandler(PREF_FTP_PASV)));
  handlers.push_back(SH(new BooleanOptionHandler(PREF_SHOW_FILES)));
  handlers.push_back(SH(new DefaultOptionHandler(PREF_TORRENT_FILE)));
  handlers.push_back(SH(new DefaultOptionHandler(PREF_METALINK_FILE)));
  handlers.push_back(SH(new NumberOptionHandler(PREF_METALINK_SERVERS, 1)));
  handlers.push_back(SH(new ParameterOptionHandler(PREF_FILE_ALLOCATION,
						V_NONE, V_PREALLOC)));
  handlers.push_back(SH(new BooleanOptionHandler(PREF_CONTINUE)));
  handlers.push_back(SH(new DefaultOptionHandler(PREF_USER_AGENT)));
  handlers.push_back(SH(new BooleanOptionHandler(PREF_NO_NETRC)));
  handlers.push_back(SH(new DefaultOptionHandler(PREF_INPUT_FILE)));
  handlers.push_back(SH(new NumberOptionHandler(PREF_MAX_CONCURRENT_DOWNLOADS, 1, 45)));
  handlers.push_back(SH(new DefaultOptionHandler(PREF_LOAD_COOKIES)));
  handlers.push_back(SH(new DefaultOptionHandler(PREF_PEER_ID_PREFIX)));
  handlers.push_back(SH(new BooleanOptionHandler(PREF_FORCE_SEQUENTIAL)));  
  handlers.push_back(SH(new BooleanOptionHandler(PREF_AUTO_FILE_RENAMING)));  
  handlers.push_back(SH(new BooleanOptionHandler(PREF_PARAMETERIZED_URI)));  
  handlers.push_back(SH(new BooleanOptionHandler(PREF_ENABLE_HTTP_KEEP_ALIVE)));
  handlers.push_back(SH(new BooleanOptionHandler(PREF_ENABLE_HTTP_PIPELINING)));
  handlers.push_back(SH(new UnitNumberOptionHandler(PREF_NO_FILE_ALLOCATION_LIMIT, 0)));
  handlers.push_back(SH(new BooleanOptionHandler(PREF_ENABLE_DIRECT_IO)));
  handlers.push_back(SH(new BooleanOptionHandler(PREF_ALLOW_PIECE_LENGTH_CHANGE)));
  {
    const std::string params[] = { V_HTTP, V_HTTPS, V_FTP, V_NONE };
    handlers.push_back(SH(new ParameterOptionHandler(PREF_METALINK_PREFERRED_PROTOCOL,
						     std::deque<std::string>(&params[0], &params[arrayLength(params)]))));
  }
  handlers.push_back(SH(new BooleanOptionHandler(PREF_METALINK_ENABLE_UNIQUE_PROTOCOL)));
  handlers.push_back(SH(new BooleanOptionHandler(PREF_ENABLE_PEER_EXCHANGE)));
  handlers.push_back(SH(new BooleanOptionHandler(PREF_ENABLE_DHT)));
  handlers.push_back(SH(new IntegerRangeOptionHandler(PREF_DHT_LISTEN_PORT, 1024, UINT16_MAX)));
  handlers.push_back(SH(new HostPortOptionHandler(PREF_DHT_ENTRY_POINT,
					       PREF_DHT_ENTRY_POINT_HOST,
					       PREF_DHT_ENTRY_POINT_PORT)));
  handlers.push_back(SH(new NumberOptionHandler(PREF_STOP, 0, INT32_MAX)));
  handlers.push_back(SH(new ParameterOptionHandler(PREF_BT_MIN_CRYPTO_LEVEL, V_PLAIN, V_ARC4)));
  handlers.push_back(SH(new BooleanOptionHandler(PREF_BT_REQUIRE_CRYPTO)));
  handlers.push_back(SH(new NumberOptionHandler(PREF_BT_REQUEST_PEER_SPEED_LIMIT, 0)));
  handlers.push_back(SH(new NumberOptionHandler(PREF_BT_MAX_OPEN_FILES, 1)));
  handlers.push_back(SH(new CumulativeOptionHandler(PREF_HEADER, "\n")));
  handlers.push_back(SH(new BooleanOptionHandler(PREF_QUIET)));
#ifdef ENABLE_ASYNC_DNS
  handlers.push_back(SH(new BooleanOptionHandler(PREF_ASYNC_DNS)));
#endif // ENABLE_ASYNC_DNS
  handlers.push_back(SH(new BooleanOptionHandler(PREF_FTP_REUSE_CONNECTION)));
  handlers.push_back(SH(new NumberOptionHandler(PREF_SUMMARY_INTERVAL, 0, INT32_MAX)));
  {
    const std::string params[] = { V_DEBUG, V_INFO, V_NOTICE, V_WARN, V_ERROR };
    handlers.push_back(SH(new ParameterOptionHandler
			  (PREF_LOG_LEVEL,
			   std::deque<std::string>(&params[0],
						   &params[arrayLength(params)]))));
  }
  return handlers;
}

} // namespace aria2
