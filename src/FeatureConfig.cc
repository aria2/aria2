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
#include "FeatureConfig.h"

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
  int portArraySize = sizeof(portArray)/sizeof(PortMap::value_type);
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
#ifdef HAVE_LIBARES
			   true
#else
			   false
#endif // HAVE_LIBARES
			   ),
  };

  int featureArraySize = sizeof(featureArray)/sizeof(FeatureMap::value_type);
  supportedFeatures.insert(&featureArray[0],
			   &featureArray[featureArraySize]);

  for(int i = 0; i < featureArraySize; i++) {
    features.push_back(featureArray[i].first);
  }
}
