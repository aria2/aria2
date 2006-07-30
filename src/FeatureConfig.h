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
#ifndef _D_FEATURE_CONFIG_H_
#define _D_FEATURE_CONFIG_H_

#include "common.h"
#include <map>

typedef map<string, int> ProtocolPortMap;
typedef map<string, bool> SupportedFeatureMap;

class FeatureConfig {
private:
  static ProtocolPortMap defaultPorts;
  static SupportedFeatureMap supportedFeatures;
public:
  static string FEATURE_HTTP;
  static string FEATURE_HTTPS;
  static string FEATURE_FTP;
  static string FEATURE_BITTORRENT;
  static string FEATURE_METALINK;

  static int getDefaultPort(const string& protocol) {
    if(defaultPorts.count(protocol)) {
      return defaultPorts[protocol];
    } else {
      return 0;
    }
  }

  static bool isSupported(const string& protocol) {
    if(supportedFeatures.count(protocol)) {
      return supportedFeatures[protocol];
    } else {
      return false;
    }
  }
  
  static string getConfigurationSummary() {
    string protos[] = {
      FEATURE_HTTP,
      FEATURE_HTTPS,
      FEATURE_FTP,
      FEATURE_BITTORRENT,
      FEATURE_METALINK
    };
    string summary;
    for(int i = 0; i < (int)(sizeof(protos)/sizeof(string)); i++) {
      summary += protos[i];
      if(isSupported(protos[i])) {
	summary += ": yes";
      } else {
	summary += ": no";
      }
      summary += "\n";
    }
    return summary;
  }
};

#endif // _D_FEATURE_CONFIG_H_
