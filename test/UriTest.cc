#include "uri.h"

#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

namespace uri {

class UriTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(UriTest);
  CPPUNIT_TEST(testSetUri1);
  CPPUNIT_TEST(testSetUri2);
  CPPUNIT_TEST(testSetUri3);
  CPPUNIT_TEST(testSetUri4);
  CPPUNIT_TEST(testSetUri5);
  CPPUNIT_TEST(testSetUri6);
  CPPUNIT_TEST(testSetUri7);
  CPPUNIT_TEST(testSetUri8);
  CPPUNIT_TEST(testSetUri9);
  CPPUNIT_TEST(testSetUri10);
  CPPUNIT_TEST(testSetUri11);
  CPPUNIT_TEST(testSetUri12);
  CPPUNIT_TEST(testSetUri13);
  CPPUNIT_TEST(testSetUri14);
  CPPUNIT_TEST(testSetUri15);
  CPPUNIT_TEST(testSetUri16);
  CPPUNIT_TEST(testSetUri18);
  CPPUNIT_TEST(testSetUri19);
  CPPUNIT_TEST(testSetUri20);
  CPPUNIT_TEST(testSetUri_username);
  CPPUNIT_TEST(testSetUri_usernamePassword);
  CPPUNIT_TEST(testSetUri_zeroUsername);
  CPPUNIT_TEST(testSetUri_ipv6);
  CPPUNIT_TEST(testInnerLink);
  CPPUNIT_TEST(testConstruct);
  CPPUNIT_TEST(testSwap);
  CPPUNIT_TEST(testJoinUri);
  CPPUNIT_TEST_SUITE_END();
  
public:
  void testSetUri1();
  void testSetUri2();
  void testSetUri3();
  void testSetUri4();
  void testSetUri5();
  void testSetUri6();
  void testSetUri7();
  void testSetUri8();
  void testSetUri9();
  void testSetUri10();
  void testSetUri11();
  void testSetUri12();
  void testSetUri13();
  void testSetUri14();
  void testSetUri15();
  void testSetUri16();
  void testSetUri18();
  void testSetUri19();
  void testSetUri20();
  void testSetUri_username();
  void testSetUri_usernamePassword();
  void testSetUri_zeroUsername();
  void testSetUri_ipv6();
  void testInnerLink();
  void testConstruct();
  void testSwap();
  void testJoinUri();
};


CPPUNIT_TEST_SUITE_REGISTRATION( UriTest );

void UriTest::testSetUri1()
{
  UriStruct us;
  bool v = parse(us, "http://aria.rednoah.com/");

  CPPUNIT_ASSERT(v);
  CPPUNIT_ASSERT_EQUAL(std::string("http"), us.protocol);
  CPPUNIT_ASSERT_EQUAL((uint16_t)80, us.port);
  CPPUNIT_ASSERT_EQUAL(std::string("aria.rednoah.com"), us.host);
  CPPUNIT_ASSERT_EQUAL(std::string("/"), us.dir);
  CPPUNIT_ASSERT_EQUAL(std::string(""), us.file);
  CPPUNIT_ASSERT_EQUAL(std::string(""), us.query);
  CPPUNIT_ASSERT_EQUAL(std::string(""), us.username);
  CPPUNIT_ASSERT_EQUAL(std::string(""), us.password);
  CPPUNIT_ASSERT(!us.ipv6LiteralAddress);
}

void UriTest::testSetUri2()
{
  UriStruct us;
  bool v = parse(us, "http://aria.rednoah.com:8080/index.html");

  CPPUNIT_ASSERT(v);
  CPPUNIT_ASSERT_EQUAL(std::string("http"), us.protocol);
  CPPUNIT_ASSERT_EQUAL((uint16_t)8080, us.port);
  CPPUNIT_ASSERT_EQUAL(std::string("aria.rednoah.com"), us.host);
  CPPUNIT_ASSERT_EQUAL(std::string("/"), us.dir);
  CPPUNIT_ASSERT_EQUAL(std::string("index.html"), us.file);
  CPPUNIT_ASSERT_EQUAL(std::string(""), us.query);
}

