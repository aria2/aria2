#include "uri_split.h"

#include <cstring>

#include <cppunit/extensions/HelperMacros.h>

#include "uri_split.h"

namespace aria2 {

class UriSplitTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(UriSplitTest);
  CPPUNIT_TEST(testUriSplit);
  CPPUNIT_TEST(testUriSplit_fail);
  CPPUNIT_TEST_SUITE_END();

public:
  void testUriSplit();
  void testUriSplit_fail();
};

CPPUNIT_TEST_SUITE_REGISTRATION(UriSplitTest);

namespace {
const char* fieldstr[] = {
    "USR_SCHEME",   "USR_HOST",     "USR_PORT", "USR_PATH",   "USR_QUERY",
    "USR_FRAGMENT", "USR_USERINFO", "USR_USER", "USR_PASSWD", "USR_BASENAME"};
} // namespace

#define CHECK_FIELD_SET(RES, FLAGS)                                            \
  for (int i = 0; i < USR_MAX; ++i) {                                          \
    int mask = 1 << i;                                                         \
    if ((FLAGS)&mask) {                                                        \
      CPPUNIT_ASSERT_MESSAGE(fieldstr[i], RES.field_set& mask);                \
    }                                                                          \
    else {                                                                     \
      CPPUNIT_ASSERT_MESSAGE(fieldstr[i], !(RES.field_set & mask));            \
    }                                                                          \
  }

namespace {
std::string mkstr(const uri_split_result& res, int field, const char* base)
{
  return std::string(base + res.fields[field].off, res.fields[field].len);
}
} // namespace

