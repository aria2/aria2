#include "IndexedList.h"

#include <vector>
#include <deque>
#include <iostream>

#include <cppunit/extensions/HelperMacros.h>

#include "TestUtil.h"
#include "array_fun.h"
#include "TimerA2.h"

namespace aria2 {

class IndexedListTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(IndexedListTest);
  CPPUNIT_TEST(testPushBack);
  CPPUNIT_TEST(testPushFront);
  CPPUNIT_TEST(testRemove);
  CPPUNIT_TEST(testErase);
  CPPUNIT_TEST(testPopFront);
  CPPUNIT_TEST(testMove);
  CPPUNIT_TEST(testGet);
  CPPUNIT_TEST(testInsert);
  CPPUNIT_TEST(testInsert_keyFunc);
  CPPUNIT_TEST(testIterator);
  CPPUNIT_TEST(testRemoveIf);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() {}

  void testPushBack();
  void testPushFront();
  void testRemove();
  void testErase();
  void testPopFront();
  void testMove();
  void testGet();
  void testInsert();
  void testInsert_keyFunc();
  void testIterator();
  void testRemoveIf();
};

CPPUNIT_TEST_SUITE_REGISTRATION(IndexedListTest);

void IndexedListTest::testPushBack()
{
  int a[] = {1, 2, 3, 4, 5};
  IndexedList<int, int*> list;
  for (int i = 0; i < 5; ++i) {
    CPPUNIT_ASSERT(list.push_back(i, &a[i]));
  }
  for (int i = 0; i < 5; ++i) {
    CPPUNIT_ASSERT_EQUAL(a[i], *list.get(i));
  }
  int ai = 0;
  for (IndexedList<int, int*>::iterator i = list.begin(); i != list.end();
       ++i) {
    CPPUNIT_ASSERT_EQUAL(a[ai++], **i);
  }
}

void IndexedListTest::testPushFront()
{
  int a[] = {1, 2, 3, 4, 5};
  IndexedList<int, int*> list;
  for (int i = 0; i < 5; ++i) {
    CPPUNIT_ASSERT(list.push_front(i, &a[i]));
  }
  for (int i = 0; i < 5; ++i) {
    CPPUNIT_ASSERT_EQUAL(a[i], *list.get(i));
  }
  int ai = 4;
  for (IndexedList<int, int*>::iterator i = list.begin(); i != list.end();
       ++i) {
    CPPUNIT_ASSERT_EQUAL(a[ai--], **i);
  }
}

void IndexedListTest::testRemove()
{
  int a[] = {1, 2, 3, 4, 5};
  IndexedList<int, int*> list;
  for (int i = 0; i < 5; ++i) {
    list.push_back(i, &a[i]);
  }
  for (int i = 0; i < 5; ++i) {
    CPPUNIT_ASSERT(list.remove(i));
    CPPUNIT_ASSERT_EQUAL((size_t)5 - i - 1, list.size());
    for (int j = i + 1; j < 5; ++j) {
      CPPUNIT_ASSERT_EQUAL(a[j], *list.get(j));
    }
  }
}

void IndexedListTest::testErase()
{
  int a[] = {1, 2, 3, 4, 5};
  IndexedList<int, int*> list;
  for (int i = 0; i < 5; ++i) {
    list.push_back(i, &a[i]);
  }
  int* p = a;
  for (IndexedList<int, int*>::iterator i = list.begin(); i != list.end();) {
    i = list.erase(i);
    CPPUNIT_ASSERT_EQUAL((size_t)(std::distance(i, list.end())), list.size());

    int* pp = ++p;
    for (IndexedList<int, int*>::iterator j = list.begin(); j != list.end();
         ++j, ++pp) {
      CPPUNIT_ASSERT_EQUAL(*pp, **j);
    }
  }
}

void IndexedListTest::testPopFront()
{
  int a[] = {1, 2, 3, 4, 5};
  IndexedList<int, int*> list;
  for (int i = 0; i < 5; ++i) {
    list.push_back(i, &a[i]);
  }
  for (int i = 0; i < 5; ++i) {
    CPPUNIT_ASSERT(list.pop_front());
    CPPUNIT_ASSERT_EQUAL((size_t)5 - i - 1, list.size());
    for (int j = i + 1; j < 5; ++j) {
      CPPUNIT_ASSERT_EQUAL(a[j], *list.get(j));
    }
  }
}

#define LIST_CHECK(a, list)                                                    \
  {                                                                            \
    int ai = 0;                                                                \
    for (IndexedList<int, int*>::iterator i = list.begin(); i != list.end();   \
         ++i) {                                                                \
      CPPUNIT_ASSERT_EQUAL(a[ai++], **i);                                      \
    }                                                                          \
  }