void UriTest::testSetUri3()
{
  UriStruct us;
  bool v = parse(us, "http://aria.rednoah.com/aria2/index.html");

  CPPUNIT_ASSERT(v);
  CPPUNIT_ASSERT_EQUAL(std::string("http"), us.protocol);
  CPPUNIT_ASSERT_EQUAL((uint16_t)80, us.port);
  CPPUNIT_ASSERT_EQUAL(std::string("aria.rednoah.com"), us.host);
  CPPUNIT_ASSERT_EQUAL(std::string("/aria2/"), us.dir);
  CPPUNIT_ASSERT_EQUAL(std::string("index.html"), us.file);
  CPPUNIT_ASSERT_EQUAL(std::string(""), us.query);
}

void UriTest::testSetUri4()
{
  UriStruct us;
  bool v = parse(us, "http://aria.rednoah.com/aria2/aria3/index.html");

  CPPUNIT_ASSERT(v);
  CPPUNIT_ASSERT_EQUAL(std::string("http"), us.protocol);
  CPPUNIT_ASSERT_EQUAL((uint16_t)80, us.port);
  CPPUNIT_ASSERT_EQUAL(std::string("aria.rednoah.com"), us.host);
  CPPUNIT_ASSERT_EQUAL(std::string("/aria2/aria3/"), us.dir);
  CPPUNIT_ASSERT_EQUAL(std::string("index.html"), us.file);
  CPPUNIT_ASSERT_EQUAL(std::string(""), us.query);
}

void UriTest::testSetUri5()
{
  UriStruct us;
  bool v = parse(us, "http://aria.rednoah.com/aria2/aria3/");

  CPPUNIT_ASSERT(v);
  CPPUNIT_ASSERT_EQUAL(std::string("http"), us.protocol);
  CPPUNIT_ASSERT_EQUAL((uint16_t)80, us.port);
  CPPUNIT_ASSERT_EQUAL(std::string("aria.rednoah.com"), us.host);
  CPPUNIT_ASSERT_EQUAL(std::string("/aria2/aria3/"), us.dir);
  CPPUNIT_ASSERT_EQUAL(std::string(""), us.file);
  CPPUNIT_ASSERT_EQUAL(std::string(""), us.query);
}

void UriTest::testSetUri6()
{
  UriStruct us;
  bool v = parse(us, "http://aria.rednoah.com/aria2/aria3");

  CPPUNIT_ASSERT(v);
  CPPUNIT_ASSERT_EQUAL(std::string("http"), us.protocol);
  CPPUNIT_ASSERT_EQUAL((uint16_t)80, us.port);
  CPPUNIT_ASSERT_EQUAL(std::string("aria.rednoah.com"), us.host);
  CPPUNIT_ASSERT_EQUAL(std::string("/aria2/"), us.dir);
  CPPUNIT_ASSERT_EQUAL(std::string("aria3"), us.file);
  CPPUNIT_ASSERT_EQUAL(std::string(""), us.query);
}

void UriTest::testSetUri7()
{
  UriStruct us;
  bool v = parse(us, "http://");

  CPPUNIT_ASSERT(!v);
}

void UriTest::testSetUri8()
{
  UriStruct us;
  bool v = parse(us, "http:/aria.rednoah.com");

  CPPUNIT_ASSERT(!v);
}

void UriTest::testSetUri9()
{
  UriStruct us;
  bool v = parse(us, "h");

  CPPUNIT_ASSERT(!v);
}

void UriTest::testSetUri10()
{
  UriStruct us;
  bool v = parse(us, "");

  CPPUNIT_ASSERT(!v);
}

void UriTest::testSetUri11()
{
  UriStruct us;
  bool v = parse(us, "http://host?query/");

  CPPUNIT_ASSERT(v);
  CPPUNIT_ASSERT_EQUAL(std::string("http"), us.protocol);
  CPPUNIT_ASSERT_EQUAL(std::string("host"), us.host);
  CPPUNIT_ASSERT_EQUAL(std::string("/"), us.dir);
  CPPUNIT_ASSERT_EQUAL(std::string(""), us.file);
  CPPUNIT_ASSERT_EQUAL(std::string("?query/"), us.query);
}