void UriSplitTest::testUriSplit()
{
  uri_split_result res;
  const char* uri;
  uri = "http://aria2.sf.net/path/";
  memset(&res, 0, sizeof(res));
  CPPUNIT_ASSERT_EQUAL(0, uri_split(&res, uri));
  CHECK_FIELD_SET(res, (1 << USR_SCHEME) | (1 << USR_HOST) | (1 << USR_PATH));
  CPPUNIT_ASSERT_EQUAL(std::string("http"), mkstr(res, USR_SCHEME, uri));
  CPPUNIT_ASSERT_EQUAL(std::string("aria2.sf.net"), mkstr(res, USR_HOST, uri));
  CPPUNIT_ASSERT_EQUAL(std::string("/path/"), mkstr(res, USR_PATH, uri));

  uri = "http://user@aria2.sf.net/path/";
  memset(&res, 0, sizeof(res));
  CPPUNIT_ASSERT_EQUAL(0, uri_split(&res, uri));
  CHECK_FIELD_SET(res, (1 << USR_SCHEME) | (1 << USR_HOST) | (1 << USR_PATH) |
                           (1 << USR_USERINFO) | (1 << USR_USER));
  CPPUNIT_ASSERT_EQUAL(std::string("aria2.sf.net"), mkstr(res, USR_HOST, uri));
  CPPUNIT_ASSERT_EQUAL(std::string("/path/"), mkstr(res, USR_PATH, uri));
  CPPUNIT_ASSERT_EQUAL(std::string("user"), mkstr(res, USR_USERINFO, uri));
  CPPUNIT_ASSERT_EQUAL(std::string("user"), mkstr(res, USR_USER, uri));

  uri = "http://user:pass@aria2.sf.net/path/";
  memset(&res, 0, sizeof(res));
  CPPUNIT_ASSERT_EQUAL(0, uri_split(&res, uri));
  CHECK_FIELD_SET(res, (1 << USR_SCHEME) | (1 << USR_HOST) | (1 << USR_PATH) |
                           (1 << USR_USERINFO) | (1 << USR_USER) |
                           (1 << USR_PASSWD));
  CPPUNIT_ASSERT_EQUAL(std::string("aria2.sf.net"), mkstr(res, USR_HOST, uri));
  CPPUNIT_ASSERT_EQUAL(std::string("/path/"), mkstr(res, USR_PATH, uri));
  CPPUNIT_ASSERT_EQUAL(std::string("user:pass"), mkstr(res, USR_USERINFO, uri));
  CPPUNIT_ASSERT_EQUAL(std::string("user"), mkstr(res, USR_USER, uri));
  CPPUNIT_ASSERT_EQUAL(std::string("pass"), mkstr(res, USR_PASSWD, uri));

  // According to RFC 3986, @ in userinfo is illegal. But many people
  // have e-mail account as username and don't understand
  // percent-encoding and keep getting erros putting it in URI in
  // unecoded form. Because of this, we support @ in username.
  uri = "http://user@foo.com:pass@aria2.sf.net/path/";
  memset(&res, 0, sizeof(res));
  CPPUNIT_ASSERT_EQUAL(0, uri_split(&res, uri));
  CHECK_FIELD_SET(res, (1 << USR_SCHEME) | (1 << USR_HOST) | (1 << USR_PATH) |
                           (1 << USR_USERINFO) | (1 << USR_USER) |
                           (1 << USR_PASSWD));
  CPPUNIT_ASSERT_EQUAL(std::string("aria2.sf.net"), mkstr(res, USR_HOST, uri));
  CPPUNIT_ASSERT_EQUAL(std::string("/path/"), mkstr(res, USR_PATH, uri));
  CPPUNIT_ASSERT_EQUAL(std::string("user@foo.com:pass"),
                       mkstr(res, USR_USERINFO, uri));
  CPPUNIT_ASSERT_EQUAL(std::string("user@foo.com"), mkstr(res, USR_USER, uri));
  CPPUNIT_ASSERT_EQUAL(std::string("pass"), mkstr(res, USR_PASSWD, uri));

  // Port processed in URI_MAYBE_USER -> URI_PORT
  uri = "https://aria2.sf.net:443/path/";
  memset(&res, 0, sizeof(res));
  CPPUNIT_ASSERT_EQUAL(0, uri_split(&res, uri));
  CHECK_FIELD_SET(res, (1 << USR_SCHEME) | (1 << USR_HOST) | (1 << USR_PORT) |
                           (1 << USR_PATH));
  CPPUNIT_ASSERT_EQUAL(std::string("https"), mkstr(res, USR_SCHEME, uri));
  CPPUNIT_ASSERT_EQUAL(std::string("aria2.sf.net"), mkstr(res, USR_HOST, uri));
  CPPUNIT_ASSERT_EQUAL(std::string("/path/"), mkstr(res, USR_PATH, uri));
  CPPUNIT_ASSERT_EQUAL((uint16_t)443, res.port);

  // Port processed in URI_PORT
  uri = "https://user:pass@aria2.sf.net:443/path/";
  memset(&res, 0, sizeof(res));
  CPPUNIT_ASSERT_EQUAL(0, uri_split(&res, uri));
  CHECK_FIELD_SET(res, (1 << USR_SCHEME) | (1 << USR_HOST) | (1 << USR_PORT) |
                           (1 << USR_PATH) | (1 << USR_USERINFO) |
                           (1 << USR_USER) | (1 << USR_PASSWD));
  CPPUNIT_ASSERT_EQUAL(std::string("aria2.sf.net"), mkstr(res, USR_HOST, uri));
  CPPUNIT_ASSERT_EQUAL(std::string("/path/"), mkstr(res, USR_PATH, uri));
  CPPUNIT_ASSERT_EQUAL(std::string("user:pass"), mkstr(res, USR_USERINFO, uri));
  CPPUNIT_ASSERT_EQUAL(std::string("user"), mkstr(res, USR_USER, uri));
  CPPUNIT_ASSERT_EQUAL(std::string("pass"), mkstr(res, USR_PASSWD, uri));
  CPPUNIT_ASSERT_EQUAL((uint16_t)443, res.port);

  // Port processed in URI_MAYBE_PASSWD
  uri = "https://user@aria2.sf.net:443/path/";
  memset(&res, 0, sizeof(res));
  CPPUNIT_ASSERT_EQUAL(0, uri_split(&res, uri));
  CHECK_FIELD_SET(res, (1 << USR_SCHEME) | (1 << USR_HOST) | (1 << USR_PORT) |
                           (1 << USR_PATH) | (1 << USR_USERINFO) |
                           (1 << USR_USER));
  CPPUNIT_ASSERT_EQUAL(std::string("aria2.sf.net"), mkstr(res, USR_HOST, uri));
  CPPUNIT_ASSERT_EQUAL(std::string("/path/"), mkstr(res, USR_PATH, uri));
  CPPUNIT_ASSERT_EQUAL(std::string("user"), mkstr(res, USR_USERINFO, uri));
  CPPUNIT_ASSERT_EQUAL(std::string("user"), mkstr(res, USR_USER, uri));
  CPPUNIT_ASSERT_EQUAL((uint16_t)443, res.port);

  // Port processed in URI_MAYBE_PASSWD
  uri = "http://aria2";
  memset(&res, 0, sizeof(res));
  CPPUNIT_ASSERT_EQUAL(0, uri_split(&res, uri));
  CHECK_FIELD_SET(res, (1 << USR_SCHEME) | (1 << USR_HOST));
  CPPUNIT_ASSERT_EQUAL(std::string("aria2"), mkstr(res, USR_HOST, uri));

  uri = "http://aria2:8080";
  memset(&res, 0, sizeof(res));
  CPPUNIT_ASSERT_EQUAL(0, uri_split(&res, uri));
  CHECK_FIELD_SET(res, (1 << USR_SCHEME) | (1 << USR_HOST) | (1 << USR_PORT));
  CPPUNIT_ASSERT_EQUAL(std::string("aria2"), mkstr(res, USR_HOST, uri));
  CPPUNIT_ASSERT_EQUAL((uint16_t)8080, res.port);

  uri = "http://user@aria2";
  memset(&res, 0, sizeof(res));
  CPPUNIT_ASSERT_EQUAL(0, uri_split(&res, uri));
  CHECK_FIELD_SET(res, (1 << USR_SCHEME) | (1 << USR_HOST) |
                           (1 << USR_USERINFO) | (1 << USR_USER));
  CPPUNIT_ASSERT_EQUAL(std::string("aria2"), mkstr(res, USR_HOST, uri));
  CPPUNIT_ASSERT_EQUAL(std::string("user"), mkstr(res, USR_USERINFO, uri));
  CPPUNIT_ASSERT_EQUAL(std::string("user"), mkstr(res, USR_USER, uri));

  uri = "http://user:@aria2";
  memset(&res, 0, sizeof(res));
  CPPUNIT_ASSERT_EQUAL(0, uri_split(&res, uri));
  CHECK_FIELD_SET(res, (1 << USR_SCHEME) | (1 << USR_HOST) |
                           (1 << USR_USERINFO) | (1 << USR_USER) |
                           (1 << USR_PASSWD));
  CPPUNIT_ASSERT_EQUAL(std::string("aria2"), mkstr(res, USR_HOST, uri));
  CPPUNIT_ASSERT_EQUAL(std::string("user:"), mkstr(res, USR_USERINFO, uri));
  CPPUNIT_ASSERT_EQUAL(std::string("user"), mkstr(res, USR_USER, uri));
  CPPUNIT_ASSERT_EQUAL(std::string(""), mkstr(res, USR_PASSWD, uri));

  uri = "http://aria2/?foo#bar";
  memset(&res, 0, sizeof(res));
  CPPUNIT_ASSERT_EQUAL(0, uri_split(&res, uri));
  CHECK_FIELD_SET(res, (1 << USR_SCHEME) | (1 << USR_HOST) | (1 << USR_PATH) |
                           (1 << USR_QUERY) | (1 << USR_FRAGMENT));
  CPPUNIT_ASSERT_EQUAL(std::string("aria2"), mkstr(res, USR_HOST, uri));
  CPPUNIT_ASSERT_EQUAL(std::string("/"), mkstr(res, USR_PATH, uri));
  CPPUNIT_ASSERT_EQUAL(std::string("foo"), mkstr(res, USR_QUERY, uri));
  CPPUNIT_ASSERT_EQUAL(std::string("bar"), mkstr(res, USR_FRAGMENT, uri));

  // URI_MAYBE_USER
  uri = "http://aria2?foo";
  memset(&res, 0, sizeof(res));
  CPPUNIT_ASSERT_EQUAL(0, uri_split(&res, uri));
  CHECK_FIELD_SET(res, (1 << USR_SCHEME) | (1 << USR_HOST) | (1 << USR_QUERY));
  CPPUNIT_ASSERT_EQUAL(std::string("aria2"), mkstr(res, USR_HOST, uri));
  CPPUNIT_ASSERT_EQUAL(std::string("foo"), mkstr(res, USR_QUERY, uri));

  // URI_MAYBE_USER
  uri = "http://aria2#bar";
  memset(&res, 0, sizeof(res));
  CPPUNIT_ASSERT_EQUAL(0, uri_split(&res, uri));
  CHECK_FIELD_SET(res,
                  (1 << USR_SCHEME) | (1 << USR_HOST) | (1 << USR_FRAGMENT));
  CPPUNIT_ASSERT_EQUAL(std::string("aria2"), mkstr(res, USR_HOST, uri));
  CPPUNIT_ASSERT_EQUAL(std::string("bar"), mkstr(res, USR_FRAGMENT, uri));

  // URI_MAYBE_PASSWD
  uri = "https://aria2:443?foo";
  memset(&res, 0, sizeof(res));
  CPPUNIT_ASSERT_EQUAL(0, uri_split(&res, uri));
  CHECK_FIELD_SET(res, (1 << USR_SCHEME) | (1 << USR_HOST) | (1 << USR_PORT) |
                           (1 << USR_QUERY));
  CPPUNIT_ASSERT_EQUAL(std::string("aria2"), mkstr(res, USR_HOST, uri));
  CPPUNIT_ASSERT_EQUAL(std::string("foo"), mkstr(res, USR_QUERY, uri));
  CPPUNIT_ASSERT_EQUAL((uint16_t)443, res.port);

  // URI_MAYBE_PASSWD
  uri = "https://aria2:443#bar";
  memset(&res, 0, sizeof(res));
  CPPUNIT_ASSERT_EQUAL(0, uri_split(&res, uri));
  CHECK_FIELD_SET(res, (1 << USR_SCHEME) | (1 << USR_HOST) | (1 << USR_PORT) |
                           (1 << USR_FRAGMENT));
  CPPUNIT_ASSERT_EQUAL(std::string("aria2"), mkstr(res, USR_HOST, uri));
  CPPUNIT_ASSERT_EQUAL(std::string("bar"), mkstr(res, USR_FRAGMENT, uri));
  CPPUNIT_ASSERT_EQUAL((uint16_t)443, res.port);

  // URI_PORT
  uri = "https://user:pass@aria2:443?foo";
  memset(&res, 0, sizeof(res));
  CPPUNIT_ASSERT_EQUAL(0, uri_split(&res, uri));
  CHECK_FIELD_SET(res, (1 << USR_SCHEME) | (1 << USR_HOST) | (1 << USR_PORT) |
                           (1 << USR_QUERY) | (1 << USR_USERINFO) |
                           (1 << USR_USER) | (1 << USR_PASSWD));
  CPPUNIT_ASSERT_EQUAL(std::string("aria2"), mkstr(res, USR_HOST, uri));
  CPPUNIT_ASSERT_EQUAL(std::string("user"), mkstr(res, USR_USER, uri));
  CPPUNIT_ASSERT_EQUAL(std::string("pass"), mkstr(res, USR_PASSWD, uri));
  CPPUNIT_ASSERT_EQUAL(std::string("foo"), mkstr(res, USR_QUERY, uri));
  CPPUNIT_ASSERT_EQUAL((uint16_t)443, res.port);

  // URI_PORT
  uri = "https://user:pass@aria2:443#bar";
  memset(&res, 0, sizeof(res));
  CPPUNIT_ASSERT_EQUAL(0, uri_split(&res, uri));
  CHECK_FIELD_SET(res, (1 << USR_SCHEME) | (1 << USR_HOST) | (1 << USR_PORT) |
                           (1 << USR_FRAGMENT) | (1 << USR_USERINFO) |
                           (1 << USR_USER) | (1 << USR_PASSWD));
  CPPUNIT_ASSERT_EQUAL(std::string("aria2"), mkstr(res, USR_HOST, uri));
  CPPUNIT_ASSERT_EQUAL(std::string("user"), mkstr(res, USR_USER, uri));
  CPPUNIT_ASSERT_EQUAL(std::string("pass"), mkstr(res, USR_PASSWD, uri));
  CPPUNIT_ASSERT_EQUAL(std::string("bar"), mkstr(res, USR_FRAGMENT, uri));
  CPPUNIT_ASSERT_EQUAL((uint16_t)443, res.port);

  // URI_HOST
  uri = "http://user:pass@aria2?foo";
  memset(&res, 0, sizeof(res));
  CPPUNIT_ASSERT_EQUAL(0, uri_split(&res, uri));
  CHECK_FIELD_SET(res, (1 << USR_SCHEME) | (1 << USR_HOST) | (1 << USR_QUERY) |
                           (1 << USR_USERINFO) | (1 << USR_USER) |
                           (1 << USR_PASSWD));
  CPPUNIT_ASSERT_EQUAL(std::string("aria2"), mkstr(res, USR_HOST, uri));
  CPPUNIT_ASSERT_EQUAL(std::string("user"), mkstr(res, USR_USER, uri));
  CPPUNIT_ASSERT_EQUAL(std::string("pass"), mkstr(res, USR_PASSWD, uri));
  CPPUNIT_ASSERT_EQUAL(std::string("foo"), mkstr(res, USR_QUERY, uri));

  // URI_HOST
  uri = "http://user:pass@aria2#bar";
  memset(&res, 0, sizeof(res));
  CPPUNIT_ASSERT_EQUAL(0, uri_split(&res, uri));
  CHECK_FIELD_SET(res, (1 << USR_SCHEME) | (1 << USR_HOST) |
                           (1 << USR_FRAGMENT) | (1 << USR_USERINFO) |
                           (1 << USR_USER) | (1 << USR_PASSWD));
  CPPUNIT_ASSERT_EQUAL(std::string("aria2"), mkstr(res, USR_HOST, uri));
  CPPUNIT_ASSERT_EQUAL(std::string("user"), mkstr(res, USR_USER, uri));
  CPPUNIT_ASSERT_EQUAL(std::string("pass"), mkstr(res, USR_PASSWD, uri));
  CPPUNIT_ASSERT_EQUAL(std::string("bar"), mkstr(res, USR_FRAGMENT, uri));

  // empty query
  uri = "http://aria2/?";
  memset(&res, 0, sizeof(res));
  CPPUNIT_ASSERT_EQUAL(0, uri_split(&res, uri));
  CHECK_FIELD_SET(res, (1 << USR_SCHEME) | (1 << USR_HOST) | (1 << USR_PATH) |
                           (1 << USR_QUERY));
  CPPUNIT_ASSERT_EQUAL(std::string("aria2"), mkstr(res, USR_HOST, uri));
  CPPUNIT_ASSERT_EQUAL(std::string("/"), mkstr(res, USR_PATH, uri));
  CPPUNIT_ASSERT_EQUAL(std::string(""), mkstr(res, USR_QUERY, uri));

  // empty fragment
  uri = "http://aria2/#";
  memset(&res, 0, sizeof(res));
  CPPUNIT_ASSERT_EQUAL(0, uri_split(&res, uri));
  CHECK_FIELD_SET(res, (1 << USR_SCHEME) | (1 << USR_HOST) | (1 << USR_PATH) |
                           (1 << USR_FRAGMENT));
  CPPUNIT_ASSERT_EQUAL(std::string("aria2"), mkstr(res, USR_HOST, uri));
  CPPUNIT_ASSERT_EQUAL(std::string("/"), mkstr(res, USR_PATH, uri));
  CPPUNIT_ASSERT_EQUAL(std::string(""), mkstr(res, USR_FRAGMENT, uri));

  // empty query and fragment
  uri = "http://aria2/?#";
  memset(&res, 0, sizeof(res));
  CPPUNIT_ASSERT_EQUAL(0, uri_split(&res, uri));
  CHECK_FIELD_SET(res, (1 << USR_SCHEME) | (1 << USR_HOST) | (1 << USR_PATH) |
                           (1 << USR_QUERY) | (1 << USR_FRAGMENT));
  CPPUNIT_ASSERT_EQUAL(std::string("aria2"), mkstr(res, USR_HOST, uri));
  CPPUNIT_ASSERT_EQUAL(std::string("/"), mkstr(res, USR_PATH, uri));
  CPPUNIT_ASSERT_EQUAL(std::string(""), mkstr(res, USR_QUERY, uri));
  CPPUNIT_ASSERT_EQUAL(std::string(""), mkstr(res, USR_FRAGMENT, uri));

  // IPv6 numeric address
  uri = "http://[::1]";
  memset(&res, 0, sizeof(res));
  CPPUNIT_ASSERT_EQUAL(0, uri_split(&res, uri));
  CHECK_FIELD_SET(res, (1 << USR_SCHEME) | (1 << USR_HOST));
  CPPUNIT_ASSERT_EQUAL(std::string("::1"), mkstr(res, USR_HOST, uri));
  CPPUNIT_ASSERT(res.flags & USF_IPV6ADDR);

  uri = "https://[::1]:443";
  memset(&res, 0, sizeof(res));
  CPPUNIT_ASSERT_EQUAL(0, uri_split(&res, uri));
  CHECK_FIELD_SET(res, (1 << USR_SCHEME) | (1 << USR_HOST) | (1 << USR_PORT));
  CPPUNIT_ASSERT_EQUAL(std::string("::1"), mkstr(res, USR_HOST, uri));
  CPPUNIT_ASSERT_EQUAL((uint16_t)443, res.port);
  CPPUNIT_ASSERT(res.flags & USF_IPV6ADDR);

  // USR_MAYBE_USER
  uri = "https://user@[::1]";
  memset(&res, 0, sizeof(res));
  CPPUNIT_ASSERT_EQUAL(0, uri_split(&res, uri));
  CHECK_FIELD_SET(res, (1 << USR_SCHEME) | (1 << USR_HOST) |
                           (1 << USR_USERINFO) | (1 << USR_USER));
  CPPUNIT_ASSERT_EQUAL(std::string("::1"), mkstr(res, USR_HOST, uri));
  CPPUNIT_ASSERT_EQUAL(std::string("user"), mkstr(res, USR_USERINFO, uri));
  CPPUNIT_ASSERT_EQUAL(std::string("user"), mkstr(res, USR_USER, uri));
  CPPUNIT_ASSERT(res.flags & USF_IPV6ADDR);

  // USR_BEFORE_HOST
  uri = "https://user:pass@[::1]";
  memset(&res, 0, sizeof(res));
  CPPUNIT_ASSERT_EQUAL(0, uri_split(&res, uri));
  CHECK_FIELD_SET(res, (1 << USR_SCHEME) | (1 << USR_HOST) |
                           (1 << USR_USERINFO) | (1 << USR_USER) |
                           (1 << USR_PASSWD));
  CPPUNIT_ASSERT_EQUAL(std::string("::1"), mkstr(res, USR_HOST, uri));
  CPPUNIT_ASSERT_EQUAL(std::string("user:pass"), mkstr(res, USR_USERINFO, uri));
  CPPUNIT_ASSERT_EQUAL(std::string("user"), mkstr(res, USR_USER, uri));
  CPPUNIT_ASSERT_EQUAL(std::string("pass"), mkstr(res, USR_PASSWD, uri));
  CPPUNIT_ASSERT(res.flags & USF_IPV6ADDR);

  uri = "http://aria2/f";
  memset(&res, 0, sizeof(res));
  CPPUNIT_ASSERT_EQUAL(0, uri_split(&res, uri));
  CHECK_FIELD_SET(res, (1 << USR_SCHEME) | (1 << USR_HOST) | (1 << USR_PATH) |
                           (1 << USR_BASENAME));
  CPPUNIT_ASSERT_EQUAL(std::string("/f"), mkstr(res, USR_PATH, uri));
  CPPUNIT_ASSERT_EQUAL(std::string("f"), mkstr(res, USR_BASENAME, uri));

  uri = "http://[::1]/f";
  memset(&res, 0, sizeof(res));
  CPPUNIT_ASSERT_EQUAL(0, uri_split(&res, uri));
  CHECK_FIELD_SET(res, (1 << USR_SCHEME) | (1 << USR_HOST) | (1 << USR_PATH) |
                           (1 << USR_BASENAME));
  CPPUNIT_ASSERT_EQUAL(std::string("::1"), mkstr(res, USR_HOST, uri));
  CPPUNIT_ASSERT_EQUAL(std::string("/f"), mkstr(res, USR_PATH, uri));
  CPPUNIT_ASSERT_EQUAL(std::string("f"), mkstr(res, USR_BASENAME, uri));

  uri = "http://[::1]:8080/f";
  memset(&res, 0, sizeof(res));
  CPPUNIT_ASSERT_EQUAL(0, uri_split(&res, uri));
  CHECK_FIELD_SET(res, (1 << USR_SCHEME) | (1 << USR_HOST) | (1 << USR_PORT) |
                           (1 << USR_PATH) | (1 << USR_BASENAME));
  CPPUNIT_ASSERT_EQUAL((uint16_t)8080, res.port);
  CPPUNIT_ASSERT_EQUAL(std::string("/f"), mkstr(res, USR_PATH, uri));
  CPPUNIT_ASSERT_EQUAL(std::string("f"), mkstr(res, USR_BASENAME, uri));

  uri = "https://user:pass@host/f";
  memset(&res, 0, sizeof(res));
  CPPUNIT_ASSERT_EQUAL(0, uri_split(&res, uri));
  CHECK_FIELD_SET(res, (1 << USR_SCHEME) | (1 << USR_HOST) |
                           (1 << USR_USERINFO) | (1 << USR_USER) |
                           (1 << USR_PASSWD) | (1 << USR_PATH) |
                           (1 << USR_BASENAME));
  CPPUNIT_ASSERT_EQUAL(std::string("host"), mkstr(res, USR_HOST, uri));
  CPPUNIT_ASSERT_EQUAL(std::string("user:pass"), mkstr(res, USR_USERINFO, uri));
  CPPUNIT_ASSERT_EQUAL(std::string("/f"), mkstr(res, USR_PATH, uri));
  CPPUNIT_ASSERT_EQUAL(std::string("f"), mkstr(res, USR_BASENAME, uri));

  uri = "http://aria2/index.html?foo";
  memset(&res, 0, sizeof(res));
  CPPUNIT_ASSERT_EQUAL(0, uri_split(&res, uri));
  CHECK_FIELD_SET(res, (1 << USR_SCHEME) | (1 << USR_HOST) | (1 << USR_PATH) |
                           (1 << USR_QUERY) | (1 << USR_BASENAME));
  CPPUNIT_ASSERT_EQUAL(std::string("/index.html"), mkstr(res, USR_PATH, uri));
  CPPUNIT_ASSERT_EQUAL(std::string("index.html"),
                       mkstr(res, USR_BASENAME, uri));
  CPPUNIT_ASSERT_EQUAL(std::string("foo"), mkstr(res, USR_QUERY, uri));
}

