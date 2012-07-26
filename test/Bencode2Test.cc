#include "bencode2.h"

#include <cppunit/extensions/HelperMacros.h>

#include "RecoverableException.h"

namespace aria2 {

class Bencode2Test:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(Bencode2Test);
  CPPUNIT_TEST(testEncode);
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void testEncode();
};

CPPUNIT_TEST_SUITE_REGISTRATION( Bencode2Test );

void Bencode2Test::testEncode()
{
  {
    Dict dict;
    dict["name"] = String::g("aria2");
    dict["loc"] = Integer::g(80000);
    SharedHandle<List> files = List::g();
    files->append(String::g("aria2c"));
    dict["files"] = files;
    SharedHandle<Dict> attrs = Dict::g();
    attrs->put("license", String::g("GPL"));
    dict["attrs"] = attrs;

    CPPUNIT_ASSERT_EQUAL(std::string("d"
                                     "5:attrsd7:license3:GPLe"
                                     "5:filesl6:aria2ce"
                                     "3:loci80000e"
                                     "4:name5:aria2"
                                     "e"),
                         bencode2::encode(&dict));
  }
}

} // namespace aria2
