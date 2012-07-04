#include "base64.h"
#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class Base64Test:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(Base64Test);
  CPPUNIT_TEST(testEncode);
  CPPUNIT_TEST(testDecode);
  CPPUNIT_TEST(testLongString);
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp() {
  }

  void testEncode();
  void testDecode();
  void testLongString();
};


CPPUNIT_TEST_SUITE_REGISTRATION( Base64Test );

void Base64Test::testEncode() {
  std::string s = "Hello World!";
  CPPUNIT_ASSERT_EQUAL(std::string("SGVsbG8gV29ybGQh"),
                       base64::encode(s.begin(), s.end()));

  s = "Hello World";
  CPPUNIT_ASSERT_EQUAL(std::string("SGVsbG8gV29ybGQ="),
                       base64::encode(s.begin(), s.end()));

  s = "Hello Worl";
  CPPUNIT_ASSERT_EQUAL(std::string("SGVsbG8gV29ybA=="),
                       base64::encode(s.begin(), s.end()));

  s = "Man";
  CPPUNIT_ASSERT_EQUAL(std::string("TWFu"), base64::encode(s.begin(), s.end()));

  s = "M";
  CPPUNIT_ASSERT_EQUAL(std::string("TQ=="), base64::encode(s.begin(), s.end()));

  s = "";
  CPPUNIT_ASSERT_EQUAL(std::string(), base64::encode(s.begin(), s.end()));

  s.assign(1, (char)-1);
  base64::encode(s.begin(), s.end());
  CPPUNIT_ASSERT_EQUAL(std::string("/w=="), base64::encode(s.begin(), s.end()));

  s.assign(2, (char)-1);
  base64::encode(s.begin(), s.end());
  CPPUNIT_ASSERT_EQUAL(std::string("//8="), base64::encode(s.begin(), s.end()));

  s.assign(3, (char)-1);
  base64::encode(s.begin(), s.end());
  CPPUNIT_ASSERT_EQUAL(std::string("////"), base64::encode(s.begin(), s.end()));
}

void Base64Test::testDecode()
{
  std::string s = "SGVsbG8gV29ybGQh";
  CPPUNIT_ASSERT_EQUAL(std::string("Hello World!"),
                       base64::decode(s.begin(), s.end()));

  s = "SGVsbG8gV29ybGQ=";
  CPPUNIT_ASSERT_EQUAL(std::string("Hello World"),
                       base64::decode(s.begin(), s.end()));

  s = "SGVsbG8gV29ybA==";
  CPPUNIT_ASSERT_EQUAL(std::string("Hello Worl"),
                       base64::decode(s.begin(), s.end()));

  s = "TWFu";
  CPPUNIT_ASSERT_EQUAL(std::string("Man"), base64::decode(s.begin(), s.end()));

  s = "TWFu\n";
  CPPUNIT_ASSERT_EQUAL(std::string("Man"), base64::decode(s.begin(), s.end()));

  s = "TQ==";
  CPPUNIT_ASSERT_EQUAL(std::string("M"), base64::decode(s.begin(), s.end()));

  s = "";
  CPPUNIT_ASSERT_EQUAL(std::string(""), base64::decode(s.begin(), s.end()));

  s = "SGVsbG8\ngV2*9ybGQ=";
  CPPUNIT_ASSERT_EQUAL(std::string("Hello World"),
                       base64::decode(s.begin(), s.end()));

  s = "SGVsbG8\ngV2*9ybGQ";
  CPPUNIT_ASSERT_EQUAL(std::string(""), base64::decode(s.begin(), s.end()));

  s = "/w==";
  CPPUNIT_ASSERT_EQUAL(std::string(1, -1), base64::decode(s.begin(), s.end()));
}

