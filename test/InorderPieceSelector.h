#ifndef D_IN_ORDER_PIECE_SELECTOR_H
#define D_IN_ORDER_PIECE_SELECTOR_H

#include "PieceSelector.h"
#include "bitfield.h"

namespace aria2 {

class InorderPieceSelector:public PieceSelector {
public:
  virtual bool select
  (size_t& index, const unsigned char* bitfield, size_t nbits) const
  {
    for(size_t i = 0; i < nbits; ++i) {
      if(bitfield::test(bitfield, nbits, i)) {
        index = i;
        return true;
      }
    }
    return false;
  }
};

} // namespace aria2

#endif // D_IN_ORDER_PIECE_SELECTOR_H
