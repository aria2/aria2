#ifndef _D_MOCK_PIECE_SELECTOR_H_
#define _D_MOCK_PIECE_SELECTOR_H_

#include "PieceSelector.h"

namespace aria2 {

class MockPieceSelector:public PieceSelector {
public:
  virtual bool select
  (size_t& index, const unsigned char* bitfield, size_t nbits) const
  {
    return false;
  }
};

} // namespace aria2

#endif // _D_MOCK_PIECE_SELECTOR_H_
