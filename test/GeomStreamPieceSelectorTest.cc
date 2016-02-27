#include "GeomStreamPieceSelector.h"

#include <cstring>

#include <cppunit/extensions/HelperMacros.h>

#include "Exception.h"
#include "util.h"
#include "BitfieldMan.h"

namespace aria2 {

class GeomStreamPieceSelectorTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(GeomStreamPieceSelectorTest);
  CPPUNIT_TEST(testOnBitfieldInit);
  CPPUNIT_TEST_SUITE_END();

public:
  void testOnBitfieldInit();
};

CPPUNIT_TEST_SUITE_REGISTRATION(GeomStreamPieceSelectorTest);

void GeomStreamPieceSelectorTest::testOnBitfieldInit()
{
  BitfieldMan bf(1_k, 20_k);
  bf.setBitRange(0, 10);
  GeomStreamPieceSelector sel(&bf, 2);
  sel.onBitfieldInit();
  unsigned char igbf[3];
  memset(igbf, 0, 3);
  size_t index;
  // 11111|11111|00000|00000
  CPPUNIT_ASSERT(sel.select(index, 20_k, igbf, sizeof(igbf)));
  CPPUNIT_ASSERT_EQUAL((size_t)11, index);
  bf.setUseBit(11);
  // 11111|11111|10000|00000
  CPPUNIT_ASSERT(sel.select(index, 20_k, igbf, sizeof(igbf)));
  CPPUNIT_ASSERT_EQUAL((size_t)12, index);
  bf.setUseBit(12);
  // 11111|11111|11000|00000
  CPPUNIT_ASSERT(sel.select(index, 20_k, igbf, sizeof(igbf)));
  CPPUNIT_ASSERT_EQUAL((size_t)13, index);
  bf.setUseBit(13);
  // 11111|11111|11100|00000
  CPPUNIT_ASSERT(sel.select(index, 20_k, igbf, sizeof(igbf)));
  CPPUNIT_ASSERT_EQUAL((size_t)15, index);
}

} // namespace aria2
