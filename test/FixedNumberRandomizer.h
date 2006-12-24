#ifndef _D_FIXED_NUMBER_RANDOMIZER_H_
#define _D_FIXED_NUMBER_RANDOMIZER_H_

#include "Randomizer.h"

class FixedNumberRandomizer : public Randomizer {
private:
  int fixedNumber;
public:
  FixedNumberRandomizer():fixedNumber(0) {}

  virtual ~FixedNumberRandomizer() {}

  virtual int getRandomNumber() {
    return fixedNumber;
  }

  virtual int getMaxRandomNumber() {
    return RAND_MAX;
  }

  void setFixedNumber(int num) {
    this->fixedNumber = num;
  }

  int getFixedNumber() const {
    return fixedNumber;
  }
};

#endif // _D_FIXED_NUMBER_RANDOMIZER_H_
