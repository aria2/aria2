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
#include "AbstractBtMessage.h"
#include "Peer.h"
#include "BtContext.h"
#include "PieceStorage.h"
#include "BtRegistry.h"
#include "BtEventListener.h"
#include "BtMessageValidator.h"
#include "LogFactory.h"
#include "Logger.h"

namespace aria2 {

AbstractBtMessage::AbstractBtMessage():sendingInProgress(false),
				       invalidate(false),
				       uploading(false),
				       cuid(0),
				       logger(LogFactory::getInstance())
{}

AbstractBtMessage::~AbstractBtMessage() {}

SharedHandle<Peer> AbstractBtMessage::getPeer() const
{
  return peer;
}

void AbstractBtMessage::setPeer(const SharedHandle<Peer>& peer)
{
  this->peer = peer;
}

bool AbstractBtMessage::validate(std::deque<std::string>& errors)
{
  if(validator.get()) {
    return validator->validate(errors);
  } else {
    return true;
  }
}

void AbstractBtMessage::handleEvent(const SharedHandle<BtEvent>& event)
{
  for(std::deque<SharedHandle<BtEventListener> >::iterator itr = listeners.begin();
      itr != listeners.end(); ++itr) {
    (*itr)->handleEvent(event);
  }
}

void
AbstractBtMessage::addEventListener(const SharedHandle<BtEventListener>& listener)
{
  listeners.push_back(listener);
}

void
AbstractBtMessage::setBtMessageValidator(const SharedHandle<BtMessageValidator>& validator) {
  this->validator = validator;
}

SharedHandle<BtMessageValidator> AbstractBtMessage::getBtMessageValidator() const
{
  return validator;
}

void AbstractBtMessage::setBtContext(const SharedHandle<BtContext>& btContext)
{
  this->btContext = btContext;
  this->pieceStorage = PIECE_STORAGE(btContext);
}

SharedHandle<BtContext> AbstractBtMessage::getBtContext() const
{
  return btContext;
}

void AbstractBtMessage::setBtMessageDispatcher(const WeakHandle<BtMessageDispatcher>& dispatcher)
{
  this->dispatcher = dispatcher;
}

void AbstractBtMessage::setPeerConnection(const WeakHandle<PeerConnection>& peerConnection)
{
  this->peerConnection = peerConnection;
}

void AbstractBtMessage::setBtMessageFactory(const WeakHandle<BtMessageFactory>& factory)
{
  this->messageFactory = factory;
}

void AbstractBtMessage::setBtRequestFactory(const WeakHandle<BtRequestFactory>& factory)
{
  this->requestFactory = factory;
}

} // namespace aria2
