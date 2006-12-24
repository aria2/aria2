#ifndef _D_MOCK_BT_MESSAGE_DISPATCHER_H_
#define _D_MOCK_BT_MESSAGE_DISPATCHER_H_

#include "BtMessageDispatcher.h"

class MockBtMessageDispatcher : public BtMessageDispatcher {
public:
  BtMessages messageQueue;

  virtual ~MockBtMessageDispatcher() {}

  virtual void addMessageToQueue(const BtMessageHandle& btMessage) {
    messageQueue.push_back(btMessage);
  }

  virtual void addMessageToQueue(const BtMessages& btMessages) {
    copy(btMessages.begin(), btMessages.end(), back_inserter(messageQueue));
  }

  virtual void sendMessages() {}

  virtual void doCancelSendingPieceAction(uint32_t index, uint32_t begin, uint32_t blockLength) {}

  virtual void doCancelSendingPieceAction(const PieceHandle& piece) {}

  virtual void doAbortOutstandingRequestAction(const PieceHandle& piece) {}

  virtual void doChokedAction() {}

  virtual void doChokingAction() {}

  virtual void checkRequestSlotAndDoNecessaryThing() {}

  virtual bool isSendingInProgress() {
    return false;
  }

  virtual uint32_t countMessageInQueue() {
    return messageQueue.size();
  }

  virtual uint32_t countOutstandingRequest() {
    return 0;
  }

  virtual bool isOutstandingRequest(uint32_t index, uint32_t blockIndex) {
    return false;
  }

  virtual RequestSlot getOutstandingRequest(uint32_t index, uint32_t begin, uint32_t blockLength) {
    return RequestSlot::nullSlot;
  }

  virtual void removeOutstandingRequest(const RequestSlot& slot) {}
  
  virtual void addOutstandingRequest(const RequestSlot& slot) {}
};

typedef SharedHandle<MockBtMessageDispatcher> MockBtMessageDispatcherHandle;

#endif // _D_MOCK_BT_MESSAGE_DISPATCHER_H_
