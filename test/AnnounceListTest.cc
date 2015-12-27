#include "AnnounceList.h"

#include <cppunit/extensions/HelperMacros.h>

#include "Exception.h"
#include "bencode2.h"

namespace aria2 {

class AnnounceListTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(AnnounceListTest);
  CPPUNIT_TEST(testSingleElementList);
  CPPUNIT_TEST(testMultiElementList);
  CPPUNIT_TEST(testSingleAndMulti);
  CPPUNIT_TEST(testNoGroup);
  CPPUNIT_TEST(testEvent);
  CPPUNIT_TEST(testNextEventIfAfterStarted);
  CPPUNIT_TEST(testCountStoppedAllowedTier);
  CPPUNIT_TEST(testCountCompletedAllowedTier);
  CPPUNIT_TEST(testMoveToStoppedAllowedTier);
  CPPUNIT_TEST(testMoveToCompletedAllowedTier);
  CPPUNIT_TEST_SUITE_END();

private:
public:
  void setUp() {}

  void testSingleElementList();
  void testMultiElementList();
  void testSingleAndMulti();
  void testNoGroup();
  void testEvent();
  void testNextEventIfAfterStarted();
  void testCountStoppedAllowedTier();
  void testCountCompletedAllowedTier();
  void testMoveToStoppedAllowedTier();
  void testMoveToCompletedAllowedTier();
};

CPPUNIT_TEST_SUITE_REGISTRATION(AnnounceListTest);

namespace {
std::vector<std::vector<std::string>> toVector(const List* announceList)
{
  std::vector<std::vector<std::string>> dest;
  for (List::ValueType::const_iterator tierIter = announceList->begin(),
                                       eoi = announceList->end();
       tierIter != eoi; ++tierIter) {
    std::vector<std::string> ntier;
    const List* tier = downcast<List>(*tierIter);
    for (List::ValueType::const_iterator uriIter = tier->begin(),
                                         eoi2 = tier->end();
         uriIter != eoi2; ++uriIter) {
      const String* uri = downcast<String>(*uriIter);
      ntier.push_back(uri->s());
    }
    dest.push_back(ntier);
  }
  return dest;
}
} // namespace

void AnnounceListTest::testSingleElementList()
{
  std::string peersString = "ll8:tracker1el8:tracker2el8:tracker3ee";
  std::shared_ptr<ValueBase> announcesList = bencode2::decode(peersString);

  // ANNOUNCE_LIST
  // [ [ tracker1 ], [ tracker2 ], [ tracker3 ] ]
  AnnounceList announceList(toVector(downcast<List>(announcesList)));

  CPPUNIT_ASSERT(!announceList.allTiersFailed());
  std::string url = announceList.getAnnounce();
  std::string event = announceList.getEventString();
  CPPUNIT_ASSERT_EQUAL(std::string("tracker1"), url);
  CPPUNIT_ASSERT_EQUAL(std::string("started"), event);
  announceList.announceFailure();
  url = announceList.getAnnounce();
  CPPUNIT_ASSERT_EQUAL(std::string("tracker2"), url);
  announceList.announceFailure();
  url = announceList.getAnnounce();
  CPPUNIT_ASSERT_EQUAL(std::string("tracker3"), url);
  announceList.announceFailure();
  CPPUNIT_ASSERT(announceList.allTiersFailed());
  announceList.resetTier();
  CPPUNIT_ASSERT(!announceList.allTiersFailed());
  // back to the first list
  url = announceList.getAnnounce();
  event = announceList.getEventString();
  CPPUNIT_ASSERT_EQUAL(std::string("tracker1"), url);
  CPPUNIT_ASSERT_EQUAL(std::string("started"), event);
  announceList.announceFailure();
  url = announceList.getAnnounce();
  event = announceList.getEventString();
  CPPUNIT_ASSERT_EQUAL(std::string("tracker2"), url);
  CPPUNIT_ASSERT_EQUAL(std::string("started"), event);
  announceList.announceSuccess();
  // back to the first list because announce to tracker2 succeeded.
  url = announceList.getAnnounce();
  event = announceList.getEventString();
  CPPUNIT_ASSERT_EQUAL(std::string("tracker1"), url);
  CPPUNIT_ASSERT_EQUAL(std::string("started"), event);
  announceList.announceFailure();
  url = announceList.getAnnounce();
  event = announceList.getEventString();
  CPPUNIT_ASSERT_EQUAL(std::string("tracker2"), url);
  CPPUNIT_ASSERT_EQUAL(std::string(""), event);
}

