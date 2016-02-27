#include "Option.h"

#include <string>

#include <cppunit/extensions/HelperMacros.h>

#include "prefs.h"

namespace aria2 {

class OptionTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(OptionTest);
  CPPUNIT_TEST(testPutAndGet);
  CPPUNIT_TEST(testPutAndGetAsInt);
  CPPUNIT_TEST(testPutAndGetAsDouble);
  CPPUNIT_TEST(testDefined);
  CPPUNIT_TEST(testBlank);
  CPPUNIT_TEST(testMerge);
  CPPUNIT_TEST(testParent);
  CPPUNIT_TEST(testRemove);
  CPPUNIT_TEST_SUITE_END();

private:
public:
  void setUp() {}

  void testPutAndGet();
  void testPutAndGetAsInt();
  void testPutAndGetAsDouble();
  void testDefined();
  void testBlank();
  void testMerge();
  void testParent();
  void testRemove();
};

CPPUNIT_TEST_SUITE_REGISTRATION(OptionTest);

void OptionTest::testPutAndGet()
{
  Option op;
  op.put(PREF_TIMEOUT, "value");

  CPPUNIT_ASSERT(op.defined(PREF_TIMEOUT));
  CPPUNIT_ASSERT_EQUAL(std::string("value"), op.get(PREF_TIMEOUT));
}

void OptionTest::testPutAndGetAsInt()
{
  Option op;
  op.put(PREF_TIMEOUT, "1000");

  CPPUNIT_ASSERT(op.defined(PREF_TIMEOUT));
  CPPUNIT_ASSERT_EQUAL((int32_t)1000, op.getAsInt(PREF_TIMEOUT));
}

void OptionTest::testPutAndGetAsDouble()
{
  Option op;
  op.put(PREF_TIMEOUT, "10.0");

  CPPUNIT_ASSERT_EQUAL(10.0, op.getAsDouble(PREF_TIMEOUT));
}

void OptionTest::testDefined()
{
  Option op;
  op.put(PREF_TIMEOUT, "v");
  op.put(PREF_DIR, "");
  CPPUNIT_ASSERT(op.defined(PREF_TIMEOUT));
  CPPUNIT_ASSERT(op.defined(PREF_DIR));
  CPPUNIT_ASSERT(!op.defined(PREF_DAEMON));
}

void OptionTest::testBlank()
{
  Option op;
  op.put(PREF_TIMEOUT, "v");
  op.put(PREF_DIR, "");
  CPPUNIT_ASSERT(!op.blank(PREF_TIMEOUT));
  CPPUNIT_ASSERT(op.blank(PREF_DIR));
  CPPUNIT_ASSERT(op.blank(PREF_DAEMON));
}

void OptionTest::testMerge()
{
  Option src;
  src.put(PREF_TIMEOUT, "100");
  src.put(PREF_DAEMON, "true");
  Option dest;
  dest.put(PREF_DAEMON, "false");
  dest.put(PREF_DIR, "foo");
  dest.merge(src);
  CPPUNIT_ASSERT_EQUAL(100, dest.getAsInt(PREF_TIMEOUT));
  CPPUNIT_ASSERT(dest.getAsBool(PREF_DAEMON));
  CPPUNIT_ASSERT_EQUAL(std::string("foo"), dest.get(PREF_DIR));
  CPPUNIT_ASSERT(!dest.defined(PREF_OUT));
}

void OptionTest::testParent()
{
  Option child;
  std::shared_ptr<Option> parent(new Option());
  parent->put(PREF_TIMEOUT, "100");
  child.put(PREF_DIR, "foo");
  CPPUNIT_ASSERT(!child.defined(PREF_TIMEOUT));
  CPPUNIT_ASSERT(!child.definedLocal(PREF_TIMEOUT));
  child.setParent(parent);
  CPPUNIT_ASSERT(child.defined(PREF_TIMEOUT));
  CPPUNIT_ASSERT_EQUAL(std::string("100"), child.get(PREF_TIMEOUT));
  CPPUNIT_ASSERT_EQUAL((int32_t)100, child.getAsInt(PREF_TIMEOUT));
  CPPUNIT_ASSERT(!child.definedLocal(PREF_TIMEOUT));
  // blank
  CPPUNIT_ASSERT(!child.blank(PREF_DIR));
  child.put(PREF_DIR, "");
  CPPUNIT_ASSERT(child.blank(PREF_DIR));
  CPPUNIT_ASSERT(!child.blank(PREF_TIMEOUT));
  // override
  child.put(PREF_TIMEOUT, "200");
  CPPUNIT_ASSERT(child.defined(PREF_TIMEOUT));
  CPPUNIT_ASSERT(child.definedLocal(PREF_TIMEOUT));
  CPPUNIT_ASSERT_EQUAL(std::string("200"), child.get(PREF_TIMEOUT));
  child.removeLocal(PREF_TIMEOUT);
  CPPUNIT_ASSERT(child.defined(PREF_TIMEOUT));
  CPPUNIT_ASSERT(!child.definedLocal(PREF_TIMEOUT));
}

void OptionTest::testRemove()
{
  Option child;
  auto parent = std::make_shared<Option>();
  child.setParent(parent);

  child.put(PREF_DIR, "foo");
  child.put(PREF_TIMEOUT, "200");
  parent->put(PREF_DIR, "bar");
  parent->put(PREF_TIMEOUT, "400");

  child.remove(PREF_DIR);

  CPPUNIT_ASSERT(!child.defined(PREF_DIR));

  child.removeLocal(PREF_TIMEOUT);

  CPPUNIT_ASSERT(!child.definedLocal(PREF_TIMEOUT));
  CPPUNIT_ASSERT(child.defined(PREF_TIMEOUT));
  CPPUNIT_ASSERT(parent->defined(PREF_TIMEOUT));
}

} // namespace aria2
