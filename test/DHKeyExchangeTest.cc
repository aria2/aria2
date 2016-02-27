#include "DHKeyExchange.h"
#include "Exception.h"
#include "util.h"
#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class DHKeyExchangeTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(DHKeyExchangeTest);
  CPPUNIT_TEST(testHandshake);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() {}

  void tearDown() {}

  void testHandshake();
};

CPPUNIT_TEST_SUITE_REGISTRATION(DHKeyExchangeTest);

void DHKeyExchangeTest::testHandshake()
{
  DHKeyExchange dhA;
  DHKeyExchange dhB;

  const unsigned char* PRIME = reinterpret_cast<const unsigned char*>(
      "FFFFFFFFFFFFFFFFC90FDAA22168C234C4C6628B80DC1CD129024E088A67CC74020BBEA6"
      "3B139B22514A08798E3404DDEF9519B3CD3A431B302B0A6DF25F14374FE1356D6D51C245"
      "E485B576625E7EC6F44C42E9A63A36210000000000090563");

  const size_t PRIME_BITS = 768;

  const unsigned char* GENERATOR = reinterpret_cast<const unsigned char*>("2");

  dhA.init(PRIME, PRIME_BITS, GENERATOR, 160);
  dhB.init(PRIME, PRIME_BITS, GENERATOR, 160);

  dhA.generatePublicKey();
  dhB.generatePublicKey();

  unsigned char publicKeyA[96];
  unsigned char publicKeyB[96];
  dhA.getPublicKey(publicKeyA, sizeof(publicKeyA));
  dhB.getPublicKey(publicKeyB, sizeof(publicKeyB));

  unsigned char secretA[96];
  unsigned char secretB[96];
  dhA.computeSecret(secretA, sizeof(secretA), publicKeyB, sizeof(publicKeyB));
  dhB.computeSecret(secretB, sizeof(secretB), publicKeyA, sizeof(publicKeyA));

  CPPUNIT_ASSERT_EQUAL(util::toHex(secretA, sizeof(secretA)),
                       util::toHex(secretB, sizeof(secretB)));
}

} // namespace aria2
