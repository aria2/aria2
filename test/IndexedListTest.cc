#include "IndexedList.h"

#include <vector>
#include <deque>
#include <iostream>

#include <cppunit/extensions/HelperMacros.h>

#include "TestUtil.h"
#include "array_fun.h"
#include "TimerA2.h"

namespace aria2 {

class IndexedListTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(IndexedListTest);
  CPPUNIT_TEST(testPushBack);
  CPPUNIT_TEST(testPushFront);
  CPPUNIT_TEST(testErase);
  CPPUNIT_TEST(testPopFront);
  CPPUNIT_TEST(testMove);
  CPPUNIT_TEST(testGet);
  CPPUNIT_TEST(testInsert);
  CPPUNIT_TEST_SUITE_END();
public:
  void setUp()
  {}

  void testPushBack();
  void testPushFront();
  void testErase();
  void testPopFront();
  void testMove();
  void testGet();
  void testInsert();
};

CPPUNIT_TEST_SUITE_REGISTRATION( IndexedListTest );

void IndexedListTest::testPushBack()
{
  int a[] = {1,2,3,4,5};
  IndexedList<int, int*> list;
  for(int i = 0; i < 5; ++i) {
    CPPUNIT_ASSERT(list.push_back(i, &a[i]));
  }
  for(int i = 0; i < 5; ++i) {
    CPPUNIT_ASSERT_EQUAL(a[i], *list.get(i));
  }
  int ai = 0;
  for(IndexedList<int, int*>::SeqType::iterator i = list.begin();
      i != list.end(); ++i) {
    CPPUNIT_ASSERT_EQUAL(a[ai++], *((*i).second));
  }
}

void IndexedListTest::testPushFront()
{
  int a[] = {1,2,3,4,5};
  IndexedList<int, int*> list;
  for(int i = 0; i < 5; ++i) {
    CPPUNIT_ASSERT(list.push_front(i, &a[i]));
  }
  for(int i = 0; i < 5; ++i) {
    CPPUNIT_ASSERT_EQUAL(a[i], *list.get(i));
  }
  int ai = 4;
  for(IndexedList<int, int*>::SeqType::iterator i = list.begin();
      i != list.end(); ++i) {
    CPPUNIT_ASSERT_EQUAL(a[ai--], *((*i).second));
  }
}

void IndexedListTest::testErase()
{
  int a[] = {1,2,3,4,5};
  IndexedList<int, int*> list;
  for(int i = 0; i < 5; ++i) {
    list.push_back(i, &a[i]);
  }
  for(int i = 0; i < 5; ++i) {
    CPPUNIT_ASSERT(list.erase(i));
    CPPUNIT_ASSERT_EQUAL((size_t)5-i-1, list.size());
    for(int j = i+1; j < 5; ++j) {
      CPPUNIT_ASSERT_EQUAL(a[j], *list.get(j));
    }
  }
}

void IndexedListTest::testPopFront()
{
  int a[] = {1,2,3,4,5};
  IndexedList<int, int*> list;
  for(int i = 0; i < 5; ++i) {
    list.push_back(i, &a[i]);
  }
  for(int i = 0; i < 5; ++i) {
    CPPUNIT_ASSERT(list.pop_front());
    CPPUNIT_ASSERT_EQUAL((size_t)5-i-1, list.size());
    for(int j = i+1; j < 5; ++j) {
      CPPUNIT_ASSERT_EQUAL(a[j], *list.get(j));
    }
  }
}

#define LIST_CHECK(a, list)                                             \
  {                                                                     \
    int ai = 0;                                                         \
    for(IndexedList<int, int*>::SeqType::iterator i = list.begin();     \
        i != list.end(); ++i) {                                         \
      CPPUNIT_ASSERT_EQUAL(a[ai++], *((*i).second));                    \
    }                                                                   \
  }

