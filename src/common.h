/* <!-- copyright */
/*
 * aria2 - a simple utility for downloading files faster
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
/* copyright --> */
#ifndef _D_COMMON_H_
#define _D_COMMON_H_
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <iostream>
#include <assert.h>
#include <limits.h>
#include <string>
#include <deque>
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

#define BITFIELD_LEN_FROM_PIECES(X) (X/8+(X%8? 1 : 0))

using namespace std;

typedef deque<string> Strings;

#endif // _D_COMMON_H_
