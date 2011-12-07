#ifndef D_MOCK_BT_MESSAGE_DISPATCHER_H
#define D_MOCK_BT_MESSAGE_DISPATCHER_H

#include "BtMessageDispatcher.h"

#include <algorithm>

#include "BtMessage.h"
#include "Piece.h"

namespace aria2 {

class MockBtMessageDispatcher : public BtMessageDispatcher {
public:
  std::deque<SharedHandle<BtMessage> > messageQueue;

  virtual ~MockBtMessageDispatcher() {}

  virtual void addMessageToQueue(const SharedHandle<BtMessage>& btMessage) {
    messageQueue.push_back(btMessage);
  }

  virtual void addMessageToQueue
  (const std::vector<SharedHandle<BtMessage> >& btMessages)
  {
    std::copy(btMessages.begin(), btMessages.end(), back_inserter(messageQueue));
  }

  virtual void sendMessages() {}

  virtual void doCancelSendingPieceAction
  (size_t index, int32_t begin, int32_t length) {}

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

  virtual RequestSlot getOutstandingRequest
  (size_t index, int32_t begin, int32_t length) {
    return RequestSlot::nullSlot;
  }

  virtual void removeOutstandingRequest(const RequestSlot& slot) {}
  
  virtual void addOutstandingRequest(const RequestSlot& slot) {}

  virtual size_t countOutstandingUpload()
  {
    return 0;
  }
};

} // namespace aria2

#endif // D_MOCK_BT_MESSAGE_DISPATCHER_H
