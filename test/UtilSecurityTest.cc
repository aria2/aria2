#include "util.h"
#include "util_security.h"

#include <cppunit/extensions/HelperMacros.h>

// Test vectors from RFC 6234
enum {
  SHA1HashSize = 20,
  SHA224HashSize = 28,
  SHA256HashSize = 32,
  SHA384HashSize = 48,
  SHA512HashSize = 64,
};

static struct hmachash {
  const char* keyarray[5];
  int keylength[5];
  const char* dataarray[5];
  int datalength[5];
  const char* resultarray[5];
  int resultlength[5];
} hmachashes[] = {
    {/* 1 */ {"\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b"
              "\x0b\x0b\x0b\x0b\x0b"},
     {20},
     {
         "\x48\x69\x20\x54\x68\x65\x72\x65" /* "Hi There" */
     },
     {8},
     {/* HMAC-SHA-1 */
      "B617318655057264E28BC0B6FB378C8EF146BE00",
      /* HMAC-SHA-224 */
      "896FB1128ABBDF196832107CD49DF33F47B4B1169912BA4F53684B22",
      /* HMAC-SHA-256 */
      "B0344C61D8DB38535CA8AFCEAF0BF12B881DC200C9833DA726E9376C2E32"
      "CFF7",
      /* HMAC-SHA-384 */
      "AFD03944D84895626B0825F4AB46907F15F9DADBE4101EC682AA034C7CEB"
      "C59CFAEA9EA9076EDE7F4AF152E8B2FA9CB6",
      /* HMAC-SHA-512 */
      "87AA7CDEA5EF619D4FF0B4241A1D6CB02379F4E2CE4EC2787AD0B30545E1"
      "7CDEDAA833B7D6B8A702038B274EAEA3F4E4BE9D914EEB61F1702E696C20"
      "3A126854"},
     {SHA1HashSize, SHA224HashSize, SHA256HashSize, SHA384HashSize,
      SHA512HashSize}},
    {/* 2 */ {
         "\x4a\x65\x66\x65" /* "Jefe" */
     },
     {4},
     {
         "\x77\x68\x61\x74\x20\x64\x6f\x20\x79\x61\x20\x77\x61\x6e\x74"
         "\x20\x66\x6f\x72\x20\x6e\x6f\x74\x68\x69\x6e\x67\x3f"
         /* "what do ya want for nothing?" */
     },
     {28},
     {/* HMAC-SHA-1 */
      "EFFCDF6AE5EB2FA2D27416D5F184DF9C259A7C79",
      /* HMAC-SHA-224 */
      "A30E01098BC6DBBF45690F3A7E9E6D0F8BBEA2A39E6148008FD05E44",
      /* HMAC-SHA-256 */
      "5BDCC146BF60754E6A042426089575C75A003F089D2739839DEC58B964EC"
      "3843",
      /* HMAC-SHA-384 */
      "AF45D2E376484031617F78D2B58A6B1B9C7EF464F5A01B47E42EC3736322"
      "445E8E2240CA5E69E2C78B3239ECFAB21649",
      /* HMAC-SHA-512 */
      "164B7A7BFCF819E2E395FBE73B56E0A387BD64222E831FD610270CD7EA25"
      "05549758BF75C05A994A6D034F65F8F0E6FDCAEAB1A34D4A6B4B636E070A"
      "38BCE737"},
     {SHA1HashSize, SHA224HashSize, SHA256HashSize, SHA384HashSize,
      SHA512HashSize}},
    {/* 3 */
     {"\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
      "\xaa\xaa\xaa\xaa\xaa"},
     {20},
     {"\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd"
      "\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd"
      "\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd"
      "\xdd\xdd\xdd\xdd\xdd"},
     {50},
     {/* HMAC-SHA-1 */
      "125D7342B9AC11CD91A39AF48AA17B4F63F175D3",
      /* HMAC-SHA-224 */
      "7FB3CB3588C6C1F6FFA9694D7D6AD2649365B0C1F65D69D1EC8333EA",
      /* HMAC-SHA-256 */
      "773EA91E36800E46854DB8EBD09181A72959098B3EF8C122D9635514CED5"
      "65FE",
      /* HMAC-SHA-384 */
      "88062608D3E6AD8A0AA2ACE014C8A86F0AA635D947AC9FEBE83EF4E55966"
      "144B2A5AB39DC13814B94E3AB6E101A34F27",
      /* HMAC-SHA-512 */
      "FA73B0089D56A284EFB0F0756C890BE9B1B5DBDD8EE81A3655F83E33B227"
      "9D39BF3E848279A722C806B485A47E67C807B946A337BEE8942674278859"
      "E13292FB"},
     {SHA1HashSize, SHA224HashSize, SHA256HashSize, SHA384HashSize,
      SHA512HashSize}},
    {/* 4 */ {"\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f"
              "\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19"},
     {25},
     {"\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd"
      "\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd"
      "\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd"
      "\xcd\xcd\xcd\xcd\xcd"},
     {50},
     {/* HMAC-SHA-1 */
      "4C9007F4026250C6BC8414F9BF50C86C2D7235DA",
      /* HMAC-SHA-224 */
      "6C11506874013CAC6A2ABC1BB382627CEC6A90D86EFC012DE7AFEC5A",
      /* HMAC-SHA-256 */
      "82558A389A443C0EA4CC819899F2083A85F0FAA3E578F8077A2E3FF46729"
      "665B",
      /* HMAC-SHA-384 */
      "3E8A69B7783C25851933AB6290AF6CA77A9981480850009CC5577C6E1F57"
      "3B4E6801DD23C4A7D679CCF8A386C674CFFB",
      /* HMAC-SHA-512 */
      "B0BA465637458C6990E5A8C5F61D4AF7E576D97FF94B872DE76F8050361E"
      "E3DBA91CA5C11AA25EB4D679275CC5788063A5F19741120C4F2DE2ADEBEB"
      "10A298DD"},
     {SHA1HashSize, SHA224HashSize, SHA256HashSize, SHA384HashSize,
      SHA512HashSize}},
    {/* 5 */ {"\x0c\x0c\x0c\x0c\x0c\x0c\x0c\x0c\x0c\x0c\x0c\x0c\x0c\x0c\x0c"
              "\x0c\x0c\x0c\x0c\x0c"},
     {20},
     {"Test With Truncation"},
     {20},
     {/* HMAC-SHA-1 */
      "4C1A03424B55E07FE7F27BE1",
      /* HMAC-SHA-224 */
      "0E2AEA68A90C8D37C988BCDB9FCA6FA8",
      /* HMAC-SHA-256 */
      "A3B6167473100EE06E0C796C2955552B",
      /* HMAC-SHA-384 */
      "3ABF34C3503B2A23A46EFC619BAEF897",
      /* HMAC-SHA-512 */
      "415FAD6271580A531D4179BC891D87A6"},
     {12, 16, 16, 16, 16}},
    {/* 6 */ {"\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
              "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
              "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
              "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
              "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
              "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
              "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
              "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
              "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"},
     {80, 131},
     {"Test Using Larger Than Block-Size Key - Hash Key First"},
     {54},
     {/* HMAC-SHA-1 */
      "AA4AE5E15272D00E95705637CE8A3B55ED402112",
      /* HMAC-SHA-224 */
      "95E9A0DB962095ADAEBE9B2D6F0DBCE2D499F112F2D2B7273FA6870E",
      /* HMAC-SHA-256 */
      "60E431591EE0B67F0D8A26AACBF5B77F8E0BC6213728C5140546040F0EE3"
      "7F54",
      /* HMAC-SHA-384 */
      "4ECE084485813E9088D2C63A041BC5B44F9EF1012A2B588F3CD11F05033A"
      "C4C60C2EF6AB4030FE8296248DF163F44952",
      /* HMAC-SHA-512 */
      "80B24263C7C1A3EBB71493C1DD7BE8B49B46D1F41B4AEEC1121B013783F8"
      "F3526B56D037E05F2598BD0FD2215D6A1E5295E64F73F63F0AEC8B915A98"
      "5D786598"},
     {SHA1HashSize, SHA224HashSize, SHA256HashSize, SHA384HashSize,
      SHA512HashSize}},
    {/* 7 */ {"\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
              "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
              "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
              "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
              "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
              "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
              "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
              "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
              "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"},
     {80, 131},
     {
         "Test Using Larger Than Block-Size Key and "
         "Larger Than One Block-Size Data",
         "\x54\x68\x69\x73\x20\x69\x73\x20\x61\x20\x74\x65\x73\x74\x20"
         "\x75\x73\x69\x6e\x67\x20\x61\x20\x6c\x61\x72\x67\x65\x72\x20"
         "\x74\x68\x61\x6e\x20\x62\x6c\x6f\x63\x6b\x2d\x73\x69\x7a\x65"
         "\x20\x6b\x65\x79\x20\x61\x6e\x64\x20\x61\x20\x6c\x61\x72\x67"
         "\x65\x72\x20\x74\x68\x61\x6e\x20\x62\x6c\x6f\x63\x6b\x2d\x73"
         "\x69\x7a\x65\x20\x64\x61\x74\x61\x2e\x20\x54\x68\x65\x20\x6b"
         "\x65\x79\x20\x6e\x65\x65\x64\x73\x20\x74\x6f\x20\x62\x65\x20"
         "\x68\x61\x73\x68\x65\x64\x20\x62\x65\x66\x6f\x72\x65\x20\x62"
         "\x65\x69\x6e\x67\x20\x75\x73\x65\x64\x20\x62\x79\x20\x74\x68"
         "\x65\x20\x48\x4d\x41\x43\x20\x61\x6c\x67\x6f\x72\x69\x74\x68"
         "\x6d\x2e"
         /* "This is a test using a larger than block-size key and a "
             "larger than block-size data.  The key needs to be hashed "
             "before being used by the HMAC algorithm." */
     },
     {73, 152},
     {/* HMAC-SHA-1 */
      "E8E99D0F45237D786D6BBAA7965C7808BBFF1A91",
      /* HMAC-SHA-224 */
      "3A854166AC5D9F023F54D517D0B39DBD946770DB9C2B95C9F6F565D1",
      /* HMAC-SHA-256 */
      "9B09FFA71B942FCB27635FBCD5B0E944BFDC63644F0713938A7F51535C3A"
      "35E2",
      /* HMAC-SHA-384 */
      "6617178E941F020D351E2F254E8FD32C602420FEB0B8FB9ADCCEBB82461E"
      "99C5A678CC31E799176D3860E6110C46523E",
      /* HMAC-SHA-512 */
      "E37B6A775DC87DBAA4DFA9F96E5E3FFDDEBD71F8867289865DF5A32D20CD"
      "C944B6022CAC3C4982B10D5EEB55C3E4DE15134676FB6DE0446065C97440"
      "FA8C6A58"},
     {SHA1HashSize, SHA224HashSize, SHA256HashSize, SHA384HashSize,
      SHA512HashSize}}};

