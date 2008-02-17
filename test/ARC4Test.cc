#include "ARC4Encryptor.h"
#include "ARC4Decryptor.h"
#include "Exception.h"
#include "Util.h"
#include "DHTUtil.h"
#include <cstring>
#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class ARC4Test:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(ARC4Test);
  CPPUNIT_TEST(testEncryptDecrypt);
  CPPUNIT_TEST_SUITE_END();
public:
  void setUp() {}

  void tearDown() {}

  void testEncryptDecrypt();
};


CPPUNIT_TEST_SUITE_REGISTRATION(ARC4Test);

void ARC4Test::testEncryptDecrypt()
{
  ARC4Encryptor enc;
  ARC4Decryptor dec;
  const size_t LEN = 20;
  unsigned char key[LEN];
  memset(key, 0, LEN);
  DHTUtil::generateRandomData(key, sizeof(key));
  enc.init(key, sizeof(key));
  dec.init(key, sizeof(key));

  unsigned char encrypted[LEN];
  unsigned char decrypted[LEN];
  enc.encrypt(encrypted, LEN, key, LEN);
  dec.decrypt(decrypted, LEN, encrypted, LEN);

  CPPUNIT_ASSERT(memcmp(key, decrypted, LEN) == 0);
  // once more
  enc.encrypt(encrypted, LEN, key, LEN);
  dec.decrypt(decrypted, LEN, encrypted, LEN);

  CPPUNIT_ASSERT(memcmp(key, decrypted, LEN) == 0);
}

} // namespace aria2