void IndexedListTest::testMove()
{
  int a[] = {0, 1, 2, 3, 4};
  IndexedList<int, int*> list;
  for (int i = 0; i < 5; ++i) {
    list.push_back(i, &a[i]);
  }
  CPPUNIT_ASSERT_EQUAL((ssize_t)-1, list.move(100, 0, OFFSET_MODE_SET));
  int a0[] = {0, 1, 2, 3, 4};
  CPPUNIT_ASSERT_EQUAL((ssize_t)0, list.move(0, 0, OFFSET_MODE_SET));
  LIST_CHECK(a0, list);

  int a1[] = {0, 2, 3, 4, 1};
  CPPUNIT_ASSERT_EQUAL((ssize_t)4, list.move(1, 4, OFFSET_MODE_SET));
  LIST_CHECK(a1, list);

  int a2[] = {0, 3, 4, 2, 1};
  CPPUNIT_ASSERT_EQUAL((ssize_t)3, list.move(2, 3, OFFSET_MODE_SET));
  LIST_CHECK(a2, list);

  int a3[] = {0, 2, 3, 4, 1};
  CPPUNIT_ASSERT_EQUAL((ssize_t)1, list.move(2, 1, OFFSET_MODE_SET));
  LIST_CHECK(a3, list);

  int a4[] = {1, 0, 2, 3, 4};
  CPPUNIT_ASSERT_EQUAL((ssize_t)0, list.move(1, 0, OFFSET_MODE_SET));
  LIST_CHECK(a4, list);

  int a5[] = {1, 0, 3, 2, 4};
  CPPUNIT_ASSERT_EQUAL((ssize_t)2, list.move(3, 2, OFFSET_MODE_SET));
  LIST_CHECK(a5, list);

  int a6[] = {1, 3, 2, 4, 0};
  CPPUNIT_ASSERT_EQUAL((ssize_t)4, list.move(0, 5, OFFSET_MODE_SET));
  LIST_CHECK(a6, list);

  int a7[] = {3, 1, 2, 4, 0};
  CPPUNIT_ASSERT_EQUAL((ssize_t)1, list.move(1, 1, OFFSET_MODE_CUR));
  LIST_CHECK(a7, list);

  int a8[] = {3, 2, 4, 1, 0};
  CPPUNIT_ASSERT_EQUAL((ssize_t)3, list.move(1, 2, OFFSET_MODE_CUR));
  LIST_CHECK(a8, list);

  int a9[] = {3, 2, 1, 4, 0};
  CPPUNIT_ASSERT_EQUAL((ssize_t)2, list.move(1, -1, OFFSET_MODE_CUR));
  LIST_CHECK(a9, list);

  int a10[] = {1, 3, 2, 4, 0};
  CPPUNIT_ASSERT_EQUAL((ssize_t)0, list.move(1, -1233, OFFSET_MODE_CUR));
  LIST_CHECK(a10, list);

  int a11[] = {3, 2, 4, 0, 1};
  CPPUNIT_ASSERT_EQUAL((ssize_t)4, list.move(1, 8733, OFFSET_MODE_CUR));
  LIST_CHECK(a11, list);

  int a12[] = {3, 2, 4, 0, 1};
  CPPUNIT_ASSERT_EQUAL((ssize_t)3, list.move(0, -1, OFFSET_MODE_END));
  LIST_CHECK(a12, list);

  int a13[] = {3, 2, 0, 4, 1};
  CPPUNIT_ASSERT_EQUAL((ssize_t)2, list.move(0, -2, OFFSET_MODE_END));
  LIST_CHECK(a13, list);

  int a14[] = {0, 3, 2, 4, 1};
  CPPUNIT_ASSERT_EQUAL((ssize_t)0, list.move(0, -8733, OFFSET_MODE_END));
  LIST_CHECK(a14, list);

  int a15[] = {0, 2, 4, 1, 3};
  CPPUNIT_ASSERT_EQUAL((ssize_t)4, list.move(3, 0, OFFSET_MODE_END));
  LIST_CHECK(a15, list);

  int a16[] = {2, 4, 1, 3, 0};
  CPPUNIT_ASSERT_EQUAL((ssize_t)4, list.move(0, 1000, OFFSET_MODE_END));
  LIST_CHECK(a16, list);

  int a17[] = {2, 1, 4, 3, 0};
  CPPUNIT_ASSERT_EQUAL((ssize_t)2, list.move(4, 2, OFFSET_MODE_SET));
  LIST_CHECK(a17, list);
}

void IndexedListTest::testGet()
{
  IndexedList<int, int*> list;
  int a = 1000;
  int b = 1;
  list.push_back(123, &a);
  list.push_back(1, &b);
  CPPUNIT_ASSERT_EQUAL((int*)nullptr, list.get(1000));
  CPPUNIT_ASSERT_EQUAL(&a, list.get(123));
}

