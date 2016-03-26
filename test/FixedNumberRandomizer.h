#ifndef D_FIXED_NUMBER_RANDOMIZER_H
#define D_FIXED_NUMBER_RANDOMIZER_H

#include "Randomizer.h"
#include <cstdlib>

namespace aria2 {

class FixedNumberRandomizer : public Randomizer {
private:
  int32_t fixedNumber;

public:
  FixedNumberRandomizer() : fixedNumber(0) {}

  virtual ~FixedNumberRandomizer() {}

  virtual long int getRandomNumber(long int to) CXX11_OVERRIDE
  {
    return fixedNumber;
  }

  void setFixedNumber(int32_t num) { this->fixedNumber = num; }

  int32_t getFixedNumber() const { return fixedNumber; }
};

} // namespace aria2

#endif // D_FIXED_NUMBER_RANDOMIZER_H