void AnnounceListTest::testMultiElementList()
{
  std::string peersString = "ll8:tracker18:tracker28:tracker3ee";
  std::shared_ptr<ValueBase> announcesList = bencode2::decode(peersString);

  // ANNOUNCE_LIST
  // [ [ tracker1, tracker2, tracker3 ] ]
  AnnounceList announceList(toVector(downcast<List>(announcesList)));

  CPPUNIT_ASSERT(!announceList.allTiersFailed());
  std::string url = announceList.getAnnounce();
  CPPUNIT_ASSERT_EQUAL(std::string("tracker1"), url);
  announceList.announceFailure();
  url = announceList.getAnnounce();
  CPPUNIT_ASSERT_EQUAL(std::string("tracker2"), url);
  announceList.announceSuccess();
  url = announceList.getAnnounce();
  // tracker2 returns because tracker2 is now first.
  CPPUNIT_ASSERT_EQUAL(std::string("tracker2"), url);
  announceList.announceFailure();
  url = announceList.getAnnounce();
  CPPUNIT_ASSERT_EQUAL(std::string("tracker1"), url);
  announceList.announceFailure();
  url = announceList.getAnnounce();
  CPPUNIT_ASSERT_EQUAL(std::string("tracker3"), url);
  announceList.announceFailure();
  CPPUNIT_ASSERT(announceList.allTiersFailed());
  announceList.resetTier();
  CPPUNIT_ASSERT(!announceList.allTiersFailed());
  // back to the first list because there is no other list.
  url = announceList.getAnnounce();
  CPPUNIT_ASSERT_EQUAL(std::string("tracker2"), url);
}

void AnnounceListTest::testSingleAndMulti()
{
  std::string peersString = "ll8:tracker18:tracker2el8:tracker3ee";
  std::shared_ptr<ValueBase> announcesList = bencode2::decode(peersString);

  // ANNOUNCE_LIST
  // [ [ tracker1, tracker2 ], [ tracker3 ] ]
  AnnounceList announceList(toVector(downcast<List>(announcesList)));

  std::string url = announceList.getAnnounce();
  CPPUNIT_ASSERT_EQUAL(std::string("tracker1"), url);
  announceList.announceSuccess();
  url = announceList.getAnnounce();
  CPPUNIT_ASSERT_EQUAL(std::string("tracker1"), url);
  announceList.announceFailure();
  url = announceList.getAnnounce();
  CPPUNIT_ASSERT_EQUAL(std::string("tracker2"), url);
  announceList.announceFailure();
  url = announceList.getAnnounce();
  CPPUNIT_ASSERT_EQUAL(std::string("tracker3"), url);
  announceList.announceSuccess();
  url = announceList.getAnnounce();
  // tracker1 returns because after the announce to tracker3 succeeds, list
  // pointer points to the first list.
  CPPUNIT_ASSERT_EQUAL(std::string("tracker1"), url);
}

void AnnounceListTest::testNoGroup()
{
  std::string peersString = "llee";
  std::shared_ptr<ValueBase> announcesList = bencode2::decode(peersString);
  AnnounceList announceList(toVector(downcast<List>(announcesList)));
  CPPUNIT_ASSERT(announceList.countTier() == 0);
}

void AnnounceListTest::testNextEventIfAfterStarted()
{
  std::string peersString = "ll8:tracker1ee";
  std::shared_ptr<ValueBase> announcesList = bencode2::decode(peersString);

  // ANNOUNCE_LIST
  // [ [ tracker1 ] ]
  AnnounceList announceList(toVector(downcast<List>(announcesList)));
  announceList.setEvent(AnnounceTier::STOPPED);
  announceList.announceFailure();
  announceList.resetTier();
  CPPUNIT_ASSERT_EQUAL(std::string(""),
                       std::string(announceList.getEventString()));
  CPPUNIT_ASSERT_EQUAL(AnnounceTier::HALTED, announceList.getEvent());

  announceList.setEvent(AnnounceTier::COMPLETED);
  announceList.announceFailure();
  announceList.resetTier();
  CPPUNIT_ASSERT_EQUAL(std::string(""),
                       std::string(announceList.getEventString()));
  CPPUNIT_ASSERT_EQUAL(AnnounceTier::SEEDING, announceList.getEvent());
}