namespace {
struct KeyFunc {
  int n;
  KeyFunc(int n) : n(n) {}
  int operator()(const std::shared_ptr<std::string>& x) { return n++; }
};
} // namespace

void IndexedListTest::testInsert_keyFunc()
{
  std::shared_ptr<std::string> s[] = {
      std::shared_ptr<std::string>(new std::string("a")),
      std::shared_ptr<std::string>(new std::string("b")),
      std::shared_ptr<std::string>(new std::string("c")),
      std::shared_ptr<std::string>(new std::string("d"))};
  size_t slen = sizeof(s) / sizeof(s[0]);
  IndexedList<int, std::shared_ptr<std::string>> list;
  list.insert(list.begin(), KeyFunc(0), std::begin(s), std::end(s));
  CPPUNIT_ASSERT_EQUAL((size_t)slen, list.size());
  for (size_t i = 0; i < slen; ++i) {
    CPPUNIT_ASSERT_EQUAL(*s[i], *list.get(i));
  }
  list.insert(list.begin() + 2, KeyFunc(slen), std::begin(s), std::end(s));
  CPPUNIT_ASSERT_EQUAL((size_t)slen * 2, list.size());
  for (size_t i = slen; i < slen * 2; ++i) {
    CPPUNIT_ASSERT_EQUAL(*s[i - slen], *list.get(i));
  }
  IndexedList<int, std::shared_ptr<std::string>>::iterator itr;
  itr = list.begin();
  CPPUNIT_ASSERT_EQUAL(std::string("a"), *(*itr++));
  CPPUNIT_ASSERT_EQUAL(std::string("b"), *(*itr++));
  CPPUNIT_ASSERT_EQUAL(std::string("a"), *(*itr++));
  CPPUNIT_ASSERT_EQUAL(std::string("b"), *(*itr++));
  CPPUNIT_ASSERT_EQUAL(std::string("c"), *(*itr++));
  CPPUNIT_ASSERT_EQUAL(std::string("d"), *(*itr++));
  CPPUNIT_ASSERT_EQUAL(std::string("c"), *(*itr++));
  CPPUNIT_ASSERT_EQUAL(std::string("d"), *(*itr++));

  list.insert(list.begin(), KeyFunc(2 * slen - 1), std::begin(s), std::end(s));
  CPPUNIT_ASSERT_EQUAL((size_t)slen * 3 - 1, list.size());
  itr = list.begin();
  CPPUNIT_ASSERT_EQUAL(std::string("b"), *(*itr++));
  CPPUNIT_ASSERT_EQUAL(std::string("c"), *(*itr++));
  CPPUNIT_ASSERT_EQUAL(std::string("d"), *(*itr++));
  CPPUNIT_ASSERT_EQUAL(std::string("a"), *(*itr++));
}

