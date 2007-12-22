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
#ifndef _D_BT_RUNTIME_H_
#define _D_BT_RUNTIME_H_

#include "common.h"
#include "BtConstants.h"

#define MIN_PEERS 40

class BtRuntime {
private:
  int64_t uploadLengthAtStartup;
  int32_t port;
  bool halt;
  int32_t connections;
  bool _ready;

  Extensions _extensions;
public:
  BtRuntime():
    uploadLengthAtStartup(0),
    port(0),
    halt(false),
    connections(0),
    _ready(false)
  {
    _extensions["ut_pex"] = 8;
  }
  ~BtRuntime() {}

  int64_t getUploadLengthAtStartup() const {
    return uploadLengthAtStartup;
  }

  void setUploadLengthAtStartup(int64_t length) {
    this->uploadLengthAtStartup = length;
  }

  void setListenPort(int32_t port) {
    this->port = port;
  }

  int32_t getListenPort() const { return port; }

  bool isHalt() const { return halt; }

  void setHalt(bool halt) {
    this->halt = halt;
  }

  int32_t getConnections() const { return connections; }

  void increaseConnections() { connections++; }

  void decreaseConnections() { connections--; }

  bool lessThanMinPeer() const { return connections < MIN_PEERS; }

  bool lessThanEqMinPeer() const { return connections <= MIN_PEERS; }

  bool ready() { return _ready; }

  void setReady(bool go) { _ready = go; }

  const Extensions& getExtensions() const
  {
    return _extensions;
  }

  uint8_t getExtensionMessageID(const string& name)
  {
    Extensions::const_iterator itr = _extensions.find(name);
    if(itr == _extensions.end()) {
      return 0;
    } else {
      return (*itr).second;
    }
  }

  string getExtensionName(uint8_t id)
  {
    for(Extensions::const_iterator itr = _extensions.begin();
      itr != _extensions.end(); ++itr) {
      const Extensions::value_type& p = *itr;
      if(p.second == id) {
	return p.first;
      }
    }
    return "";
  }
};

typedef SharedHandle<BtRuntime> BtRuntimeHandle;

#endif // _D_BT_RUNTIME_H_
