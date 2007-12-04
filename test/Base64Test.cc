#include "Base64.h"
#include <cppunit/extensions/HelperMacros.h>

class Base64Test:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(Base64Test);
  CPPUNIT_TEST(testEncode);
  CPPUNIT_TEST(testDecode);
  CPPUNIT_TEST(testEncode_string);
  CPPUNIT_TEST(testDecode_string);
  CPPUNIT_TEST(testLongString);
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp() {
  }

  void testEncode();
  void testDecode();
  void testEncode_string();
  void testDecode_string();
  void testLongString();
};


CPPUNIT_TEST_SUITE_REGISTRATION( Base64Test );

void Base64Test::testEncode() {
  unsigned char* buf = 0;
  size_t len;
  string s1 = "Hello World!";
  Base64::encode(buf, len, s1.c_str(), s1.size());
  CPPUNIT_ASSERT_EQUAL(string("SGVsbG8gV29ybGQh"), string(&buf[0], &buf[len]));
  delete [] buf;

  string s2 = "Hello World";
  Base64::encode(buf, len, s2.c_str(), s2.size());
  CPPUNIT_ASSERT_EQUAL(string("SGVsbG8gV29ybGQ="), string(&buf[0], &buf[len]));
  delete [] buf;

  string s3 = "Hello Worl";
  Base64::encode(buf, len, s3.c_str(), s3.size());
  CPPUNIT_ASSERT_EQUAL(string("SGVsbG8gV29ybA=="), string(&buf[0], &buf[len]));
  delete [] buf;

  string s4 = "Man";
  Base64::encode(buf, len, s4.c_str(), s4.size());
  CPPUNIT_ASSERT_EQUAL(string("TWFu"), string(&buf[0], &buf[len]));
  delete [] buf;

  string s5 = "M";
  Base64::encode(buf, len, s5.c_str(), s5.size());
  CPPUNIT_ASSERT_EQUAL(string("TQ=="), string(&buf[0], &buf[len]));
  delete [] buf;

  buf = 0;
  string s6 = "";
  Base64::encode(buf, len, s6.c_str(), s6.size());
  CPPUNIT_ASSERT_EQUAL(string(""), string(&buf[0], &buf[len]));
  CPPUNIT_ASSERT_EQUAL((size_t)0, len);
  CPPUNIT_ASSERT(0 == buf);

  {
    const char temp[] = { -1 };
    Base64::encode(buf, len, temp, 1);
    CPPUNIT_ASSERT_EQUAL(string("/w=="), string(&buf[0], &buf[len]));
    delete [] buf;
  }
}

void Base64Test::testEncode_string()
{
  string s1 = "Hello World!";
  CPPUNIT_ASSERT_EQUAL(string("SGVsbG8gV29ybGQh"), Base64::encode(s1));

  string s2 = "";
  CPPUNIT_ASSERT_EQUAL(string(""), Base64::encode(s2));


  
}

void Base64Test::testDecode()
{
  unsigned char* buf;
  size_t len;

  string s1 = "SGVsbG8gV29ybGQh";
  Base64::decode(buf, len, s1.c_str(), s1.size());
  CPPUNIT_ASSERT_EQUAL(string("Hello World!"), string(&buf[0], &buf[len]));
  delete [] buf;

  string s2 = "SGVsbG8gV29ybGQ=";
  Base64::decode(buf, len, s2.c_str(), s2.size());
  CPPUNIT_ASSERT_EQUAL(string("Hello World"), string(&buf[0], &buf[len]));
  delete [] buf;

  string s3 = "SGVsbG8gV29ybA==";
  Base64::decode(buf, len, s3.c_str(), s3.size());
  CPPUNIT_ASSERT_EQUAL(string("Hello Worl"), string(&buf[0], &buf[len]));
  delete [] buf;

  string s4 = "TWFu";
  Base64::decode(buf, len, s4.c_str(), s4.size());
  CPPUNIT_ASSERT_EQUAL(string("Man"), string(&buf[0], &buf[len]));
  delete [] buf;

  string s5 = "TQ==";
  Base64::decode(buf, len, s5.c_str(), s5.size());
  CPPUNIT_ASSERT_EQUAL(string("M"), string(&buf[0], &buf[len]));
  delete [] buf;

  buf = 0;
  string s6 = "";
  Base64::decode(buf, len, s6.c_str(), s6.size());
  CPPUNIT_ASSERT_EQUAL(string(""), string(&buf[0], &buf[len]));
  CPPUNIT_ASSERT_EQUAL((size_t)0, len);
  CPPUNIT_ASSERT(!buf);

  string s7 = "SGVsbG8\ngV2*9ybGQ=";
  Base64::decode(buf, len, s7.c_str(), s7.size());
  CPPUNIT_ASSERT_EQUAL(string("Hello World"), string(&buf[0], &buf[len]));
  delete [] buf;

  buf = 0;
  string s8 = "SGVsbG8\ngV2*9ybGQ";
  Base64::decode(buf, len, s8.c_str(), s8.size());
  CPPUNIT_ASSERT_EQUAL(string(""), string(&buf[0], &buf[len]));
  CPPUNIT_ASSERT_EQUAL((size_t)0, len);
  CPPUNIT_ASSERT(!buf);

  {
    string s = "/w==";
    Base64::decode(buf, len, s.c_str(), s.size());
    CPPUNIT_ASSERT_EQUAL((unsigned char)-1, buf[0]);
    delete [] buf;
  }

}

void Base64Test::testDecode_string()
{
  string s1 = "SGVsbG8gV29ybGQh";
  CPPUNIT_ASSERT_EQUAL(string("Hello World!"), Base64::decode(s1));

  string s2 = "";
  CPPUNIT_ASSERT_EQUAL(string(""), Base64::decode(s2));
}

void Base64Test::testLongString()
{
  string s =
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
  string d =
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
  CPPUNIT_ASSERT_EQUAL(d,
		       Base64::decode(s));  
  CPPUNIT_ASSERT_EQUAL(s,
		       Base64::encode(d));  
}
