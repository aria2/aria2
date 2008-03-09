#ifndef _D_MOCK_BT_MESSAGE_DISPATCHER_H_
#define _D_MOCK_BT_MESSAGE_DISPATCHER_H_

#include "BtMessageDispatcher.h"
#include <algorithm>

namespace aria2 {

class MockBtMessageDispatcher : public BtMessageDispatcher {
public:
  BtMessages messageQueue;

  virtual ~MockBtMessageDispatcher() {}

  virtual void addMessageToQueue(const SharedHandle<BtMessage>& btMessage) {
    messageQueue.push_back(btMessage);
  }

  virtual void addMessageToQueue(const std::deque<SharedHandle<BtMessage> >& btMessages) {
    std::copy(btMessages.begin(), btMessages.end(), back_inserter(messageQueue));
  }

  virtual void sendMessages() {}

  virtual void doCancelSendingPieceAction(size_t index, uint32_t begin, size_t length) {}

  virtual void doCancelSendingPieceAction(const SharedHandle<Piece>& piece) {}

  virtual void doAbortOutstandingRequestAction(const SharedHandle<Piece>& piece) {}

  virtual void doChokedAction() {}

  virtual void doChokingAction() {}

  virtual void checkRequestSlotAndDoNecessaryThing() {}

  virtual bool isSendingInProgress() {
    return false;
  }

  virtual size_t countMessageInQueue() {
    return messageQueue.size();
  }

  virtual size_t countOutstandingRequest() {
    return 0;
  }

  virtual bool isOutstandingRequest(size_t index, size_t blockIndex) {
    return false;
  }

  virtual RequestSlot getOutstandingRequest(size_t index, uint32_t begin, size_t length) {
    return RequestSlot::nullSlot;
  }

  virtual void removeOutstandingRequest(const RequestSlot& slot) {}
  
  virtual void addOutstandingRequest(const RequestSlot& slot) {}
};

} // namespace aria2

#endif // _D_MOCK_BT_MESSAGE_DISPATCHER_H_