void UriTest::testSetUri12()
{
  UriStruct us;
  bool v = parse(us, "http://host?query");
  
  CPPUNIT_ASSERT(v);
  CPPUNIT_ASSERT_EQUAL(std::string("http"), us.protocol);
  CPPUNIT_ASSERT_EQUAL(std::string("host"), us.host);
  CPPUNIT_ASSERT_EQUAL(std::string("/"), us.dir);
  CPPUNIT_ASSERT_EQUAL(std::string(""), us.file);
  CPPUNIT_ASSERT_EQUAL(std::string("?query"), us.query);
}

void UriTest::testSetUri13()
{
  UriStruct us;
  bool v = parse(us, "http://host/?query");
  
  CPPUNIT_ASSERT(v);
  CPPUNIT_ASSERT_EQUAL(std::string("http"), us.protocol);
  CPPUNIT_ASSERT_EQUAL(std::string("host"), us.host);
  CPPUNIT_ASSERT_EQUAL(std::string("/"), us.dir);
  CPPUNIT_ASSERT_EQUAL(std::string(""), us.file);
  CPPUNIT_ASSERT_EQUAL(std::string("?query"), us.query);
}

void UriTest::testSetUri14()
{
  UriStruct us;
  bool v = parse(us, "http://host:8080/abc?query");
  
  CPPUNIT_ASSERT(v);
  CPPUNIT_ASSERT_EQUAL(std::string("http"), us.protocol);
  CPPUNIT_ASSERT_EQUAL(std::string("host"), us.host);
  CPPUNIT_ASSERT_EQUAL((uint16_t)8080, us.port);
  CPPUNIT_ASSERT_EQUAL(std::string("/"), us.dir);
  CPPUNIT_ASSERT_EQUAL(std::string("abc"), us.file);
  CPPUNIT_ASSERT_EQUAL(std::string("?query"), us.query);
}

void UriTest::testSetUri15()
{
  UriStruct us;
  // 2 slashes after host name and dir
  bool v = parse(us, "http://host//dir1/dir2//file");
  CPPUNIT_ASSERT(v);
  CPPUNIT_ASSERT_EQUAL(std::string("http"), us.protocol);
  CPPUNIT_ASSERT_EQUAL(std::string("host"), us.host);
  CPPUNIT_ASSERT_EQUAL(std::string("//dir1/dir2//"), us.dir);
  CPPUNIT_ASSERT_EQUAL(std::string("file"), us.file);
  CPPUNIT_ASSERT_EQUAL(std::string(""), us.query);
}

void UriTest::testSetUri16()
{
  UriStruct us;
  // 2 slashes before file
  bool v = parse(us, "http://host//file");
  CPPUNIT_ASSERT(v);
  CPPUNIT_ASSERT_EQUAL(std::string("http"), us.protocol);
  CPPUNIT_ASSERT_EQUAL(std::string("host"), us.host);
  CPPUNIT_ASSERT_EQUAL(std::string("//"), us.dir);
  CPPUNIT_ASSERT_EQUAL(std::string("file"), us.file);
  CPPUNIT_ASSERT_EQUAL(std::string(""), us.query);
}

void UriTest::testSetUri18()
{
  UriStruct us;
  bool v = parse(us, "http://1/");

  CPPUNIT_ASSERT(v);
}

void UriTest::testSetUri19()
{
  UriStruct us;
  // No host
  bool v = parse(us, "http://user@");

  CPPUNIT_ASSERT(!v);
}

void UriTest::testSetUri20()
{
  UriStruct us;
  bool v;
  // Invalid port
  v = parse(us, "http://localhost:65536");
  CPPUNIT_ASSERT(!v);
  v = parse(us, "http://localhost:65535");
  CPPUNIT_ASSERT(v);
  v = parse(us, "http://localhost:-80");
  CPPUNIT_ASSERT(!v);
}
  