void AnnounceListTest::testEvent()
{
  std::string peersString = "ll8:tracker1el8:tracker2el8:tracker3ee";
  std::shared_ptr<ValueBase> announcesList = bencode2::decode(peersString);

  // ANNOUNCE_LIST
  // [ [ tracker1 ], [ tracker2 ], [ tracker3 ] ]
  AnnounceList announceList(toVector(downcast<List>(announcesList)));

  announceList.setEvent(AnnounceTier::STOPPED);
  announceList.announceSuccess();
  CPPUNIT_ASSERT_EQUAL(std::string(""),
                       std::string(announceList.getEventString()));
  CPPUNIT_ASSERT_EQUAL(AnnounceTier::HALTED, announceList.getEvent());

  announceList.setEvent(AnnounceTier::COMPLETED);
  announceList.announceSuccess();
  CPPUNIT_ASSERT_EQUAL(std::string(""),
                       std::string(announceList.getEventString()));
  CPPUNIT_ASSERT_EQUAL(AnnounceTier::SEEDING, announceList.getEvent());

  announceList.setEvent(AnnounceTier::STARTED_AFTER_COMPLETION);
  CPPUNIT_ASSERT_EQUAL(std::string("started"),
                       std::string(announceList.getEventString()));
  announceList.announceSuccess();
  CPPUNIT_ASSERT_EQUAL(AnnounceTier::SEEDING, announceList.getEvent());
}

void AnnounceListTest::testCountStoppedAllowedTier()
{
  std::string peersString = "ll8:tracker1el8:tracker2el8:tracker3ee";
  std::shared_ptr<ValueBase> announcesList = bencode2::decode(peersString);

  // ANNOUNCE_LIST
  // [ [ tracker1 ], [ tracker2 ], [ tracker3 ] ]
  AnnounceList announceList(toVector(downcast<List>(announcesList)));

  CPPUNIT_ASSERT_EQUAL((size_t)0, announceList.countStoppedAllowedTier());
  announceList.setEvent(AnnounceTier::STARTED);
  CPPUNIT_ASSERT_EQUAL((size_t)0, announceList.countStoppedAllowedTier());
  announceList.setEvent(AnnounceTier::STARTED_AFTER_COMPLETION);
  CPPUNIT_ASSERT_EQUAL((size_t)0, announceList.countStoppedAllowedTier());
  announceList.setEvent(AnnounceTier::HALTED);
  CPPUNIT_ASSERT_EQUAL((size_t)0, announceList.countStoppedAllowedTier());

  announceList.setEvent(AnnounceTier::DOWNLOADING);
  CPPUNIT_ASSERT_EQUAL((size_t)1, announceList.countStoppedAllowedTier());
  announceList.setEvent(AnnounceTier::STOPPED);
  CPPUNIT_ASSERT_EQUAL((size_t)1, announceList.countStoppedAllowedTier());
  announceList.setEvent(AnnounceTier::COMPLETED);
  CPPUNIT_ASSERT_EQUAL((size_t)1, announceList.countStoppedAllowedTier());
  announceList.setEvent(AnnounceTier::SEEDING);
  CPPUNIT_ASSERT_EQUAL((size_t)1, announceList.countStoppedAllowedTier());
}

