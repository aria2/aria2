#include "paramed_string.h"

#include <iostream>

#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class ParamedStringTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(ParamedStringTest);
  CPPUNIT_TEST(testExpand);
  CPPUNIT_TEST_SUITE_END();
public:
  void testExpand();
};


CPPUNIT_TEST_SUITE_REGISTRATION( ParamedStringTest );

void ParamedStringTest::testExpand()
{
  std::vector<std::string> res;

  std::string s = "alpha:{01,02,03}:bravo:{001,002}";
  paramed_string::expand(s.begin(), s.end(), std::back_inserter(res));
  CPPUNIT_ASSERT_EQUAL((size_t)6, res.size());
  CPPUNIT_ASSERT_EQUAL(std::string("alpha:01:bravo:001"), res[0]);
  CPPUNIT_ASSERT_EQUAL(std::string("alpha:01:bravo:002"), res[1]);
  CPPUNIT_ASSERT_EQUAL(std::string("alpha:02:bravo:001"), res[2]);
  CPPUNIT_ASSERT_EQUAL(std::string("alpha:02:bravo:002"), res[3]);
  CPPUNIT_ASSERT_EQUAL(std::string("alpha:03:bravo:001"), res[4]);
  CPPUNIT_ASSERT_EQUAL(std::string("alpha:03:bravo:002"), res[5]);
  res.clear();

  s = "alpha:[1-3]:bravo";
  paramed_string::expand(s.begin(), s.end(), std::back_inserter(res));
  CPPUNIT_ASSERT_EQUAL((size_t)3, res.size());
  CPPUNIT_ASSERT_EQUAL(std::string("alpha:1:bravo"), res[0]);
  CPPUNIT_ASSERT_EQUAL(std::string("alpha:2:bravo"), res[1]);
  CPPUNIT_ASSERT_EQUAL(std::string("alpha:3:bravo"), res[2]);
  res.clear();

  s = "alpha:[5-12:3]:bravo";
  paramed_string::expand(s.begin(), s.end(), std::back_inserter(res));
  CPPUNIT_ASSERT_EQUAL((size_t)3, res.size());
  CPPUNIT_ASSERT_EQUAL(std::string("alpha:5:bravo"), res[0]);
  CPPUNIT_ASSERT_EQUAL(std::string("alpha:8:bravo"), res[1]);
  CPPUNIT_ASSERT_EQUAL(std::string("alpha:11:bravo"), res[2]);
  res.clear();

  s = "alpha:[05-12:3]:bravo";
  paramed_string::expand(s.begin(), s.end(), std::back_inserter(res));
  CPPUNIT_ASSERT_EQUAL((size_t)3, res.size());
  CPPUNIT_ASSERT_EQUAL(std::string("alpha:05:bravo"), res[0]);
  CPPUNIT_ASSERT_EQUAL(std::string("alpha:08:bravo"), res[1]);
  CPPUNIT_ASSERT_EQUAL(std::string("alpha:11:bravo"), res[2]);
  res.clear();

  s = "alpha:[99-00]:bravo";
  paramed_string::expand(s.begin(), s.end(), std::back_inserter(res));
  CPPUNIT_ASSERT_EQUAL((size_t)1, res.size());
  CPPUNIT_ASSERT_EQUAL(std::string("alpha::bravo"), res[0]);
  res.clear();

  s = "alpha:[65535-65535:65535]:bravo";
  paramed_string::expand(s.begin(), s.end(), std::back_inserter(res));
  CPPUNIT_ASSERT_EQUAL((size_t)1, res.size());
  CPPUNIT_ASSERT_EQUAL(std::string("alpha:65535:bravo"), res[0]);
  res.clear();

  // Invalid loop range
  s = "alpha:[1-]:bravo";
  try {
    paramed_string::expand(s.begin(), s.end(), std::back_inserter(res));
    CPPUNIT_FAIL("Exception must be thrown.");
  } catch(const Exception& e) {
  }

  // Invalid loop range
  s = "alpha:[-1]:bravo";
  try {
    paramed_string::expand(s.begin(), s.end(), std::back_inserter(res));
    CPPUNIT_FAIL("Exception must be thrown.");
  } catch(const Exception& e) {
  }

  // Invalid loop range
  s = "alpha:[1-3a]:bravo";
  try {
    paramed_string::expand(s.begin(), s.end(), std::back_inserter(res));
    CPPUNIT_FAIL("Exception must be thrown.");
  } catch(const Exception& e) {
  }

  // Missing loop step
  s = "alpha:[1-2:]:bravo";
  try {
    paramed_string::expand(s.begin(), s.end(), std::back_inserter(res));
    CPPUNIT_FAIL("Exception must be thrown.");
  } catch(const Exception& e) {
  }

  // Range overflow
  s = "alpha:[0-65536]:bravo";
  try {
    paramed_string::expand(s.begin(), s.end(), std::back_inserter(res));
    CPPUNIT_FAIL("Exception must be thrown.");
  } catch(const Exception& e) {
  }

  // Step overflow
  s = "alpha:[0-1:65536]:bravo";
  try {
    paramed_string::expand(s.begin(), s.end(), std::back_inserter(res));
    CPPUNIT_FAIL("Exception must be thrown.");
  } catch(const Exception& e) {
  }

  s = "alpha:[c-e]:bravo";
  paramed_string::expand(s.begin(), s.end(), std::back_inserter(res));
  CPPUNIT_ASSERT_EQUAL((size_t)3, res.size());
  CPPUNIT_ASSERT_EQUAL(std::string("alpha:c:bravo"), res[0]);
  CPPUNIT_ASSERT_EQUAL(std::string("alpha:d:bravo"), res[1]);
  CPPUNIT_ASSERT_EQUAL(std::string("alpha:e:bravo"), res[2]);
  res.clear();

  s = "alpha:[C-E]:bravo";
  paramed_string::expand(s.begin(), s.end(), std::back_inserter(res));
  CPPUNIT_ASSERT_EQUAL((size_t)3, res.size());
  CPPUNIT_ASSERT_EQUAL(std::string("alpha:C:bravo"), res[0]);
  CPPUNIT_ASSERT_EQUAL(std::string("alpha:D:bravo"), res[1]);
  CPPUNIT_ASSERT_EQUAL(std::string("alpha:E:bravo"), res[2]);
  res.clear();

  s = "alpha:[v-z:2]:bravo";
  paramed_string::expand(s.begin(), s.end(), std::back_inserter(res));
  CPPUNIT_ASSERT_EQUAL((size_t)3, res.size());
  CPPUNIT_ASSERT_EQUAL(std::string("alpha:v:bravo"), res[0]);
  CPPUNIT_ASSERT_EQUAL(std::string("alpha:x:bravo"), res[1]);
  CPPUNIT_ASSERT_EQUAL(std::string("alpha:z:bravo"), res[2]);
  res.clear();

  s = "alpha:[aa-ba]:bravo";
  paramed_string::expand(s.begin(), s.end(), std::back_inserter(res));
  CPPUNIT_ASSERT_EQUAL((size_t)27, res.size());
  CPPUNIT_ASSERT_EQUAL(std::string("alpha:aa:bravo"), res[0]);
  CPPUNIT_ASSERT_EQUAL(std::string("alpha:az:bravo"), res[25]);
  CPPUNIT_ASSERT_EQUAL(std::string("alpha:ba:bravo"), res[26]);
  res.clear();

  s = "alpha:[a-ba]:bravo";
  paramed_string::expand(s.begin(), s.end(), std::back_inserter(res));
  CPPUNIT_ASSERT_EQUAL((size_t)27, res.size());
  CPPUNIT_ASSERT_EQUAL(std::string("alpha:a:bravo"), res[0]);
  CPPUNIT_ASSERT_EQUAL(std::string("alpha:z:bravo"), res[25]);
  CPPUNIT_ASSERT_EQUAL(std::string("alpha:ba:bravo"), res[26]);
  res.clear();

  s = "alpha:[z-a]:bravo";
  paramed_string::expand(s.begin(), s.end(), std::back_inserter(res));
  CPPUNIT_ASSERT_EQUAL((size_t)1, res.size());
  CPPUNIT_ASSERT_EQUAL(std::string("alpha::bravo"), res[0]);
  res.clear();

  s = "alpha:[dsyo-dsyp]:bravo";
  paramed_string::expand(s.begin(), s.end(), std::back_inserter(res));
  CPPUNIT_ASSERT_EQUAL((size_t)2, res.size());
  CPPUNIT_ASSERT_EQUAL(std::string("alpha:dsyo:bravo"), res[0]);
  CPPUNIT_ASSERT_EQUAL(std::string("alpha:dsyp:bravo"), res[1]);
  res.clear();

  // Range overflow
  s = "alpha:[dsyo-dsyq]:bravo";
  try {
    paramed_string::expand(s.begin(), s.end(), std::back_inserter(res));
    CPPUNIT_FAIL("Exception must be thrown.");
  } catch(const Exception& e) {
  }

  // Invalid loop range
  s = "alpha:[a-Z]:bravo";
  try {
    paramed_string::expand(s.begin(), s.end(), std::back_inserter(res));
    CPPUNIT_FAIL("Exception must be thrown.");
  } catch(const Exception& e) {
  }

  // Combination of {} and []
  s = "http://{jp,us}.mirror/image_cd[000-001].iso";
  paramed_string::expand(s.begin(), s.end(), std::back_inserter(res));
  CPPUNIT_ASSERT_EQUAL((size_t)4, res.size());
  CPPUNIT_ASSERT_EQUAL(std::string("http://jp.mirror/image_cd000.iso"), res[0]);
  CPPUNIT_ASSERT_EQUAL(std::string("http://jp.mirror/image_cd001.iso"), res[1]);
  CPPUNIT_ASSERT_EQUAL(std::string("http://us.mirror/image_cd000.iso"), res[2]);
  CPPUNIT_ASSERT_EQUAL(std::string("http://us.mirror/image_cd001.iso"), res[3]);
  res.clear();
}

} // namespace aria2
