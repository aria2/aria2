/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2006 Tatsuhiro Tsujikawa
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * In addition, as a special exception, the copyright holders give
 * permission to link the code of portions of this program with the
 * OpenSSL library under certain conditions as described in each
 * individual source file, and distribute linked combinations
 * including the two.
 * You must obey the GNU General Public License in all respects
 * for all of the code used other than OpenSSL.  If you modify
 * file(s) with this exception, you may extend this exception to your
 * version of the file(s), but you are not obligated to do so.  If you
 * do not wish to do so, delete this exception statement from your
 * version.  If you delete this exception statement from all source
 * files in the program, then also delete it here.
 */
/* copyright --> */
#ifndef _D_BASE64_H_
#define _D_BASE64_H_
#include <string>

using namespace std;

class Base64
{
private:
  /**
   * Removes non base64 chars(including '=') from src, and results are
   * stored in nsrc and its length is stored in nlength.
   * Caller must delete nsrc.
   */
  static void removeNonBase64Chars(unsigned char*& nsrc, size_t& nlength,
				   const unsigned char* src, size_t slength);

public:
  /**
   * Encods src whose length is slength into base64 encoded data
   * and stores them to result.
   * result is allocated in this function and the length is stored to rlength.
   * If slength is equal to 0, then return with rlength set to 0 and result
   * is left untouched.
   * A caller must deallocate the memory used by result.
   */
  static void encode(unsigned char*& result, size_t& rlength,
		     const unsigned char* src, size_t slength);

  static void encode(unsigned char*& result, size_t& rlength,
		     const char* src, size_t slength)
  {
    encode(result, rlength, (const unsigned char*)src, slength);
  }

  static string encode(const string& s);

  /**
   * Dencods base64 encoded src whose length is slength and stores them to
   * result.
   * result is allocated in this function and the length is stored to rlength.
   * If slength is equal to 0 or is not multiple of 4, then return with rlength
   * set to 0 and result is left untouched.
   * The function removes non-base64 characters before decoding.
   * A caller must deallocate the memory used by result.
   */
  static void decode(unsigned char*& result, size_t& rlength,
		     const unsigned char* src, size_t slength);

  static void decode(unsigned char*& result, size_t& rlength,
		     const char* src, size_t slength)
  {
    decode(result, rlength, (const unsigned char*)src, slength);
  }

  static string decode(const string& s);
};

#endif // _BASE64_H_
