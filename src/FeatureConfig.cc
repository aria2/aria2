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

string FeatureConfig::FEATURE_HTTP = "http";
string FeatureConfig::FEATURE_HTTPS = "https";
string FeatureConfig::FEATURE_FTP = "ftp";
string FeatureConfig::FEATURE_BITTORRENT = "bittorrent";
string FeatureConfig::FEATURE_METALINK = "metalink";

static ProtocolPortMap::value_type defaultPortsArray[] = {
  ProtocolPortMap::value_type(FeatureConfig::FEATURE_HTTP, 80),
  ProtocolPortMap::value_type(FeatureConfig::FEATURE_HTTPS, 443),
  ProtocolPortMap::value_type(FeatureConfig::FEATURE_FTP, 21),
};

ProtocolPortMap FeatureConfig::defaultPorts(&defaultPortsArray[0],
					     &defaultPortsArray[3]);

static SupportedFeatureMap::value_type supportedFeaturesArray[] = {
  SupportedFeatureMap::value_type(FeatureConfig::FEATURE_HTTP, true),
#ifdef ENABLE_SSL
  SupportedFeatureMap::value_type(FeatureConfig::FEATURE_HTTPS, true),
#endif // ENABLE_SSL
  SupportedFeatureMap::value_type(FeatureConfig::FEATURE_FTP, true),
#ifdef ENABLE_BITTORRENT
  SupportedFeatureMap::value_type(FeatureConfig::FEATURE_BITTORRENT, true),
#endif // ENABLE_BITTORRENT
#ifdef ENABLE_METALINK
  SupportedFeatureMap::value_type(FeatureConfig::FEATURE_METALINK, true),
#endif // ENABLE_METALINK
};

SupportedFeatureMap
FeatureConfig::supportedFeatures(&supportedFeaturesArray[0],
				 &supportedFeaturesArray[sizeof(supportedFeaturesArray)/sizeof(SupportedFeatureMap::value_type)]);