void IndexedListTest::testInsert()
{
  int a[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
  IndexedList<int, int*> list;
  IndexedList<int, int*>::iterator itr;
  CPPUNIT_ASSERT(list.end() == list.insert(1, 0, &a[5]));
  itr = list.insert(0, 5, &a[5]);
  CPPUNIT_ASSERT_EQUAL(5, **itr);
  itr = list.insert(1, 3, &a[3]);
  CPPUNIT_ASSERT_EQUAL(3, **itr);
  itr = list.insert(1, 4, &a[4]);
  CPPUNIT_ASSERT_EQUAL(4, **itr);
  itr = list.insert(0, 9, &a[9]);
  CPPUNIT_ASSERT_EQUAL(9, **itr);
  int a1[] = {9, 5, 4, 3};
  LIST_CHECK(a1, list);

  // use iterator to insert
  itr = list.insert(itr, 2, &a[2]);
  CPPUNIT_ASSERT_EQUAL(2, **itr);
  itr = list.insert(list.end(), 1, &a[1]);
  CPPUNIT_ASSERT_EQUAL(1, **itr);
  int a2[] = {2, 9, 5, 4, 3, 1};
  LIST_CHECK(a2, list);

  // 2 has been already added.
  CPPUNIT_ASSERT(list.end() == list.insert(list.end(), 2, &a[2]));
}

void IndexedListTest::testIterator()
{
  int a[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
  IndexedList<int, int*> list;
  IndexedList<int, int*>::iterator itr;
  IndexedList<int, int*>::const_iterator citr;
  for (auto& i : a) {
    CPPUNIT_ASSERT(list.push_back(i, &i));
  }
  CPPUNIT_ASSERT(list.begin() == list.begin());
  itr = list.begin();
  citr = list.begin();
  // operator*()
  CPPUNIT_ASSERT_EQUAL(&a[0], *itr);
  CPPUNIT_ASSERT_EQUAL(&a[0], *citr);
  // operator==(iterator, iterator)
  CPPUNIT_ASSERT(itr == list.begin());
  CPPUNIT_ASSERT(!(itr == list.end()));
  CPPUNIT_ASSERT(citr == list.begin());
  CPPUNIT_ASSERT(!(citr == list.end()));
  // operator++()
  ++itr;
  ++citr;
  // operator!=(iterator, iterator)
  CPPUNIT_ASSERT(itr != list.begin());
  CPPUNIT_ASSERT(!(itr != itr));
  CPPUNIT_ASSERT(citr != list.begin());
  CPPUNIT_ASSERT(!(citr != citr));
  // operator+(difference_type)
  CPPUNIT_ASSERT(itr == list.begin() + 1);
  CPPUNIT_ASSERT(citr == list.begin() + 1);
  // operator-(difference_type)
  CPPUNIT_ASSERT(itr - 1 == list.begin());
  CPPUNIT_ASSERT(citr - 1 == list.begin());
  // operator++(int)
  IndexedList<int, int*>::iterator itr2 = itr++;
  IndexedList<int, int*>::const_iterator citr2 = citr++;
  CPPUNIT_ASSERT(itr2 + 1 == itr);
  CPPUNIT_ASSERT(citr2 + 1 == citr);
  // operator+(difference_type, iterator)
  CPPUNIT_ASSERT(-1 + itr == itr2);
  CPPUNIT_ASSERT(-1 + citr == citr2);
  // operator<(iterator, iterator)
  CPPUNIT_ASSERT(list.begin() < itr);
  CPPUNIT_ASSERT(!(itr < list.begin()));
  CPPUNIT_ASSERT(list.begin() < citr);
  CPPUNIT_ASSERT(!(citr < list.begin()));
  // operator>(iterator, iterator)
  CPPUNIT_ASSERT(itr > list.begin());
  CPPUNIT_ASSERT(!(list.begin() > itr));
  CPPUNIT_ASSERT(citr > list.begin());
  CPPUNIT_ASSERT(!(list.begin() > citr));
  // operator<=(iterator, iterator)
  CPPUNIT_ASSERT(itr <= itr);
  CPPUNIT_ASSERT(list.begin() <= itr);
  CPPUNIT_ASSERT(!(itr <= list.begin()));
  CPPUNIT_ASSERT(citr <= citr);
  CPPUNIT_ASSERT(list.begin() <= citr);
  CPPUNIT_ASSERT(!(citr <= list.begin()));
  // operator>=(iterator, iterator)
  CPPUNIT_ASSERT(itr >= itr);
  CPPUNIT_ASSERT(itr >= list.begin());
  CPPUNIT_ASSERT(!(list.begin() >= itr));
  CPPUNIT_ASSERT(citr >= citr);
  CPPUNIT_ASSERT(citr >= list.begin());
  CPPUNIT_ASSERT(!(list.begin() >= citr));
  // operator-(iterator, iterator)
  CPPUNIT_ASSERT(2 == itr - list.begin());
  CPPUNIT_ASSERT(-2 == list.begin() - itr);
  CPPUNIT_ASSERT(2 == citr - list.begin());
  CPPUNIT_ASSERT(-2 == list.begin() - citr);
  // operator+=(difference_type)
  itr = list.begin();
  itr += 2;
  CPPUNIT_ASSERT(itr == list.begin() + 2);
  citr = list.begin();
  citr += 2;
  CPPUNIT_ASSERT(citr == list.begin() + 2);
  // operator-=(difference_type)
  itr -= 2;
  CPPUNIT_ASSERT(itr == list.begin());
  citr -= 2;
  CPPUNIT_ASSERT(citr == list.begin());
  // operator[](size_type)
  itr = list.begin();
  itr += 3;
  CPPUNIT_ASSERT_EQUAL(*(itr[1]), a[4]);
  citr = list.begin();
  citr += 3;
  CPPUNIT_ASSERT_EQUAL(*(citr[1]), a[4]);
}

namespace {
struct RemoveOdd {
  bool operator()(int* p) const { return *p % 2 == 1; }
};
} // namespace
void IndexedListTest::testRemoveIf()
{
  int a[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
  IndexedList<int, int*> list;
  for (auto& i : a) {
    CPPUNIT_ASSERT(list.push_back(i, &i));
  }
  list.remove_if(RemoveOdd());
  CPPUNIT_ASSERT_EQUAL((size_t)5, list.size());
  for (int i = 0; i < 5; ++i) {
    CPPUNIT_ASSERT_EQUAL(i * 2, *list[i]);
  }
}

} // namespace aria2
