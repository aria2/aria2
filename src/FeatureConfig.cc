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
#include "FeatureConfig.h"

#include <numeric>

#include "array_fun.h"
#include "util.h"
#include "Request.h"

namespace aria2 {

SharedHandle<FeatureConfig> FeatureConfig::featureConfig_;

const std::string FeatureConfig::FEATURE_HTTPS("HTTPS");
const std::string FeatureConfig::FEATURE_BITTORRENT("BitTorrent");
const std::string FeatureConfig::FEATURE_METALINK("Metalink");
const std::string FeatureConfig::FEATURE_MESSAGE_DIGEST("Message Digest");
const std::string FeatureConfig::FEATURE_ASYNC_DNS("Async DNS");
const std::string FeatureConfig::FEATURE_XML_RPC("XML-RPC");
const std::string FeatureConfig::FEATURE_GZIP("GZip");
const std::string FeatureConfig::FEATURE_FIREFOX3_COOKIE("Firefox3 Cookie");

#ifdef ENABLE_SSL
# define HTTPS_ENABLED true
#else // !ENABLE_SSL
# define HTTPS_ENABLED false
#endif // !ENABLE_SSL

#ifdef ENABLE_BITTORRENT
# define BITTORRENT_ENABLED true
#else // !ENABLE_BITTORRENT
# define BITTORRENT_ENABLED false
#endif // !ENABLE_BITTORRENT

#ifdef ENABLE_METALINK
# define METALINK_ENABLED true
#else // !ENABLE_METALINK
# define METALINK_ENABLED false
#endif // !ENABLE_METALINK

#ifdef ENABLE_MESSAGE_DIGEST
# define MESSAGE_DIGEST_ENABLED true
#else // !ENABLE_MESSAGE_DIGEST
# define MESSAGE_DIGEST_ENABLED false
#endif // !ENABLE_MESSAGE_DIGEST

#ifdef ENABLE_ASYNC_DNS
# define ASYNC_DNS_ENABLED true
#else // !ENABLE_ASYNC_DNS
# define ASYNC_DNS_ENABLED false
#endif // !ENABLE_ASYNC_DNS

#ifdef ENABLE_XML_RPC
# define XML_RPC_ENABLED true
#else // !ENABLE_XML_RPC
# define XML_RPC_ENABLED false
#endif // !ENABLE_XML_RPC

#ifdef HAVE_ZLIB
# define GZIP_ENABLED true
#else // !HAVE_ZLIB
# define GZIP_ENABLED false
#endif // !HAVE_ZLIB

#ifdef HAVE_SQLITE3
# define FIREFOX3_COOKIE_ENABLED true
#else // !HAVE_SQLITE3
# define FIREFOX3_COOKIE_ENABLED false
#endif // !HAVE_SQLITE3

FeatureConfig::FeatureConfig() {
  defaultPorts_.insert(PortMap::value_type(Request::PROTO_HTTP, 80));
  defaultPorts_.insert(PortMap::value_type(Request::PROTO_HTTPS, 443));
  defaultPorts_.insert(PortMap::value_type(Request::PROTO_FTP, 21));

  FeatureMap::value_type featureArray[] = {
    FeatureMap::value_type(FEATURE_HTTPS, HTTPS_ENABLED),
    FeatureMap::value_type(FEATURE_BITTORRENT, BITTORRENT_ENABLED),
    FeatureMap::value_type(FEATURE_METALINK, METALINK_ENABLED),
    FeatureMap::value_type(FEATURE_MESSAGE_DIGEST, MESSAGE_DIGEST_ENABLED),
    FeatureMap::value_type(FEATURE_ASYNC_DNS, ASYNC_DNS_ENABLED),
    FeatureMap::value_type(FEATURE_XML_RPC, XML_RPC_ENABLED),
    FeatureMap::value_type(FEATURE_GZIP, GZIP_ENABLED),
    FeatureMap::value_type(FEATURE_FIREFOX3_COOKIE, FIREFOX3_COOKIE_ENABLED),
  };

  features_.insert(vbegin(featureArray), vend(featureArray));
}

FeatureConfig::~FeatureConfig() {}

const SharedHandle<FeatureConfig>& FeatureConfig::getInstance()
{
  if(!featureConfig_) {
    featureConfig_.reset(new FeatureConfig());
  }
  return featureConfig_;
}

uint16_t FeatureConfig::getDefaultPort(const std::string& protocol) const
{
  PortMap::const_iterator itr = defaultPorts_.find(protocol);
  if(itr == defaultPorts_.end()) {
    return 0;
  } else {
    return itr->second;
  }
}

bool FeatureConfig::isSupported(const std::string& feature) const
{
  FeatureMap::const_iterator itr = features_.find(feature);
  if(itr == features_.end()) {
    return false;
  } else {
    return itr->second;
  }
}

std::string FeatureConfig::featureSummary() const
{
  std::string s;
  for(FeatureMap::const_iterator i = features_.begin(), eoi = features_.end();
      i != eoi; ++i) {
    if((*i).second) {
      s += (*i).first;
      s += ", ";
    }
  }
  return util::strip(s, ", ");
}

} // namespace aria2