namespace aria2 {

class SecurityTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(SecurityTest);
  CPPUNIT_TEST(testCompareByte);
  CPPUNIT_TEST(testCompareArray);
  CPPUNIT_TEST(testHMAC);
  CPPUNIT_TEST(testHMACRandom);
  CPPUNIT_TEST(testPBKDF2);
  CPPUNIT_TEST_SUITE_END();

private:
public:
  void setUp() {}

  void testCompareByte();
  void testCompareArray();
  void testHMAC();
  void testHMACRandom();
  void testPBKDF2();
};

CPPUNIT_TEST_SUITE_REGISTRATION(SecurityTest);

void SecurityTest::testCompareByte()
{
  CPPUNIT_ASSERT(util::security::compare('a', 'a'));
  CPPUNIT_ASSERT(util::security::compare('\0', '\0'));
  CPPUNIT_ASSERT(util::security::compare(0xff, 0xff));

  CPPUNIT_ASSERT(!util::security::compare('a', 'f'));
  CPPUNIT_ASSERT(!util::security::compare('\xfe', 'f'));
  CPPUNIT_ASSERT(!util::security::compare(0, 0xff));
}

void SecurityTest::testCompareArray()
{
  CPPUNIT_ASSERT(util::security::compare("", "", 0));
  CPPUNIT_ASSERT(util::security::compare("a", "a", 1));
  CPPUNIT_ASSERT(util::security::compare("a", "ab", 1));
  CPPUNIT_ASSERT(util::security::compare("a\0b", "a\0b", 3));
  CPPUNIT_ASSERT(
      util::security::compare("a\0b", "a\0b", 4)); // implicit null-termination
  CPPUNIT_ASSERT(util::security::compare("a\0", "a\0b", 2));
  CPPUNIT_ASSERT(util::security::compare("a\xff", "a\xff", 2));

  CPPUNIT_ASSERT(!util::security::compare("a", "b", 1));
  CPPUNIT_ASSERT(!util::security::compare("a", "bb", 1));
  CPPUNIT_ASSERT(!util::security::compare("a\1b", "a\0b", 3));
  CPPUNIT_ASSERT(
      !util::security::compare("a\1b", "a\0b", 4)); // implicit null-termination
  CPPUNIT_ASSERT(!util::security::compare("a\4", "a\0b", 2));
  CPPUNIT_ASSERT(!util::security::compare("a\0", "a\xff", 2));
}

