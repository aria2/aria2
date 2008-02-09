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
#include "Data.h"
#include "MetaEntryVisitor.h"

Data::Data(const char* data, int32_t len, bool number):number(number) {
  if(data == NULL) {
    this->data = NULL;
    this->len = 0;
  } else {
    this->data = new char[len];
    memcpy(this->data, data, len);
    this->len = len;
  }
}

Data::Data(const string& data, bool number):number(number)
{
  if(data.empty()) {
    this->data = 0;
    this->len = 0;
  } else {
    this->data = new char[data.size()];
    memcpy(this->data, data.c_str(), data.size());
    this->len = data.size();
  }
}

Data::~Data() {
  delete [] data;
}

string Data::toString() const {
  return string(&data[0], &data[len]);
}

const char* Data::getData() const {
  if(this->len == 0) {
    return NULL;
  } else {
    return data;
  }
}

int32_t Data::getLen() const {
  return len;
}

int32_t Data::toInt() const {
  return toLLInt();
}

int64_t Data::toLLInt() const {
  if(len == 0) {
    return 0;
  } else {
    return strtoll(toString().c_str(), NULL, 10);
  }
}

bool Data::isNumber() const {
  return number;
}

void Data::accept(MetaEntryVisitor* v) const {
  v->visit(this);
}
