#include "SequentialPicker.h"

#include <cppunit/extensions/HelperMacros.h>

#include "a2functional.h"

namespace aria2 {

class SequentialPickerTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(SequentialPickerTest);
  CPPUNIT_TEST(testPick);
  CPPUNIT_TEST_SUITE_END();

public:
  void testPick();
};

CPPUNIT_TEST_SUITE_REGISTRATION(SequentialPickerTest);

void SequentialPickerTest::testPick()
{
  SequentialPicker<int> picker;

  CPPUNIT_ASSERT(!picker.isPicked());
  CPPUNIT_ASSERT(!picker.hasNext());
  CPPUNIT_ASSERT_EQUAL((size_t)0, picker.countEntryInQueue());

  picker.pushEntry(make_unique<int>(1));
  picker.pushEntry(make_unique<int>(2));

  CPPUNIT_ASSERT(picker.hasNext());
  CPPUNIT_ASSERT_EQUAL((size_t)2, picker.countEntryInQueue());

  picker.pickNext();

  CPPUNIT_ASSERT(picker.isPicked());
  CPPUNIT_ASSERT_EQUAL(1, *picker.getPickedEntry());

  picker.dropPickedEntry();

  CPPUNIT_ASSERT(!picker.isPicked());
  CPPUNIT_ASSERT(picker.hasNext());

  picker.pickNext();

  CPPUNIT_ASSERT_EQUAL(2, *picker.getPickedEntry());
  CPPUNIT_ASSERT(!picker.hasNext());
}

} // namespace aria2