static struct {
  const char* hash;
  const size_t idx;
} hmacs[] = {{"sha-1", 0},
             {"sha-224", 1},
             {"sha-256", 2},
             {"sha-384", 3},
             {"sha-512", 4}};

void SecurityTest::testHMAC()
{
  for (const auto& test : hmachashes) {
    for (const auto& hmac : hmacs) {
      if (!util::security::HMAC::supports(hmac.hash)) {
        continue;
      }
#define SELECT(v)                                                              \
  test.v[hmac.idx] ? test.v[hmac.idx] : test.v[1] ? test.v[1] : test.v[0]
      auto key = SELECT(keyarray);
      auto keylen = SELECT(keylength);
      auto data = SELECT(dataarray);
      auto datalen = SELECT(datalength);
      auto result = SELECT(resultarray);
      auto resultlen = SELECT(resultlength);
#undef SELECT

      auto h = util::security::HMAC::create(hmac.hash, key, keylen);
      auto h2 = util::security::HMAC::create(hmac.hash, key, keylen - 1);
      auto r = h->getResult(data, datalen);
      auto hr = util::toUpper(util::toHex(r.getBytes()));
      CPPUNIT_ASSERT(util::security::compare(hr.data(), result, resultlen));
      CPPUNIT_ASSERT(r == r);
      auto r2 = r;
      CPPUNIT_ASSERT(r == r2);
      CPPUNIT_ASSERT(r != h2->getResult(data, datalen));
    }
  }
}

