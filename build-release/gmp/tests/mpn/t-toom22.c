#define mpn_toomMN_mul mpn_toom22_mul
#define mpn_toomMN_mul_itch mpn_toom22_mul_itch
#define MIN_AN 2

#define MIN_BN(an)				\
  ((an) >= 2*MUL_TOOM22_THRESHOLD		\
   ? (an) + 2 - MUL_TOOM22_THRESHOLD		\
   : ((an)+1)/2 + 1)

#include "toom-shared.h"
