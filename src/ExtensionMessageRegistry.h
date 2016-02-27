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
#ifndef D_EXTENSION_MESSAGE_REGISTRY_H
#define D_EXTENSION_MESSAGE_REGISTRY_H

#include "common.h"

#include <vector>

namespace aria2 {

typedef std::vector<int> Extensions;

// This class stores mapping between BitTorrent extension name and its
// ID. The BitTorrent Extension Protocol is specified in BEP10.  This
// class is defined to only stores extensions aria2 supports. See
// InterestingExtension for supported extensions.
//
// See also http://bittorrent.org/beps/bep_0010.html
class ExtensionMessageRegistry {
public:
  enum InterestingExtension {
    UT_METADATA,
    UT_PEX,
    // The number of extensions.
    MAX_EXTENSION
  };

  ExtensionMessageRegistry();

  ~ExtensionMessageRegistry();

  const Extensions& getExtensions() const { return extensions_; }

  void setExtensions(const Extensions& extensions);

  // Returns message ID corresponding the given |key|.  The |key| must
  // be one of InterestingExtension other than MAX_EXTENSION. If
  // message ID is not defined, returns 0.
  uint8_t getExtensionMessageID(int key) const;

  // Returns extension name corresponding to the given |id|. If no
  // extension is defined for the given |id|, returns NULL.
  const char* getExtensionName(uint8_t id) const;

  // Sets association of the |key| and |id|. The |key| must be one of
  // InterestingExtension other than MAX_EXTENSION.
  void setExtensionMessageID(int key, uint8_t id);

  // Removes association of the |key|. The |key| must be one of
  // InterestingExtension other than MAX_EXTENSION. After this call,
  // getExtensionMessageID(key) returns 0.
  void removeExtension(int key);

private:
  Extensions extensions_;
};

// Returns the extension name corresponding to the given |key|. The
// |key| must be one of InterestingExtension other than MAX_EXTENSION.
const char* strBtExtension(int key);

// Returns extension key corresponding to the given extension |name|.
// If no such key exists, returns
// ExtensionMessageRegistry::MAX_EXTENSION.
int keyBtExtension(const char* name);

} // namespace aria2

#endif // D_EXTENSION_MESSAGE_REGISTRY_H