void SecurityTest::testHMACRandom()
{
  auto h = util::security::HMAC::createRandom();
  CPPUNIT_ASSERT(h->getResult("abc") == h->getResult("abc"));
  auto r = h->getResult("def");
  CPPUNIT_ASSERT(r == h->getResult("def"));

  // Sanity check. At the very least 3 out of ten times 2 random hmacs should
  // be different from each other and the 0-secret hmac.
  size_t diff = 0;
  for (auto i = 0; i < 10; ++i) {
    auto h1 = util::security::HMAC::createRandom("sha-1");
    auto h2 = util::security::HMAC::createRandom("sha-1");
    auto h3 = util::security::HMAC::create("sha-1", "");
    CPPUNIT_ASSERT(h1->getResult("abc") == h1->getResult("abc"));
    CPPUNIT_ASSERT(h2->getResult("abc") == h2->getResult("abc"));
    if (h1->getResult("abc") != h2->getResult("abc") &&
        h1->getResult("abc") != h3->getResult("abc") &&
        h2->getResult("abc") != h3->getResult("abc")) {
      ++diff;
    }
  }
  CPPUNIT_ASSERT(diff > 3);
}

static struct pbkdf2 {
  char pass[32];
  size_t pass_len;
  char salt[40];
  size_t salt_len;
  size_t iterations;
  unsigned char key[32];
  size_t key_len;
} pbkdf2s[] = {{"password",
                8,
                "salt",
                4,
                1,
                {0x0c, 0x60, 0xc8, 0x0f, 0x96, 0x1f, 0x0e, 0x71, 0xf3, 0xa9,
                 0xb5, 0x24, 0xaf, 0x60, 0x12, 0x06, 0x2f, 0xe0, 0x37, 0xa6},
                20},
               {"password",
                8,
                "salt",
                4,
                1,
                {0x0c, 0x60, 0xc8, 0x0f, 0x96, 0x1f, 0x0e, 0x71, 0xf3, 0xa9,
                 0xb5, 0x24, 0xaf, 0x60, 0x12, 0x06, 0x2f, 0xe0, 0x37, 0xa6},
                0},
               {"password",
                8,
                "salt",
                4,
                2,
                {0xea, 0x6c, 0x01, 0x4d, 0xc7, 0x2d, 0x6f, 0x8c, 0xcd, 0x1e,
                 0xd9, 0x2a, 0xce, 0x1d, 0x41, 0xf0, 0xd8, 0xde, 0x89, 0x57},
                20},
               {"password",
                8,
                "salt",
                4,
                4096,
                {0x4b, 0x00, 0x79, 0x01, 0xb7, 0x65, 0x48, 0x9a, 0xbe, 0xad,
                 0x49, 0xd9, 0x26, 0xf7, 0x21, 0xd0, 0x65, 0xa4, 0x29, 0xc1},
                20},
               /* Disabled due to #236
               {
                 "password", 8,
                 "salt", 4,
                 16777216,
                 { 0xee, 0xfe, 0x3d, 0x61, 0xcd, 0x4d, 0xa4, 0xe4, 0xe9, 0x94,
               0x5b, 0x3d, 0x6b, 0xa2, 0x15, 0x8c, 0x26, 0x34, 0xe9, 0x84 }, 20,
               },
               */
               {
                   "passwordPASSWORDpassword",
                   24,
                   "saltSALTsaltSALTsaltSALTsaltSALTsalt",
                   36,
                   4096,
                   {0x3d, 0x2e, 0xec, 0x4f, 0xe4, 0x1c, 0x84, 0x9b, 0x80,
                    0xc8, 0xd8, 0x36, 0x62, 0xc0, 0xe4, 0x4a, 0x8b, 0x29,
                    0x1a, 0x96, 0x4c, 0xf2, 0xf0, 0x70, 0x38},
                   25,
               },
               {"pass\0word",
                9,
                "sa\0lt",
                5,
                4096,
                {0x56, 0xfa, 0x6a, 0xa7, 0x55, 0x48, 0x09, 0x9d, 0xcc, 0x37,
                 0xd7, 0xf0, 0x34, 0x25, 0xe0, 0xc3},
                16}};

void SecurityTest::testPBKDF2()
{
  for (const auto& test : pbkdf2s) {
    auto h = util::security::HMAC::create(test.pass, test.pass_len);
    auto r = util::security::PBKDF2(h.get(), test.salt, test.salt_len,
                                    test.iterations, test.key_len);
    const auto kl = test.key_len ? test.key_len : r.length();
    auto e = util::security::HMACResult((char*)test.key, kl);
#if 0
    printf("actual [%s]  expected [%s]\n",
        util::toHex(r.getBytes()).c_str(),
        util::toHex(e.getBytes()).c_str()
        );
#endif
    CPPUNIT_ASSERT(r == e);
    CPPUNIT_ASSERT(r.getBytes() == std::string((char*)test.key, kl));
  }
}

} // namespace aria2