void UriTest::testSetUri_zeroUsername()
{
  UriStruct us;
  CPPUNIT_ASSERT(parse(us, "ftp://@localhost/download/aria2-1.0.0.tar.bz2"));
  CPPUNIT_ASSERT_EQUAL(std::string("ftp"), us.protocol);
  CPPUNIT_ASSERT_EQUAL((uint16_t)21, us.port);
  CPPUNIT_ASSERT_EQUAL(std::string("localhost"), us.host);
  CPPUNIT_ASSERT_EQUAL(std::string("/download/"), us.dir);
  CPPUNIT_ASSERT_EQUAL(std::string("aria2-1.0.0.tar.bz2"), us.file);
  CPPUNIT_ASSERT_EQUAL(std::string(""), us.username);
  CPPUNIT_ASSERT_EQUAL(std::string(""), us.password);

  CPPUNIT_ASSERT(parse(us, "ftp://:@localhost/download/aria2-1.0.0.tar.bz2"));
  CPPUNIT_ASSERT_EQUAL(std::string("ftp"), us.protocol);
  CPPUNIT_ASSERT_EQUAL((uint16_t)21, us.port);
  CPPUNIT_ASSERT_EQUAL(std::string("localhost"), us.host);
  CPPUNIT_ASSERT_EQUAL(std::string("/download/"), us.dir);
  CPPUNIT_ASSERT_EQUAL(std::string("aria2-1.0.0.tar.bz2"), us.file);
  CPPUNIT_ASSERT_EQUAL(std::string(""), us.username);
  CPPUNIT_ASSERT_EQUAL(std::string(""), us.password);

  CPPUNIT_ASSERT(parse(us,
                       "ftp://:pass@localhost/download/aria2-1.0.0.tar.bz2"));
  CPPUNIT_ASSERT_EQUAL(std::string("ftp"), us.protocol);
  CPPUNIT_ASSERT_EQUAL((uint16_t)21, us.port);
  CPPUNIT_ASSERT_EQUAL(std::string("localhost"), us.host);
  CPPUNIT_ASSERT_EQUAL(std::string("/download/"), us.dir);
  CPPUNIT_ASSERT_EQUAL(std::string("aria2-1.0.0.tar.bz2"), us.file);
  CPPUNIT_ASSERT_EQUAL(std::string(""), us.username);
  CPPUNIT_ASSERT_EQUAL(std::string("pass"), us.password);

}

void UriTest::testSetUri_username()
{
  UriStruct us;
  CPPUNIT_ASSERT
    (parse(us, "ftp://aria2@user@localhost/download/aria2-1.0.0.tar.bz2"));
  CPPUNIT_ASSERT_EQUAL(std::string("ftp"), us.protocol);
  CPPUNIT_ASSERT_EQUAL((uint16_t)21, us.port);
  CPPUNIT_ASSERT_EQUAL(std::string("localhost"), us.host);
  CPPUNIT_ASSERT_EQUAL(std::string("/download/"), us.dir);
  CPPUNIT_ASSERT_EQUAL(std::string("aria2-1.0.0.tar.bz2"), us.file);
  CPPUNIT_ASSERT_EQUAL(std::string("aria2@user"), us.username);
  CPPUNIT_ASSERT_EQUAL(std::string(""), us.password);
}

void UriTest::testSetUri_usernamePassword()
{
  UriStruct us;
  CPPUNIT_ASSERT(parse(us,
                       "ftp://aria2@user%40:aria2@pass%40@localhost/download/"
                       "aria2-1.0.0.tar.bz2"));
  CPPUNIT_ASSERT_EQUAL(std::string("ftp"), us.protocol);
  CPPUNIT_ASSERT_EQUAL((uint16_t)21, us.port);
  CPPUNIT_ASSERT_EQUAL(std::string("localhost"), us.host);
  CPPUNIT_ASSERT_EQUAL(std::string("/download/"), us.dir);
  CPPUNIT_ASSERT_EQUAL(std::string("aria2-1.0.0.tar.bz2"), us.file);
  CPPUNIT_ASSERT_EQUAL(std::string("aria2@user@"), us.username);
  CPPUNIT_ASSERT_EQUAL(std::string("aria2@pass@"), us.password);

  // make sure that after new uri is set, username and password are updated.
  CPPUNIT_ASSERT(parse(us, "ftp://localhost/download/aria2-1.0.0.tar.bz2"));
  CPPUNIT_ASSERT_EQUAL(std::string(""), us.username);
  CPPUNIT_ASSERT_EQUAL(std::string(""), us.password);
}

