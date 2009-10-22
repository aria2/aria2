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
#ifndef _D_BT_HANDSHAKE_MESSAGE_VALIDATOR_H_
#define _D_BT_HANDSHAKE_MESSAGE_VALIDATOR_H_

#include "BtMessageValidator.h"

#include <cstring>

#include "BtHandshakeMessage.h"
#include "Util.h"
#include "StringFormat.h"

namespace aria2 {

class BtHandshakeMessageValidator : public BtMessageValidator {
private:
  const BtHandshakeMessage* message;
  unsigned char infoHash[20];
public:
  BtHandshakeMessageValidator(const BtHandshakeMessage* message,
			      const unsigned char* infoHash):
    message(message)
  {
    memcpy(this->infoHash, infoHash, sizeof(this->infoHash));
  }

  virtual bool validate(Errors& error) {
    // TODO
    if(message->getPstrlen() != 19) {
      throw DL_ABORT_EX(StringFormat("invalid handshake pstrlen=%u",
				   message->getPstrlen()).str());
    }
    if(memcmp(BtHandshakeMessage::BT_PSTR, message->getPstr(), 19) != 0) {
      throw DL_ABORT_EX
	(StringFormat("invalid handshake pstr=%s",
		      util::urlencode(message->getPstr(), 19).c_str()).str());
    }
    if(memcmp(infoHash, message->getInfoHash(), 20) != 0) {
      throw DL_ABORT_EX
	(StringFormat("invalid handshake info hash: expected:%s, actual:%s",
		      util::toHex(infoHash, 20).c_str(),
		      util::toHex(message->getInfoHash(), 20).c_str()).str());
    }
    return true;
  }
};

typedef SharedHandle<BtHandshakeMessageValidator> BtHandshakeMessageValidatorHandle;

} // namespace aria2

#endif // _D_BT_HANDSHAKE_MESSAGE_VALIDATOR_H_
