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
#include "Data.h"
#include "MetaEntryVisitor.h"

Data::Data(const char* data, int len, bool number):number(number) {
  if(data == NULL) {
    this->data = NULL;
    this->len = 0;
  } else {
    this->data = new char[len];
    memcpy(this->data, data, len);
    this->len = len;
  }
}

Data::~Data() {
  delete [] data;
}

string Data::toString() const {
  if(len == 0) {
    return "";
  } else {
    char* temp = new char[len+1];
    memcpy(temp, data, len);
    temp[len] = '\0';
    string str(temp);
    delete [] temp;
    return str;
  }
}

const char* Data::getData() const {
  if(this->len == 0) {
    return NULL;
  } else {
    return data;
  }
}

int Data::getLen() const {
  return len;
}

int Data::toInt() const {
  return (int)toLLInt();
}

long long int Data::toLLInt() const {
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
