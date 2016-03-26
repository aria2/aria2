#ifndef D_MOCK_BT_REQUEST_FACTORY_H
#define D_MOCK_BT_REQUEST_FACTORY_H

#include "BtRequestFactory.h"
#include "BtRequestMessage.h"

namespace aria2 {

class MockBtRequestFactory : public BtRequestFactory {
public:
  virtual ~MockBtRequestFactory() {}

  virtual void
  addTargetPiece(const std::shared_ptr<Piece>& piece) CXX11_OVERRIDE
  {
  }

  virtual void
  removeTargetPiece(const std::shared_ptr<Piece>& piece) CXX11_OVERRIDE
  {
  }

  virtual void removeAllTargetPiece() CXX11_OVERRIDE {}

  virtual size_t countTargetPiece() CXX11_OVERRIDE { return 0; }

  virtual size_t countMissingBlock() CXX11_OVERRIDE { return 0; }

  virtual void removeCompletedPiece() CXX11_OVERRIDE {}

  virtual void doChokedAction() CXX11_OVERRIDE {}

  virtual std::vector<std::unique_ptr<BtRequestMessage>>
  createRequestMessages(size_t max, bool endGame) CXX11_OVERRIDE
  {
    return std::vector<std::unique_ptr<BtRequestMessage>>{};
  }

  virtual std::vector<size_t> getTargetPieceIndexes() const CXX11_OVERRIDE
  {
    return std::vector<size_t>{};
  }
};

} // namespace aria2

#endif // D_MOCK_BT_REQUEST_FACTORY_H
