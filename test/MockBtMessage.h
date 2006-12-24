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
#ifndef _D_MOCK_BT_MESSAGE_H_
#define _D_MOCK_BT_MESSAGE_H_

#include "BtMessage.h"

class MockBtMessage : public BtMessage {
private:
  bool sendingInProgress;
  bool invalidate;
  bool uploading;
  int32_t id;
public:
  MockBtMessage() {}

  virtual ~MockBtMessage() {}

  virtual bool isSendingInProgress() {
    return sendingInProgress;
  }

  void setSendingInProgress(bool flag) {
    this->sendingInProgress = flag;
  }

  virtual bool isInvalidate() {
    return invalidate;
  }

  void setInvalidate(bool flag) {
    this->invalidate = flag;
  }

  virtual bool isUploading() {
    return uploading;
  }

  void setUploading(bool flag) {
    this->uploading = flag;
  }

  virtual int32_t getId() {
    return id;
  }

  void setId(int32_t id) {
    this->id = id;
  }

  virtual void doReceivedAction() {
  }

  virtual void send() {}

  virtual bool validate(Errors& errors) {
    return false;
  }

  virtual void handleEvent(const BtEventHandle& event) {}

  virtual void onQueued() {}

  virtual string toString() const { return "MockBtMessage"; }

};

typedef SharedHandle<MockBtMessage> MockBtMessageHandle;

#endif // _D_MOCK_BT_MESSAGE_H_
