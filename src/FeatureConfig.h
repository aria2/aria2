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

typedef map<string, int> PortMap;
typedef map<string, bool> FeatureMap;

class FeatureConfig {
private:
  static FeatureConfig* featureConfig;

  PortMap defaultPorts;
  FeatureMap supportedFeatures;
  Strings features;

  FeatureConfig();
  ~FeatureConfig() {}
public:
  static FeatureConfig* getInstance() {
    if(!featureConfig) {
      featureConfig = new FeatureConfig();
    }
    return featureConfig;
  }

  static void release() {
    delete featureConfig;
    featureConfig = 0;
  }

  int getDefaultPort(const string& protocol) const {
    PortMap::const_iterator itr = defaultPorts.find(protocol);
    if(itr == defaultPorts.end()) {
      return 0;
    } else {
      return itr->second;
    }
  }

  bool isSupported(const string& feature) const {
    FeatureMap::const_iterator itr = supportedFeatures.find(feature);
    if(itr == supportedFeatures.end()) {
      return false;
    } else {
      return itr->second;
    }
  }

  const Strings& getFeatures() const {
    return features;
  }

  string getConfigurationSummary() const {
    string summary;
    for(Strings::const_iterator itr = features.begin();
	itr != features.end(); itr++) {
      summary += *itr;
      if(isSupported(*itr)) {
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