void UriTest::testSetUri_ipv6()
{
  UriStruct us;
  CPPUNIT_ASSERT(!parse(us, "http://[::1"));
  CPPUNIT_ASSERT(parse(us, "http://[::1]"));
  CPPUNIT_ASSERT_EQUAL(std::string("::1"), us.host);

  CPPUNIT_ASSERT(parse(us, "http://[::1]:8000/dir/file"));
  CPPUNIT_ASSERT_EQUAL(std::string("::1"), us.host);
  CPPUNIT_ASSERT_EQUAL((uint16_t)8000, us.port);
  CPPUNIT_ASSERT_EQUAL(std::string("/dir/"), us.dir);
  CPPUNIT_ASSERT_EQUAL(std::string("file"), us.file);
  CPPUNIT_ASSERT(us.ipv6LiteralAddress);
}

void UriTest::testInnerLink()
{
  UriStruct us;
  bool v = parse(us, "http://aria.rednoah.com/index.html#download");
  CPPUNIT_ASSERT(v);
  CPPUNIT_ASSERT_EQUAL(std::string("index.html"), us.file);
  CPPUNIT_ASSERT_EQUAL(std::string(""), us.query);
}

void UriTest::testConstruct()
{
  {
    UriStruct us;
    CPPUNIT_ASSERT(parse(us, "http://host/dir/file?q=abc#foo"));
    CPPUNIT_ASSERT_EQUAL(std::string("http://host/dir/file?q=abc"),
                         construct(us));
  }
  {
    UriStruct us;
    CPPUNIT_ASSERT(parse(us, "http://host/dir/file"));
    CPPUNIT_ASSERT_EQUAL(std::string("http://host/dir/file"),
                         construct(us));
  }
  {
    UriStruct us;
    CPPUNIT_ASSERT(parse(us, "http://host/dir/"));
    CPPUNIT_ASSERT_EQUAL(std::string("http://host/dir/"),
                         construct(us));
  }
  {
    UriStruct us;
    CPPUNIT_ASSERT(parse(us, "http://host/dir"));
    CPPUNIT_ASSERT_EQUAL(std::string("http://host/dir"),
                         construct(us));
  }
  {
    UriStruct us;
    CPPUNIT_ASSERT(parse(us, "http://host/"));
    CPPUNIT_ASSERT_EQUAL(std::string("http://host/"),
                         construct(us));
  }
  {
    UriStruct us;
    CPPUNIT_ASSERT(parse(us, "http://host"));
    CPPUNIT_ASSERT_EQUAL(std::string("http://host/"),
                         construct(us));
  }
  {
    UriStruct us;
    us.protocol = "http";
    us.host = "host";
    us.file = "foo.xml";
    CPPUNIT_ASSERT_EQUAL(std::string("http://host/foo.xml"),
                         construct(us));
  }
  {
    UriStruct us;
    CPPUNIT_ASSERT(parse(us, "http://host:80"));
    CPPUNIT_ASSERT_EQUAL(std::string("http://host/"),
                         construct(us));
  }
  {
    UriStruct us;
    CPPUNIT_ASSERT(parse(us, "http://host:8080"));
    CPPUNIT_ASSERT_EQUAL(std::string("http://host:8080/"),
                         construct(us));
  }
  {
    UriStruct us;
    CPPUNIT_ASSERT(parse(us, "http://[::1]:8000/dir/file"));
    CPPUNIT_ASSERT_EQUAL(std::string("http://[::1]:8000/dir/file"),
                         construct(us));
  }
  {
    UriStruct us;
    CPPUNIT_ASSERT(parse(us, "ftp://user%40@host/dir/file"));
    CPPUNIT_ASSERT_EQUAL(std::string("ftp://user%40@host/dir/file"),
                         construct(us));
  }
  {
    UriStruct us;
    CPPUNIT_ASSERT(parse(us, "ftp://user:@host/dir/file"));
    CPPUNIT_ASSERT_EQUAL(std::string("ftp://user:@host/dir/file"),
                         construct(us));
  }
  {
    UriStruct us;
    CPPUNIT_ASSERT(parse(us, "ftp://user:passwd%40@host/dir/file"));
    CPPUNIT_ASSERT_EQUAL(std::string("ftp://user:passwd%40@host/dir/file"),
                         construct(us));
  }
}

