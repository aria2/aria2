#ifndef _D_MOCK_BT_REQUEST_FACTORY_H_
#define _D_MOCK_BT_REQUEST_FACTORY_H_

#include "BtRequestFactory.h"

namespace aria2 {

class MockBtRequestFactory : public BtRequestFactory {
public:
  virtual ~MockBtRequestFactory() {}

  virtual void addTargetPiece(const SharedHandle<Piece>& piece) {}

  virtual void removeTargetPiece(const SharedHandle<Piece>& piece) {}

  virtual void removeAllTargetPiece() {}

  virtual size_t countTargetPiece() { return 0; }

  virtual size_t countMissingBlock() { return 0; }

  virtual void removeCompletedPiece() {}

  virtual void doChokedAction() {}

  virtual void createRequestMessages
  (std::deque<SharedHandle<BtMessage> >& requests, size_t max) {}

  virtual void createRequestMessagesOnEndGame
  (std::deque<SharedHandle<BtMessage> >& requests, size_t max) {}
};

} // namespace aria2

#endif // _D_MOCK_BT_REQUEST_FACTORY_H_