void IndexedListTest::testMove()
{
  int a[] = {0,1,2,3,4};
  IndexedList<int, int*> list;
  for(int i = 0; i < 5; ++i) {
    list.push_back(i, &a[i]);
  }
  CPPUNIT_ASSERT_EQUAL((ssize_t)-1, list.move(100, 0, A2_POS_SET));
  int a0[] = {0,1,2,3,4};
  CPPUNIT_ASSERT_EQUAL((ssize_t)0, list.move(0, 0, A2_POS_SET));
  LIST_CHECK(a0, list);

  int a1[] = {0,2,3,4,1};
  CPPUNIT_ASSERT_EQUAL((ssize_t)4, list.move(1, 4, A2_POS_SET));
  LIST_CHECK(a1, list);

  int a2[] = {0,3,4,2,1};
  CPPUNIT_ASSERT_EQUAL((ssize_t)3, list.move(2, 3, A2_POS_SET));
  LIST_CHECK(a2, list);

  int a3[] = {0,2,3,4,1};
  CPPUNIT_ASSERT_EQUAL((ssize_t)1, list.move(2, 1, A2_POS_SET));
  LIST_CHECK(a3, list);

  int a4[] = {1,0,2,3,4};
  CPPUNIT_ASSERT_EQUAL((ssize_t)0, list.move(1, 0, A2_POS_SET));
  LIST_CHECK(a4, list);

  int a5[] = {1,0,3,2,4};
  CPPUNIT_ASSERT_EQUAL((ssize_t)2, list.move(3, 2, A2_POS_SET));
  LIST_CHECK(a5, list);

  int a6[] = {1,3,2,4,0};
  CPPUNIT_ASSERT_EQUAL((ssize_t)4, list.move(0, 5, A2_POS_SET));
  LIST_CHECK(a6, list);

  int a7[] = {3,1,2,4,0};
  CPPUNIT_ASSERT_EQUAL((ssize_t)1, list.move(1, 1, A2_POS_CUR));
  LIST_CHECK(a7, list);

  int a8[] = {3,2,4,1,0};
  CPPUNIT_ASSERT_EQUAL((ssize_t)3, list.move(1, 2, A2_POS_CUR));
  LIST_CHECK(a8, list);

  int a9[] = {3,2,1,4,0};
  CPPUNIT_ASSERT_EQUAL((ssize_t)2, list.move(1, -1, A2_POS_CUR));
  LIST_CHECK(a9, list);

  int a10[] = {1,3,2,4,0};
  CPPUNIT_ASSERT_EQUAL((ssize_t)0, list.move(1, -1233, A2_POS_CUR));
  LIST_CHECK(a10, list);

  int a11[] = {3,2,4,0,1};
  CPPUNIT_ASSERT_EQUAL((ssize_t)4, list.move(1, 8733, A2_POS_CUR));
  LIST_CHECK(a11, list);

  int a12[] = {3,2,4,0,1};
  CPPUNIT_ASSERT_EQUAL((ssize_t)3, list.move(0, -1, A2_POS_END));
  LIST_CHECK(a12, list);

  int a13[] = {3,2,0,4,1};
  CPPUNIT_ASSERT_EQUAL((ssize_t)2, list.move(0, -2, A2_POS_END));
  LIST_CHECK(a13, list);

  int a14[] = {0,3,2,4,1};
  CPPUNIT_ASSERT_EQUAL((ssize_t)0, list.move(0, -8733, A2_POS_END));
  LIST_CHECK(a14, list);

  int a15[] = {0,2,4,1,3};
  CPPUNIT_ASSERT_EQUAL((ssize_t)4, list.move(3, 0, A2_POS_END));
  LIST_CHECK(a15, list);

  int a16[] = {2,4,1,3,0};
  CPPUNIT_ASSERT_EQUAL((ssize_t)4, list.move(0, 1000, A2_POS_END));
  LIST_CHECK(a16, list);

  int a17[] = {2,1,4,3,0};
  CPPUNIT_ASSERT_EQUAL((ssize_t)2, list.move(4, 2, A2_POS_SET));
  LIST_CHECK(a17, list);
}

void IndexedListTest::testGet()
{
  IndexedList<int, int*> list;
  int a = 1000;
  int b = 1;
  list.push_back(123, &a);
  list.push_back(1, &b);
  CPPUNIT_ASSERT_EQUAL((int*)0, list.get(1000));
  CPPUNIT_ASSERT_EQUAL(&a, list.get(123));

  CPPUNIT_ASSERT(list.begin() == list.find(123));
  CPPUNIT_ASSERT(list.end() == list.find(124));
}

void IndexedListTest::testInsert()
{
  int a[] = {0,1,2,3,4,5,6,7,8,9};
  IndexedList<int, int*> list;
  IndexedList<int, int*>::SeqType::iterator itr;
  CPPUNIT_ASSERT(list.end() == list.insert(1, 0, &a[5]));
  itr = list.insert(0, 5, &a[5]);
  CPPUNIT_ASSERT_EQUAL(5, *(*itr).second);
  itr = list.insert(1, 3, &a[3]);
  CPPUNIT_ASSERT_EQUAL(3, *(*itr).second);
  itr = list.insert(1, 4, &a[4]);
  CPPUNIT_ASSERT_EQUAL(4, *(*itr).second);
  itr = list.insert(0, 9, &a[9]);
  CPPUNIT_ASSERT_EQUAL(9, *(*itr).second);
  int a1[] = { 9,5,4,3 };
  LIST_CHECK(a1, list);

  // use iterator to insert
  itr = list.insert(itr, 2, &a[2]);
  CPPUNIT_ASSERT_EQUAL(2, *(*itr).second);
  itr = list.insert(list.end(), 1, &a[1]);
  CPPUNIT_ASSERT_EQUAL(1, *(*itr).second);
  int a2[] = { 2,9,5,4,3,1 };
  LIST_CHECK(a2, list);

  // 2 has been already added.
  CPPUNIT_ASSERT(list.end() == list.insert(list.end(), 2, &a[2]));
}

} // namespace aria2