void UriSplitTest::testUriSplit_fail()
{
  CPPUNIT_ASSERT_EQUAL(-1, uri_split(nullptr, ""));
  CPPUNIT_ASSERT_EQUAL(-1, uri_split(nullptr, "h"));
  CPPUNIT_ASSERT_EQUAL(-1, uri_split(nullptr, "http:"));
  CPPUNIT_ASSERT_EQUAL(-1, uri_split(nullptr, "http:a"));
  CPPUNIT_ASSERT_EQUAL(-1, uri_split(nullptr, "http:/"));
  CPPUNIT_ASSERT_EQUAL(-1, uri_split(nullptr, "http://"));
  CPPUNIT_ASSERT_EQUAL(-1, uri_split(nullptr, "http:/a"));
  CPPUNIT_ASSERT_EQUAL(-1, uri_split(nullptr, "http://:host"));
  CPPUNIT_ASSERT_EQUAL(-1, uri_split(nullptr, "http://@user@host"));
  CPPUNIT_ASSERT_EQUAL(-1, uri_split(nullptr, "http://user:"));
  CPPUNIT_ASSERT_EQUAL(-1, uri_split(nullptr, "http://user:pass"));
  CPPUNIT_ASSERT_EQUAL(-1, uri_split(nullptr, "http://user:65536"));
  CPPUNIT_ASSERT_EQUAL(-1, uri_split(nullptr, "http://user:pass?"));
  CPPUNIT_ASSERT_EQUAL(-1, uri_split(nullptr, "http://user:pass@host:65536"));
  CPPUNIT_ASSERT_EQUAL(-1, uri_split(nullptr, "http://user:pass@host:x"));
  CPPUNIT_ASSERT_EQUAL(-1, uri_split(nullptr, "http://user:pass@host:80x"));
  CPPUNIT_ASSERT_EQUAL(-1, uri_split(nullptr, "http://user@"));
  CPPUNIT_ASSERT_EQUAL(-1, uri_split(nullptr, "http://[]"));
  CPPUNIT_ASSERT_EQUAL(-1, uri_split(nullptr, "http://[::"));
  CPPUNIT_ASSERT_EQUAL(-1, uri_split(nullptr, "http://user[::1]"));
  CPPUNIT_ASSERT_EQUAL(-1, uri_split(nullptr, "http://user[::1]x"));
  CPPUNIT_ASSERT_EQUAL(-1, uri_split(nullptr, "http://user:pass[::1]"));
}

} // namespace aria2
