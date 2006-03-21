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
#ifndef _D_DATA_H_
#define _D_DATA_H_

#include "MetaEntry.h"
#include <string>

using namespace std;

class Data : public MetaEntry {
private:
  int len;
  char* data;
  bool number;
public:
  /**
   * This class stores the copy of data. So caller must take care of freeing
   * memory of data.
   */
  Data(const char* data, int len, bool number = false);
  ~Data();

  string toString() const;
  int toInt() const;
  long long int toLLInt() const;
  
  const char* getData() const;
  int getLen() const;
  bool isNumber() const;

  void accept(MetaEntryVisitor* v) const;
};

#endif // _D_DATA_H_
