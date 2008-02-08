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

namespace aria2 {

FeatureConfig* FeatureConfig::featureConfig = 0;

#define FEATURE_HTTP "http"
#define FEATURE_HTTPS "https"
#define FEATURE_FTP "ftp"
#define FEATURE_BITTORRENT "bittorrent"
#define FEATURE_METALINK "metalink"
#define FEATURE_MESSAGE_DIGEST "message digest"
#define FEATURE_ASYNC_DNS "async dns"

FeatureConfig::FeatureConfig() {
  static PortMap::value_type portArray[] = {
    PortMap::value_type("http", 80),
    PortMap::value_type("https", 443),
    PortMap::value_type("ftp", 21),
  };
  int32_t portArraySize = sizeof(portArray)/sizeof(PortMap::value_type);
  defaultPorts.insert(&portArray[0],
		      &portArray[portArraySize]);

  static FeatureMap::value_type featureArray[] = {
    FeatureMap::value_type(FEATURE_HTTP, true),
    FeatureMap::value_type(FEATURE_HTTPS,
#ifdef ENABLE_SSL
			   true
#else
			   false
#endif // ENABLE_SSL
			   ),
    FeatureMap::value_type(FEATURE_FTP, true),
    FeatureMap::value_type(FEATURE_BITTORRENT,
#ifdef ENABLE_BITTORRENT
			   true
#else
			   false
#endif // ENABLE_BITTORRENT
			   ),
    FeatureMap::value_type(FEATURE_METALINK,
#ifdef ENABLE_METALINK
			   true
#else
			   false
#endif // ENABLE_METALINK
			   ),
    FeatureMap::value_type(FEATURE_MESSAGE_DIGEST,
#ifdef ENABLE_MESSAGE_DIGEST
			   true
#else
			   false
#endif // ENABLE_MESSAGE_DIGEST
			   ),
    FeatureMap::value_type(FEATURE_ASYNC_DNS,
#ifdef ENABLE_ASYNC_DNS
			   true
#else
			   false
#endif // ENABLE_ASYNC_DNS
			   ),
  };

  int32_t featureArraySize = sizeof(featureArray)/sizeof(FeatureMap::value_type);
  supportedFeatures.insert(&featureArray[0],
			   &featureArray[featureArraySize]);

  for(int32_t i = 0; i < featureArraySize; i++) {
    features.push_back(featureArray[i].first);
  }
}

} // namespace aria2
