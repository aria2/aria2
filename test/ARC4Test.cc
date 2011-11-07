#include "ARC4Encryptor.h"

#include <cstring>
#include <cppunit/extensions/HelperMacros.h>

#include "Exception.h"
#include "util.h"

namespace aria2 {

class ARC4Test:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(ARC4Test);
  CPPUNIT_TEST(testEncrypt);
  CPPUNIT_TEST_SUITE_END();
public:
  void setUp() {}

  void tearDown() {}

  void testEncrypt();
};


CPPUNIT_TEST_SUITE_REGISTRATION(ARC4Test);

void ARC4Test::testEncrypt()
{
  ARC4Encryptor enc;
  ARC4Encryptor dec;
  const size_t LEN = 20;
  unsigned char key[LEN];
  memset(key, 0, LEN);
  util::generateRandomData(key, sizeof(key));
  enc.init(key, sizeof(key));
  dec.init(key, sizeof(key));

  unsigned char encrypted[LEN];
  unsigned char decrypted[LEN];
  enc.encrypt(LEN, encrypted, key);
  dec.encrypt(LEN, decrypted, encrypted);

  CPPUNIT_ASSERT(memcmp(key, decrypted, LEN) == 0);
  // once more
  enc.encrypt(LEN, encrypted, key);
  dec.encrypt(LEN, decrypted, encrypted);

  CPPUNIT_ASSERT(memcmp(key, decrypted, LEN) == 0);
}

} // namespace aria2
