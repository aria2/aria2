#ifndef D_MOCK_BT_MESSAGE_DISPATCHER_H
#define D_MOCK_BT_MESSAGE_DISPATCHER_H

#include "BtMessageDispatcher.h"

#include <algorithm>

#include "BtMessage.h"
#include "Piece.h"

namespace aria2 {

class MockBtMessageDispatcher : public BtMessageDispatcher {
public:
  std::deque<std::unique_ptr<BtMessage>> messageQueue;

  virtual ~MockBtMessageDispatcher() {}

  virtual void
  addMessageToQueue(std::unique_ptr<BtMessage> btMessage) CXX11_OVERRIDE
  {
    messageQueue.push_back(std::move(btMessage));
  }

  virtual void sendMessages() CXX11_OVERRIDE {}

  virtual void doCancelSendingPieceAction(size_t index, int32_t begin,
                                          int32_t length) CXX11_OVERRIDE
  {
  }

  virtual void
  doCancelSendingPieceAction(const std::shared_ptr<Piece>& piece) CXX11_OVERRIDE
  {
  }

  virtual void doAbortOutstandingRequestAction(
      const std::shared_ptr<Piece>& piece) CXX11_OVERRIDE
  {
  }

  virtual void doChokedAction() CXX11_OVERRIDE {}

  virtual void doChokingAction() CXX11_OVERRIDE {}

  virtual void checkRequestSlotAndDoNecessaryThing() CXX11_OVERRIDE {}

  virtual bool isSendingInProgress() CXX11_OVERRIDE { return false; }

  virtual size_t countMessageInQueue() CXX11_OVERRIDE
  {
    return messageQueue.size();
  }

  virtual size_t countOutstandingRequest() CXX11_OVERRIDE { return 0; }

  virtual bool isOutstandingRequest(size_t index,
                                    size_t blockIndex) CXX11_OVERRIDE
  {
    return false;
  }

  virtual const RequestSlot*
  getOutstandingRequest(size_t index, int32_t begin,
                        int32_t length) CXX11_OVERRIDE
  {
    return nullptr;
  }

  virtual void removeOutstandingRequest(const RequestSlot* slot) CXX11_OVERRIDE
  {
  }

  virtual void
  addOutstandingRequest(std::unique_ptr<RequestSlot> slot) CXX11_OVERRIDE
  {
  }

  virtual size_t countOutstandingUpload() CXX11_OVERRIDE { return 0; }
};

} // namespace aria2

#endif // D_MOCK_BT_MESSAGE_DISPATCHER_H
