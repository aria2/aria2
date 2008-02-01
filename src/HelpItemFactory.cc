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
#include "HelpItemFactory.h"
#include "TagContainer.h"
#include "HelpItem.h"
#include "usage_text.h"
#include "prefs.h"
#include "a2io.h"
#include "help_tags.h"

HelpItemFactory::HelpItemFactory() {}

TagContainerHandle HelpItemFactory::createHelpItems()
{
  TagContainerHandle tc = new TagContainer();
  {
    HelpItemHandle item = new HelpItem(PREF_DIR, TEXT_DIR);
    item->addTag(TAG_BASIC);
    tc->addItem(item);
  }
  {
    HelpItemHandle item = new HelpItem(PREF_OUT, TEXT_OUT);
    item->addTag(TAG_BASIC);
    item->addTag(TAG_HTTP);
    item->addTag(TAG_FTP);
    tc->addItem(item);
  }
#ifdef HAVE_DAEMON
  {
    HelpItemHandle item = new HelpItem(PREF_DAEMON, TEXT_DAEMON);
    item->addTag(TAG_ADVANCED);
    tc->addItem(item);
  }
#endif // HAVE_DAEMON
  {
    HelpItemHandle item = new HelpItem(PREF_SPLIT, TEXT_SPLIT);
    item->addTag(TAG_BASIC);
    item->addTag(TAG_HTTP);
    item->addTag(TAG_FTP);
    tc->addItem(item);
  }
  {
    HelpItemHandle item = new HelpItem(PREF_RETRY_WAIT, TEXT_RETRY_WAIT);
    item->addTag(TAG_HTTP);
    item->addTag(TAG_FTP);
    tc->addItem(item);
  }
  {
    HelpItemHandle item = new HelpItem(PREF_TIMEOUT, TEXT_TIMEOUT);
    item->addTag(TAG_HTTP);
    item->addTag(TAG_FTP);
    tc->addItem(item);
  }
  {
    HelpItemHandle item = new HelpItem(PREF_MAX_TRIES, TEXT_MAX_TRIES);
    item->addTag(TAG_HTTP);
    item->addTag(TAG_FTP);
    tc->addItem(item);
  }
  {
    HelpItemHandle item = new HelpItem(PREF_HTTP_PROXY, TEXT_HTTP_PROXY);
    item->addTag(TAG_HTTP);
    tc->addItem(item);
  }
  {
    HelpItemHandle item = new HelpItem(PREF_HTTP_USER, TEXT_HTTP_USER);
    item->addTag(TAG_BASIC);
    item->addTag(TAG_HTTP);
    tc->addItem(item);
  }
  {
    HelpItemHandle item = new HelpItem(PREF_HTTP_PASSWD, TEXT_HTTP_PASSWD);
    item->addTag(TAG_BASIC);
    item->addTag(TAG_HTTP);
    tc->addItem(item);
  }
  {
    HelpItemHandle item = new HelpItem(PREF_HTTP_PROXY_USER, TEXT_HTTP_PROXY_USER);
    item->addTag(TAG_HTTP);
    tc->addItem(item);
  }
  {
    HelpItemHandle item = new HelpItem(PREF_HTTP_PROXY_PASSWD, TEXT_HTTP_PROXY_PASSWD);
    item->addTag(TAG_HTTP);
    tc->addItem(item);
  }
  {
    HelpItemHandle item = new HelpItem(PREF_HTTP_PROXY_METHOD, TEXT_HTTP_PROXY_METHOD);
    item->addTag(TAG_HTTP);
    tc->addItem(item);
  }
  {
    HelpItemHandle item = new HelpItem(PREF_HTTP_AUTH_SCHEME, TEXT_HTTP_AUTH_SCHEME);
    item->addTag(TAG_HTTP);
    tc->addItem(item);
  }
  {
    HelpItemHandle item = new HelpItem(PREF_REFERER, TEXT_REFERER);
    item->addTag(TAG_HTTP);
    tc->addItem(item);
  }
  {
    HelpItemHandle item = new HelpItem(PREF_FTP_USER, TEXT_FTP_USER);
    item->addTag(TAG_BASIC);
    item->addTag(TAG_FTP);
    tc->addItem(item);
  }
  {
    HelpItemHandle item = new HelpItem(PREF_FTP_PASSWD, TEXT_FTP_PASSWD);
    item->addTag(TAG_BASIC);
    item->addTag(TAG_FTP);
    tc->addItem(item);
  }
  {
    HelpItemHandle item = new HelpItem(PREF_FTP_TYPE, TEXT_FTP_TYPE);
    item->addTag(TAG_FTP);
    tc->addItem(item);
  }
  {
    HelpItemHandle item = new HelpItem(PREF_FTP_PASV, TEXT_FTP_PASV);
    item->addTag(TAG_FTP);
    tc->addItem(item);
  }
  {
    HelpItemHandle item = new HelpItem(PREF_FTP_VIA_HTTP_PROXY, TEXT_FTP_VIA_HTTP_PROXY);
    item->addTag(TAG_FTP);
    tc->addItem(item);
  }
  {
    HelpItemHandle item = new HelpItem(PREF_LOWEST_SPEED_LIMIT, TEXT_LOWEST_SPEED_LIMIT);
    item->addTag(TAG_HTTP);
    item->addTag(TAG_FTP);
    tc->addItem(item);
  }
  {
    HelpItemHandle item = new HelpItem(PREF_MAX_DOWNLOAD_LIMIT, TEXT_MAX_DOWNLOAD_LIMIT);
    item->addTag(TAG_HTTP);
    item->addTag(TAG_FTP);
    item->addTag(TAG_BITTORRENT);
    tc->addItem(item);
  }
  {
    HelpItemHandle item = new HelpItem(PREF_FILE_ALLOCATION, TEXT_FILE_ALLOCATION);
    item->addTag(TAG_ADVANCED);
    tc->addItem(item);
  }
  {
    HelpItemHandle item = new HelpItem(PREF_NO_FILE_ALLOCATION_LIMIT, TEXT_NO_FILE_ALLOCATION_LIMIT, "5M");
    item->addTag(TAG_ADVANCED);
    tc->addItem(item);
  }
#ifdef ENABLE_DIRECT_IO
  {
    HelpItemHandle item = new HelpItem(PREF_ENABLE_DIRECT_IO, TEXT_ENABLE_DIRECT_IO, V_TRUE);
    item->addTag(TAG_ADVANCED);
    tc->addItem(item);
  }
#endif // ENABLE_DIRECT_IO
  {
    HelpItemHandle item = new HelpItem(PREF_ALLOW_OVERWRITE, TEXT_ALLOW_OVERWRITE);
    item->addTag(TAG_ADVANCED);
    tc->addItem(item);
  }
  {
    HelpItemHandle item = new HelpItem(PREF_ALLOW_PIECE_LENGTH_CHANGE, TEXT_ALLOW_PIECE_LENGTH_CHANGE, V_FALSE);
    item->addTag(TAG_ADVANCED);
    tc->addItem(item);
  }
  {
    HelpItemHandle item = new HelpItem(PREF_FORCE_SEQUENTIAL, TEXT_FORCE_SEQUENTIAL);
    item->addTag(TAG_ADVANCED);
    tc->addItem(item);
  }
  {
    HelpItemHandle item = new HelpItem(PREF_AUTO_FILE_RENAMING, TEXT_AUTO_FILE_RENAMING);
    item->addTag(TAG_ADVANCED);
    tc->addItem(item);
  }
  {
    HelpItemHandle item = new HelpItem(PREF_PARAMETERIZED_URI, TEXT_PARAMETERIZED_URI);
    item->addTag(TAG_ADVANCED);
    tc->addItem(item);
  }
  {
    HelpItemHandle item = new HelpItem(PREF_ENABLE_HTTP_KEEP_ALIVE, TEXT_ENABLE_HTTP_KEEP_ALIVE);
    item->addTag(TAG_HTTP);
    tc->addItem(item);
  }
  {
    HelpItemHandle item = new HelpItem(PREF_ENABLE_HTTP_PIPELINING, TEXT_ENABLE_HTTP_PIPELINING);
    item->addTag(TAG_HTTP);
    tc->addItem(item);
  }
#ifdef ENABLE_MESSAGE_DIGEST
  {
    HelpItemHandle item = new HelpItem(PREF_CHECK_INTEGRITY, TEXT_CHECK_INTEGRITY);
    item->addTag(TAG_BASIC);
    item->addTag(TAG_METALINK);
    item->addTag(TAG_BITTORRENT);
    tc->addItem(item);
  }
  {
    HelpItemHandle item = new HelpItem(PREF_REALTIME_CHUNK_CHECKSUM, TEXT_REALTIME_CHUNK_CHECKSUM);
    item->addTag(TAG_METALINK);
    tc->addItem(item);
  }
#endif // ENABLE_MESSAGE_DIGEST
  {
    HelpItemHandle item = new HelpItem(PREF_CONTINUE, TEXT_CONTINUE);
    item->addTag(TAG_BASIC);
    item->addTag(TAG_HTTP);
    item->addTag(TAG_FTP);
    tc->addItem(item);
  }
  {
    HelpItemHandle item = new HelpItem(PREF_USER_AGENT, TEXT_USER_AGENT);
    item->addTag(TAG_HTTP);
    tc->addItem(item);
  }
  {
    HelpItemHandle item = new HelpItem(PREF_NO_NETRC, TEXT_NO_NETRC);
    item->addTag(TAG_FTP);
    tc->addItem(item);
  }
  {
    HelpItemHandle item = new HelpItem(PREF_INPUT_FILE, TEXT_INPUT_FILE);
    item->addTag(TAG_BASIC);
    tc->addItem(item);
  }
  {
    HelpItemHandle item = new HelpItem(PREF_MAX_CONCURRENT_DOWNLOADS, TEXT_MAX_CONCURRENT_DOWNLOADS);
    item->addTag(TAG_ADVANCED);
    tc->addItem(item);
  }
  {
    HelpItemHandle item = new HelpItem(PREF_LOAD_COOKIES, TEXT_LOAD_COOKIES);
    item->addTag(TAG_BASIC);
    item->addTag(TAG_HTTP);
    tc->addItem(item);
  }
#if defined ENABLE_BITTORRENT || ENABLE_METALINK
  {
    HelpItemHandle item = new HelpItem(PREF_SHOW_FILES, TEXT_SHOW_FILES);
    item->addTag(TAG_BASIC);
    item->addTag(TAG_METALINK);
    item->addTag(TAG_BITTORRENT);
    tc->addItem(item);
  }
  {
    HelpItemHandle item = new HelpItem(PREF_SELECT_FILE, TEXT_SELECT_FILE);
    item->addTag(TAG_METALINK);
    item->addTag(TAG_BITTORRENT);
    tc->addItem(item);
  }
#endif // ENABLE_BITTORRENT || ENABLE_METALINK
#ifdef ENABLE_BITTORRENT
  {
    HelpItemHandle item = new HelpItem(PREF_TORRENT_FILE, TEXT_TORRENT_FILE);
    item->addTag(TAG_BASIC);
    item->addTag(TAG_BITTORRENT);
    tc->addItem(item);
  }
  {
    HelpItemHandle item = new HelpItem(PREF_FOLLOW_TORRENT, TEXT_FOLLOW_TORRENT, V_TRUE);
    item->addTag(TAG_BITTORRENT);
    tc->addItem(item);
  }
  {
    HelpItemHandle item = new HelpItem(PREF_DIRECT_FILE_MAPPING, TEXT_DIRECT_FILE_MAPPING);
    item->addTag(TAG_BITTORRENT);
    tc->addItem(item);
  }
  {
    HelpItemHandle item = new HelpItem(PREF_LISTEN_PORT, TEXT_LISTEN_PORT, "6881-6999");
    item->addTag(TAG_BASIC);
    item->addTag(TAG_BITTORRENT);
    tc->addItem(item);
  }
  {
    HelpItemHandle item = new HelpItem(PREF_MAX_UPLOAD_LIMIT, TEXT_MAX_UPLOAD_LIMIT);
    item->addTag(TAG_BASIC);
    item->addTag(TAG_BITTORRENT);
    tc->addItem(item);
  }
  {
    HelpItemHandle item = new HelpItem(PREF_SEED_TIME, TEXT_SEED_TIME);
    item->addTag(TAG_BITTORRENT);
    tc->addItem(item);
  }
  {
    HelpItemHandle item = new HelpItem(PREF_SEED_RATIO, TEXT_SEED_RATIO, "1.0");
    item->addTag(TAG_BITTORRENT);
    tc->addItem(item);
  }
  {
    HelpItemHandle item = new HelpItem(PREF_PEER_ID_PREFIX, TEXT_PEER_ID_PREFIX);
    item->addTag(TAG_BITTORRENT);
    tc->addItem(item);
  }
  {
    HelpItemHandle item = new HelpItem(PREF_ENABLE_PEER_EXCHANGE, TEXT_ENABLE_PEER_EXCHANGE, V_TRUE);
    item->addTag(TAG_BITTORRENT);
    tc->addItem(item);
  }
  {
    HelpItemHandle item = new HelpItem(PREF_ENABLE_DHT, TEXT_ENABLE_DHT, V_FALSE);
    item->addTag(TAG_BITTORRENT);
    tc->addItem(item);
  }
  {
    HelpItemHandle item = new HelpItem(PREF_DHT_LISTEN_PORT, TEXT_DHT_LISTEN_PORT, "6881-6999");
    item->addTag(TAG_BITTORRENT);
    tc->addItem(item);
  }
  {
    HelpItemHandle item = new HelpItem(PREF_DHT_ENTRY_POINT, TEXT_DHT_ENTRY_POINT);
    item->addTag(TAG_BITTORRENT);
    tc->addItem(item);
  }
#endif // ENABLE_BITTORRENT
#ifdef ENABLE_METALINK
  {
    HelpItemHandle item = new HelpItem(PREF_METALINK_FILE, TEXT_METALINK_FILE);
    item->addTag(TAG_BASIC);
    item->addTag(TAG_METALINK);
    tc->addItem(item);
  }
  {
    HelpItemHandle item = new HelpItem(PREF_METALINK_SERVERS, TEXT_METALINK_SERVERS);
    item->addTag(TAG_METALINK);
    tc->addItem(item);
  }
  {
    HelpItemHandle item = new HelpItem(PREF_METALINK_VERSION, TEXT_METALINK_VERSION);
    item->addTag(TAG_METALINK);
    tc->addItem(item);
  }
  {
    HelpItemHandle item = new HelpItem(PREF_METALINK_LANGUAGE, TEXT_METALINK_LANGUAGE);
    item->addTag(TAG_METALINK);
    tc->addItem(item);
  }
  {
    HelpItemHandle item = new HelpItem(PREF_METALINK_OS, TEXT_METALINK_OS);
    item->addTag(TAG_METALINK);
    tc->addItem(item);
  }
  {
    HelpItemHandle item = new HelpItem(PREF_METALINK_LOCATION, TEXT_METALINK_LOCATION);
    item->addTag(TAG_METALINK);
    tc->addItem(item);
  }
  {
    HelpItemHandle item = new HelpItem(PREF_METALINK_PREFERRED_PROTOCOL, TEXT_METALINK_PREFERRED_PROTOCOL, V_NONE);
    item->addTag(TAG_METALINK);
    tc->addItem(item);
  }
  {
    HelpItemHandle item = new HelpItem(PREF_FOLLOW_METALINK, TEXT_FOLLOW_METALINK, V_TRUE);
    item->addTag(TAG_METALINK);
    tc->addItem(item);
  }
  {
    HelpItemHandle item = new HelpItem(PREF_METALINK_ENABLE_UNIQUE_PROTOCOL, TEXT_METALINK_ENABLE_UNIQUE_PROTOCOL, V_TRUE);
    item->addTag(TAG_METALINK);
    tc->addItem(item);
  }
#endif // ENABLE_METALINK
  {
    HelpItemHandle item = new HelpItem("version", TEXT_VERSION);
    item->addTag(TAG_BASIC);
    tc->addItem(item);
  }
  {
    HelpItemHandle item = new HelpItem("no-conf", TEXT_NO_CONF);
    item->addTag(TAG_ADVANCED);
    tc->addItem(item);
  }
  {
    HelpItemHandle item = new HelpItem("conf-path", TEXT_CONF_PATH, "$HOME/.aria2/aria2.conf");
    item->addTag(TAG_ADVANCED);
    tc->addItem(item);
  }
  {
    HelpItemHandle item = new HelpItem("help", TEXT_HELP, TAG_BASIC);
    char buf[64];
    snprintf(buf, sizeof(buf), "%s,%s,%s,%s,%s,%s,all", TAG_BASIC, TAG_ADVANCED, TAG_HTTP, TAG_FTP, TAG_METALINK, TAG_BITTORRENT);
    item->setAvailableValues(buf);
    item->addTag(TAG_BASIC);
    tc->addItem(item);
  }
  return tc;
}
