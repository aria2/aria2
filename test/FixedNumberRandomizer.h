#ifndef _D_FIXED_NUMBER_RANDOMIZER_H_
#define _D_FIXED_NUMBER_RANDOMIZER_H_

#include "Randomizer.h"

class FixedNumberRandomizer : public Randomizer {
private:
  int32_t fixedNumber;
public:
  FixedNumberRandomizer():fixedNumber(0) {}

  virtual ~FixedNumberRandomizer() {}

  virtual long int getRandomNumber() {
    return fixedNumber;
  }

  virtual long int getMaxRandomNumber() {
    return RAND_MAX;
  }

  void setFixedNumber(int32_t num) {
    this->fixedNumber = num;
  }

  int32_t getFixedNumber() const {
    return fixedNumber;
  }
};

#endif // _D_FIXED_NUMBER_RANDOMIZER_H_
