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
#include "ExtensionMessageRegistry.h"

#include <cstring>
#include <cassert>

namespace aria2 {

ExtensionMessageRegistry::ExtensionMessageRegistry()
    : extensions_(MAX_EXTENSION)
{
}

ExtensionMessageRegistry::~ExtensionMessageRegistry() = default;

namespace {
const char* EXTENSION_NAMES[] = {"ut_metadata", "ut_pex", nullptr};
} // namespace

uint8_t ExtensionMessageRegistry::getExtensionMessageID(int key) const
{
  assert(key < MAX_EXTENSION);
  return extensions_[key];
}

const char* ExtensionMessageRegistry::getExtensionName(uint8_t id) const
{
  int i;
  if (id == 0) {
    return nullptr;
  }
  for (i = 0; i < MAX_EXTENSION; ++i) {
    if (extensions_[i] == id) {
      break;
    }
  }
  return EXTENSION_NAMES[i];
}

void ExtensionMessageRegistry::setExtensionMessageID(int key, uint8_t id)
{
  assert(key < MAX_EXTENSION);
  extensions_[key] = id;
}

void ExtensionMessageRegistry::removeExtension(int key)
{
  assert(key < MAX_EXTENSION);
  extensions_[key] = 0;
}

void ExtensionMessageRegistry::setExtensions(const Extensions& extensions)
{
  extensions_ = extensions;
}

const char* strBtExtension(int key)
{
  if (key >= ExtensionMessageRegistry::MAX_EXTENSION) {
    return nullptr;
  }
  else {
    return EXTENSION_NAMES[key];
  }
}

int keyBtExtension(const char* name)
{
  int i;
  for (i = 0; i < ExtensionMessageRegistry::MAX_EXTENSION; ++i) {
    if (strcmp(EXTENSION_NAMES[i], name) == 0) {
      break;
    }
  }
  return i;
}

} // namespace aria2
