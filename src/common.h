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
#ifndef _D_COMMON_H_
#define _D_COMMON_H_
// use C99 limit macros
#define __STDC_LIMIT_MACROS
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <iostream>
#include <assert.h>
#include <limits.h>
#include <string>
#include <deque>
#include <algorithm>
#if ENABLE_NLS
#  include <gettext.h>
#  define _(String) gettext (String)
#else
#  define _(String) (String)
#endif

#ifndef LONG_LONG_MAX
# define LONG_LONG_MAX      9223372036854775807LL
# define LONG_LONG_MIN      (-LONG_LONG_MAX - 1LL)
#endif // LONG_LONG_MAX

#define USER_AGENT "aria2"

#define BITFIELD_LEN_FROM_PIECES(X) ((X)/8+((X)%8? 1 : 0))

#define DIV_FLOOR(X,Y) ((X)/(Y)+((X)%(Y)? 1:0))

using namespace std;
//#include "debug_new.h"

class Deleter {
public:
  template<class T>
  void operator()(T* ptr) {
    delete ptr;
  }
};

#include "SharedHandle.h"

typedef deque<string> Strings;
typedef deque<int32_t> Integers;

#endif // _D_COMMON_H_
