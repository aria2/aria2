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
#include <iostream>
#include <assert.h>
#include <limits.h>
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifndef LONG_LONG_MAX
# define LONG_LONG_MAX      9223372036854775807LL
# define LONG_LONG_MIN      (-LONG_LONG_MAX - 1LL)
#endif // LONG_LONG_MAX

#define USER_AGENT "aria2"

using namespace std;
