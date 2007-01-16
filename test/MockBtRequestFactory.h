#ifndef _D_MOCK_BT_REQUEST_FACTORY_H_
#define _D_MOCK_BT_REQUEST_FACTORY_H_

#include "BtRequestFactory.h"

class MockBtRequestFactory : public BtRequestFactory {
public:
  virtual ~MockBtRequestFactory() {}

  virtual void addTargetPiece(const PieceHandle& piece) {}

  virtual void removeTargetPiece(const PieceHandle& piece) {}

  virtual void removeAllTargetPiece() {}

  virtual int countTargetPiece() { return 0; }

  virtual void removeCompletedPiece() {}

  virtual void doChokedAction() {}

  virtual BtMessages createRequestMessages(uint32_t max) { return BtMessages(); }

  virtual BtMessages createRequestMessagesOnEndGame(uint32_t max) { return BtMessages(); }
};

typedef SharedHandle<MockBtRequestFactory> MockBtRequestFactoryHandle;
typedef WeakHandle<MockBtRequestFactory> MockBtRequestFactoryWeakHandle;

#endif // _D_MOCK_BT_REQUEST_FACTORY_H_