void Base64Test::testLongString()
{
  std::string s =
    "LyogPCEtLSBjb3B5cmlnaHQgKi8KLyoKICogYXJpYTIgLSBUaGUgaGlnaCBzcGVlZCBkb3dubG9h"
    "ZCB1dGlsaXR5CiAqCiAqIENvcHlyaWdodCAoQykgMjAwNiBUYXRzdWhpcm8gVHN1amlrYXdhCiAq"
    "CiAqIFRoaXMgcHJvZ3JhbSBpcyBmcmVlIHNvZnR3YXJlOyB5b3UgY2FuIHJlZGlzdHJpYnV0ZSBp"
    "dCBhbmQvb3IgbW9kaWZ5CiAqIGl0IHVuZGVyIHRoZSB0ZXJtcyBvZiB0aGUgR05VIEdlbmVyYWwg"
    "UHVibGljIExpY2Vuc2UgYXMgcHVibGlzaGVkIGJ5CiAqIHRoZSBGcmVlIFNvZnR3YXJlIEZvdW5k"
    "YXRpb247IGVpdGhlciB2ZXJzaW9uIDIgb2YgdGhlIExpY2Vuc2UsIG9yCiAqIChhdCB5b3VyIG9w"
    "dGlvbikgYW55IGxhdGVyIHZlcnNpb24uCiAqCiAqIFRoaXMgcHJvZ3JhbSBpcyBkaXN0cmlidXRl"
    "ZCBpbiB0aGUgaG9wZSB0aGF0IGl0IHdpbGwgYmUgdXNlZnVsLAogKiBidXQgV0lUSE9VVCBBTlkg"
    "V0FSUkFOVFk7IHdpdGhvdXQgZXZlbiB0aGUgaW1wbGllZCB3YXJyYW50eSBvZgogKiBNRVJDSEFO"
    "VEFCSUxJVFkgb3IgRklUTkVTUyBGT1IgQSBQQVJUSUNVTEFSIFBVUlBPU0UuICBTZWUgdGhlCiAq"
    "IEdOVSBHZW5lcmFsIFB1YmxpYyBMaWNlbnNlIGZvciBtb3JlIGRldGFpbHMuCiAqCiAqIFlvdSBz"
    "aG91bGQgaGF2ZSByZWNlaXZlZCBhIGNvcHkgb2YgdGhlIEdOVSBHZW5lcmFsIFB1YmxpYyBMaWNl"
    "bnNlCiAqIGFsb25nIHdpdGggdGhpcyBwcm9ncmFtOyBpZiBub3QsIHdyaXRlIHRvIHRoZSBGcmVl"
    "IFNvZnR3YXJlCiAqIEZvdW5kYXRpb24sIEluYy4sIDUxIEZyYW5rbGluIFN0cmVldCwgRmlmdGgg"
    "Rmxvb3IsIEJvc3RvbiwgTUEgIDAyMTEwLTEzMDEgIFVTQQogKgogKiBJbiBhZGRpdGlvbiwgYXMg"
    "YSBzcGVjaWFsIGV4Y2VwdGlvbiwgdGhlIGNvcHlyaWdodCBob2xkZXJzIGdpdmUKICogcGVybWlz"
    "c2lvbiB0byBsaW5rIHRoZSBjb2RlIG9mIHBvcnRpb25zIG9mIHRoaXMgcHJvZ3JhbSB3aXRoIHRo"
    "ZQogKiBPcGVuU1NMIGxpYnJhcnkgdW5kZXIgY2VydGFpbiBjb25kaXRpb25zIGFzIGRlc2NyaWJl"
    "ZCBpbiBlYWNoCiAqIGluZGl2aWR1YWwgc291cmNlIGZpbGUsIGFuZCBkaXN0cmlidXRlIGxpbmtl"
    "ZCBjb21iaW5hdGlvbnMKICogaW5jbHVkaW5nIHRoZSB0d28uCiAqIFlvdSBtdXN0IG9iZXkgdGhl"
    "IEdOVSBHZW5lcmFsIFB1YmxpYyBMaWNlbnNlIGluIGFsbCByZXNwZWN0cwogKiBmb3IgYWxsIG9m"
    "IHRoZSBjb2RlIHVzZWQgb3RoZXIgdGhhbiBPcGVuU1NMLiAgSWYgeW91IG1vZGlmeQogKiBmaWxl"
    "KHMpIHdpdGggdGhpcyBleGNlcHRpb24sIHlvdSBtYXkgZXh0ZW5kIHRoaXMgZXhjZXB0aW9uIHRv"
    "IHlvdXIKICogdmVyc2lvbiBvZiB0aGUgZmlsZShzKSwgYnV0IHlvdSBhcmUgbm90IG9ibGlnYXRl"
    "ZCB0byBkbyBzby4gIElmIHlvdQogKiBkbyBub3Qgd2lzaCB0byBkbyBzbywgZGVsZXRlIHRoaXMg"
    "ZXhjZXB0aW9uIHN0YXRlbWVudCBmcm9tIHlvdXIKICogdmVyc2lvbi4gIElmIHlvdSBkZWxldGUg"
    "dGhpcyBleGNlcHRpb24gc3RhdGVtZW50IGZyb20gYWxsIHNvdXJjZQogKiBmaWxlcyBpbiB0aGUg"
    "cHJvZ3JhbSwgdGhlbiBhbHNvIGRlbGV0ZSBpdCBoZXJlLgogKi8KLyogY29weXJpZ2h0IC0tPiAq"
    "Lwo=";
  std::string d =
    "/* <!-- copyright */\n"
    "/*\n"
    " * aria2 - The high speed download utility\n"
    " *\n"
    " * Copyright (C) 2006 Tatsuhiro Tsujikawa\n"
    " *\n"
    " * This program is free software; you can redistribute it and/or modify\n"
    " * it under the terms of the GNU General Public License as published by\n"
    " * the Free Software Foundation; either version 2 of the License, or\n"
    " * (at your option) any later version.\n"
    " *\n"
    " * This program is distributed in the hope that it will be useful,\n"
    " * but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
    " * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
    " * GNU General Public License for more details.\n"
    " *\n"
    " * You should have received a copy of the GNU General Public License\n"
    " * along with this program; if not, write to the Free Software\n"
    " * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA\n"
    " *\n"
    " * In addition, as a special exception, the copyright holders give\n"
    " * permission to link the code of portions of this program with the\n"
    " * OpenSSL library under certain conditions as described in each\n"
    " * individual source file, and distribute linked combinations\n"
    " * including the two.\n"
    " * You must obey the GNU General Public License in all respects\n"
    " * for all of the code used other than OpenSSL.  If you modify\n"
    " * file(s) with this exception, you may extend this exception to your\n"
    " * version of the file(s), but you are not obligated to do so.  If you\n"
    " * do not wish to do so, delete this exception statement from your\n"
    " * version.  If you delete this exception statement from all source\n"
    " * files in the program, then also delete it here.\n"
    " */\n"
    "/* copyright --> */\n";
  CPPUNIT_ASSERT_EQUAL(d, base64::decode(s.begin(), s.end()));
  CPPUNIT_ASSERT_EQUAL(s, base64::encode(d.begin(), d.end()));
}

} // namespace aria2