void AnnounceListTest::testCountCompletedAllowedTier()
{
  std::string peersString = "ll8:tracker1el8:tracker2el8:tracker3ee";
  std::shared_ptr<ValueBase> announcesList = bencode2::decode(peersString);

  // ANNOUNCE_LIST
  // [ [ tracker1 ], [ tracker2 ], [ tracker3 ] ]
  AnnounceList announceList(toVector(downcast<List>(announcesList)));

  CPPUNIT_ASSERT_EQUAL((size_t)0, announceList.countCompletedAllowedTier());
  announceList.setEvent(AnnounceTier::STARTED);
  CPPUNIT_ASSERT_EQUAL((size_t)0, announceList.countCompletedAllowedTier());
  announceList.setEvent(AnnounceTier::STARTED_AFTER_COMPLETION);
  CPPUNIT_ASSERT_EQUAL((size_t)0, announceList.countCompletedAllowedTier());
  announceList.setEvent(AnnounceTier::STOPPED);
  CPPUNIT_ASSERT_EQUAL((size_t)0, announceList.countCompletedAllowedTier());
  announceList.setEvent(AnnounceTier::SEEDING);
  CPPUNIT_ASSERT_EQUAL((size_t)0, announceList.countCompletedAllowedTier());
  announceList.setEvent(AnnounceTier::HALTED);
  CPPUNIT_ASSERT_EQUAL((size_t)0, announceList.countCompletedAllowedTier());

  announceList.setEvent(AnnounceTier::DOWNLOADING);
  CPPUNIT_ASSERT_EQUAL((size_t)1, announceList.countCompletedAllowedTier());
  announceList.setEvent(AnnounceTier::COMPLETED);
  CPPUNIT_ASSERT_EQUAL((size_t)1, announceList.countCompletedAllowedTier());
}

std::deque<std::string> createUrls(const std::string& url)
{
  std::deque<std::string> urls;
  urls.push_back(url);
  return urls;
}

void AnnounceListTest::testMoveToStoppedAllowedTier()
{
  std::shared_ptr<AnnounceTier> t1(new AnnounceTier(createUrls("tracker1")));
  std::shared_ptr<AnnounceTier> t2(new AnnounceTier(createUrls("tracker2")));
  t2->event = AnnounceTier::COMPLETED;
  std::shared_ptr<AnnounceTier> t3(new AnnounceTier(createUrls("tracker3")));

  std::deque<std::shared_ptr<AnnounceTier>> tiers;
  tiers.push_back(t1);
  tiers.push_back(t2);
  tiers.push_back(t3);

  AnnounceList announceList(tiers);

  CPPUNIT_ASSERT(!announceList.currentTierAcceptsStoppedEvent());
  CPPUNIT_ASSERT_EQUAL(std::string("tracker1"), announceList.getAnnounce());
  announceList.moveToStoppedAllowedTier();
  CPPUNIT_ASSERT(announceList.currentTierAcceptsStoppedEvent());
  CPPUNIT_ASSERT_EQUAL(std::string("tracker2"), announceList.getAnnounce());
  announceList.announceFailure();
  CPPUNIT_ASSERT(!announceList.currentTierAcceptsStoppedEvent());
  CPPUNIT_ASSERT_EQUAL(std::string("tracker3"), announceList.getAnnounce());
  // test wrapped search
  announceList.moveToStoppedAllowedTier();
  CPPUNIT_ASSERT_EQUAL(std::string("tracker2"), announceList.getAnnounce());
}

void AnnounceListTest::testMoveToCompletedAllowedTier()
{
  std::shared_ptr<AnnounceTier> t1(new AnnounceTier(createUrls("tracker1")));
  std::shared_ptr<AnnounceTier> t2(new AnnounceTier(createUrls("tracker2")));
  t2->event = AnnounceTier::COMPLETED;
  std::shared_ptr<AnnounceTier> t3(new AnnounceTier(createUrls("tracker3")));

  std::deque<std::shared_ptr<AnnounceTier>> tiers;
  tiers.push_back(t1);
  tiers.push_back(t2);
  tiers.push_back(t3);

  AnnounceList announceList(tiers);

  CPPUNIT_ASSERT(!announceList.currentTierAcceptsCompletedEvent());
  CPPUNIT_ASSERT_EQUAL(std::string("tracker1"), announceList.getAnnounce());
  announceList.moveToStoppedAllowedTier();
  CPPUNIT_ASSERT(announceList.currentTierAcceptsCompletedEvent());
  CPPUNIT_ASSERT_EQUAL(std::string("tracker2"), announceList.getAnnounce());
  announceList.announceFailure();
  CPPUNIT_ASSERT(!announceList.currentTierAcceptsCompletedEvent());
  CPPUNIT_ASSERT_EQUAL(std::string("tracker3"), announceList.getAnnounce());
  // test wrapped search
  announceList.moveToStoppedAllowedTier();
  CPPUNIT_ASSERT_EQUAL(std::string("tracker2"), announceList.getAnnounce());
}

} // namespace aria2
