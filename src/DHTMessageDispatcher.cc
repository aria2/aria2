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
#include "DHTMessageDispatcher.h"
#include "DHTMessage.h"
#include "DHTMessageCallback.h"
#include "DHTMessageEntry.h"
#include "DHTMessageTracker.h"
#include "RecoverableException.h"
#include "LogFactory.h"
#include "DHTConstants.h"

DHTMessageDispatcher::DHTMessageDispatcher(const DHTMessageTrackerHandle& tracker):_tracker(tracker), _logger(LogFactory::getInstance()) {}

DHTMessageDispatcher::~DHTMessageDispatcher() {}

void DHTMessageDispatcher::addMessageToQueue(const DHTMessageHandle& message,
					     time_t timeout,
					     const DHTMessageCallbackHandle& callback)
{
  _messageQueue.push_back(new DHTMessageEntry(message, timeout, callback));
}

void DHTMessageDispatcher::addMessageToQueue(const DHTMessageHandle& message,
					     const DHTMessageCallbackHandle& callback)
{
  addMessageToQueue(message, DHT_MESSAGE_TIMEOUT, callback);
}

void DHTMessageDispatcher::sendMessage(const DHTMessageEntryHandle& entry)
{
  try {
    entry->_message->send();
    if(!entry->_message->isReply()) {
      _tracker->addMessage(entry->_message, entry->_timeout, entry->_callback);
    }
    _logger->info("Message sent: %s", entry->_message->toString().c_str());
  } catch(RecoverableException* e) {
    _logger->error("Failed to send message: %s", e, entry->_message->toString().c_str());
    delete e;
  }
}

void DHTMessageDispatcher::sendMessages()
{
  for(DHTMessageEntries::iterator itr = _messageQueue.begin(); itr != _messageQueue.end(); ++itr) {
    sendMessage(*itr);
  }
  _messageQueue.clear();
}

size_t DHTMessageDispatcher::countMessageInQueue() const
{
  return _messageQueue.size();
}
