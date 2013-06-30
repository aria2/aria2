#ifndef D_MOCK_BT_REQUEST_FACTORY_H
#define D_MOCK_BT_REQUEST_FACTORY_H

#include "BtRequestFactory.h"
#include "BtRequestMessage.h"

namespace aria2 {

class MockBtRequestFactory : public BtRequestFactory {
public:
  virtual ~MockBtRequestFactory() {}

  virtual void addTargetPiece(const std::shared_ptr<Piece>& piece) {}

  virtual void removeTargetPiece(const std::shared_ptr<Piece>& piece) {}

  virtual void removeAllTargetPiece() {}

  virtual size_t countTargetPiece() { return 0; }

  virtual size_t countMissingBlock() { return 0; }

  virtual void removeCompletedPiece() {}

  virtual void doChokedAction() {}

  virtual std::vector<std::unique_ptr<BtRequestMessage>> createRequestMessages
  (size_t max, bool endGame)
  {
    return std::vector<std::unique_ptr<BtRequestMessage>>{};
  }

  virtual std::vector<size_t> getTargetPieceIndexes() const
  {
    return std::vector<size_t>{};
  }
};

} // namespace aria2

#endif // D_MOCK_BT_REQUEST_FACTORY_H
