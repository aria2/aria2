#ifndef __CHESS_H__
#define __CHESS_H__

#include "BoardGame.h"

class Chess: public BoardGame {
  public:
    virtual int getNumberOfPieces() const;
};

#endif