void UriTest::testSwap()
{
  UriStruct us1;
  CPPUNIT_ASSERT(parse(us1, "http://u1:p1@[::1]/dir1/file1?k1=v1"));
  UriStruct us2;
  CPPUNIT_ASSERT(parse(us2, "ftp://host2/dir2/file2?k2=v2"));
  us1.swap(us2);
  CPPUNIT_ASSERT_EQUAL(std::string("ftp://host2/dir2/file2?k2=v2"),
                       construct(us1));
  CPPUNIT_ASSERT_EQUAL(std::string("http://u1:p1@[::1]/dir1/file1?k1=v1"),
                       construct(us2));
}

void UriTest::testJoinUri()
{
  CPPUNIT_ASSERT_EQUAL(std::string("http://host/dir/file"),
                       joinUri("http://base/d/f",
                               "http://host/dir/file"));

  CPPUNIT_ASSERT_EQUAL(std::string("http://base/dir/file"),
                       joinUri("http://base/d/f",
                               "/dir/file"));

  CPPUNIT_ASSERT_EQUAL(std::string("http://base/d/dir/file"),
                       joinUri("http://base/d/f",
                               "dir/file"));

  CPPUNIT_ASSERT_EQUAL(std::string("http://base/d/"),
                       joinUri("http://base/d/f",
                               ""));

  CPPUNIT_ASSERT_EQUAL(std::string("http://base/d/dir/file?q=k"),
                       joinUri("http://base/d/f",
                               "dir/file?q=k"));

  CPPUNIT_ASSERT_EQUAL(std::string("dir/file"),
                       joinUri("baduri", "dir/file"));

  CPPUNIT_ASSERT_EQUAL(std::string("http://base/a/b/d/file"),
                       joinUri("http://base/a/b/c/x",
                               "../d/file"));

  CPPUNIT_ASSERT_EQUAL(std::string("http://base/a/b/file"),
                       joinUri("http://base/c/x",
                               "../../a/b/file"));

  CPPUNIT_ASSERT_EQUAL(std::string("http://base/"),
                       joinUri("http://base/c/x",
                               "../.."));

  CPPUNIT_ASSERT_EQUAL(std::string("http://base/"),
                       joinUri("http://base/c/x",
                               ".."));

  CPPUNIT_ASSERT_EQUAL(std::string("http://base/a/file"),
                       joinUri("http://base/b/c/x",
                               "/a/x/../file"));

  CPPUNIT_ASSERT_EQUAL(std::string("http://base/file"),
                       joinUri("http://base/f/?q=k",
                               "/file"));

  CPPUNIT_ASSERT_EQUAL(std::string("http://base/file?q=/"),
                       joinUri("http://base/",
                               "/file?q=/"));

  CPPUNIT_ASSERT_EQUAL(std::string("http://base/file?q=v"),
                       joinUri("http://base/",
                               "/file?q=v#a?q=x"));

  CPPUNIT_ASSERT_EQUAL(std::string("http://base/file"),
                       joinUri("http://base/",
                               "/file#a?q=x"));
}

} // namespace uri

} // namespace aria2
