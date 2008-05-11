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
#include "FeatureConfig.h"
#include "array_fun.h"
#include "Util.h"
#include <numeric>

namespace aria2 {

SharedHandle<FeatureConfig> FeatureConfig::_featureConfig;

const std::string FeatureConfig::FEATURE_HTTPS("HTTPS");
const std::string FeatureConfig::FEATURE_BITTORRENT("BitTorrent");
const std::string FeatureConfig::FEATURE_METALINK("Metalink");
const std::string FeatureConfig::FEATURE_MESSAGE_DIGEST("Message Digest");
const std::string FeatureConfig::FEATURE_ASYNC_DNS("Async DNS");

#ifdef ENABLE_SSL
# define HTTPS_ENABLED true
#else
# define HTTPS_ENABLED false
#endif // ENABLE_SSL

#ifdef ENABLE_BITTORRENT
# define BITTORRENT_ENABLED true
#else
# define BITTORRENT_ENABLED false
#endif // ENABLE_BITTORRENT

#ifdef ENABLE_METALINK
# define METALINK_ENABLED true
#else
# define METALINK_ENABLED false
#endif // ENABLE_METALINK

#ifdef ENABLE_MESSAGE_DIGEST
# define MESSAGE_DIGEST_ENABLED true
#else
# define MESSAGE_DIGEST_ENABLED false
#endif // ENABLE_MESSAGE_DIGEST

#ifdef ENABLE_ASYNC_DNS
# define ASYNC_DNS_ENABLED true
#else
# define ASYNC_DNS_ENABLED false
#endif // ENABLE_ASYNC_DNS

FeatureConfig::FeatureConfig() {
  _defaultPorts.insert(PortMap::value_type("http", 80));
  _defaultPorts.insert(PortMap::value_type("https", 443));
  _defaultPorts.insert(PortMap::value_type("ftp", 21));

  FeatureMap::value_type featureArray[] = {
    FeatureMap::value_type(FEATURE_HTTPS, HTTPS_ENABLED),
    FeatureMap::value_type(FEATURE_BITTORRENT, BITTORRENT_ENABLED),
    FeatureMap::value_type(FEATURE_METALINK, METALINK_ENABLED),
    FeatureMap::value_type(FEATURE_MESSAGE_DIGEST, MESSAGE_DIGEST_ENABLED),
    FeatureMap::value_type(FEATURE_ASYNC_DNS, ASYNC_DNS_ENABLED)
  };

  _features.insert(&featureArray[0], &featureArray[arrayLength(featureArray)]);
}

SharedHandle<FeatureConfig> FeatureConfig::getInstance()
{
  if(_featureConfig.isNull()) {
    _featureConfig.reset(new FeatureConfig());
  }
  return _featureConfig;
}

uint16_t FeatureConfig::getDefaultPort(const std::string& protocol) const
{
  PortMap::const_iterator itr = _defaultPorts.find(protocol);
  if(itr == _defaultPorts.end()) {
    return 0;
  } else {
    return itr->second;
  }
}

bool FeatureConfig::isSupported(const std::string& feature) const
{
  FeatureMap::const_iterator itr = _features.find(feature);
  if(itr == _features.end()) {
    return false;
  } else {
    return itr->second;
  }
}

std::string FeatureConfig::featureSummary() const
{
  std::string s;
  for(FeatureMap::const_iterator i = _features.begin();
      i != _features.end(); ++i) {
    if((*i).second) {
      s += (*i).first+", ";
    }
  }
  return Util::trim(s, ", ");
}

} // namespace aria2
