#ifndef D_MOCK_PIECE_SELECTOR_H
#define D_MOCK_PIECE_SELECTOR_H

#include "PieceSelector.h"

namespace aria2 {

class MockPieceSelector : public PieceSelector {
public:
  virtual bool select(size_t& index, const unsigned char* bitfield,
                      size_t nbits) const CXX11_OVERRIDE
  {
    return false;
  }
};

} // namespace aria2

#endif // D_MOCK_PIECE_SELECTOR_H
