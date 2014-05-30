/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2012 Tatsuhiro Tsujikawa
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
#ifndef D_FEATURE_CONFIG_H
#define D_FEATURE_CONFIG_H

#include "common.h"

#include <string>

namespace aria2 {

// Returns default port for the given |protocol|.
uint16_t getDefaultPort(const std::string& protocol);

enum FeatureType {
  FEATURE_ASYNC_DNS,
  FEATURE_BITTORRENT,
  FEATURE_FF3_COOKIE,
  FEATURE_GZIP,
  FEATURE_HTTPS,
  FEATURE_MESSAGE_DIGEST,
  FEATURE_METALINK,
  FEATURE_XML_RPC,
  MAX_FEATURE
};

// Returns summary string of the available features.
std::string featureSummary();

// Returns the string representation of the given |feature| if it is
// available in the build. If it is not available, returns NULL.
const char* strSupportedFeature(int feature);

// Returns summary string of 3rd party libraries directly used by
// aria2.
std::string usedLibs();

// Returns a summary string of the used compiler/platform.
std::string usedCompilerAndPlatform();

// Returns the system information about the OS this binary is running on.
std::string getOperatingSystemInfo();

} // namespace aria2

#endif // D_FEATURE_CONFIG_H
