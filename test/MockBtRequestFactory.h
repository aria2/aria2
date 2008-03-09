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

  virtual void removeCompletedPiece() {}

  virtual void doChokedAction() {}

  virtual std::deque<SharedHandle<BtMessage> >
  createRequestMessages(size_t max) { return std::deque<SharedHandle<BtMessage> >(); }

  virtual std::deque<SharedHandle<BtMessage> >
  createRequestMessagesOnEndGame(size_t max) { return std::deque<SharedHandle<BtMessage> >(); }
};

} // namespace aria2

#endif // _D_MOCK_BT_REQUEST_FACTORY_H_
