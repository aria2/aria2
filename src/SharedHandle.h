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
#ifndef _D_SHARED_HANDLE_H_
#define _D_SHARED_HANDLE_H_

#include <ostream>

template<class T>
class SharedHandle {

  template<class T1>
  friend std::ostream& operator<<(std::ostream& o, const SharedHandle<T1>& sp);

  template<class T1, class T2>
  friend bool operator==(const SharedHandle<T1>& t1,
			 const SharedHandle<T2>& t2);

  template<class T1, class T2>
  friend bool operator!=(const SharedHandle<T1>& t1,
			 const SharedHandle<T2>& t2);

  template<class T1, class T2>
  friend bool operator<(const SharedHandle<T1>& t1, const SharedHandle<T2>& t2);

private:
  T* obj;
  int* ucount;
public:
  SharedHandle():obj(new T()), ucount(new int(1)) {}
  SharedHandle(T* obj):obj(obj), ucount(new int(1)) {}

  template<class S>
  SharedHandle(const SharedHandle<S>& t):obj(t.get()), ucount(t.getRefCount()) {
    ++*ucount;
  }

  ~SharedHandle() {
    if(--*ucount == 0) {
      delete obj;
      delete ucount;
    }
  }

  template<class S>
  SharedHandle<T>& operator=(const SharedHandle<S>& t) {
    ++*t.getRefCount();
    if(--*ucount == 0) {
      delete obj;
      delete ucount;
    }
    obj = t.get();
    ucount = t.getRefCount();
    return *this;
  }

  T* operator->() { return obj; }

  T* operator->() const { return obj; }

  T* get() const {
    return obj;
  }

  int* getRefCount() const {
    return ucount;
  }
};

template<class T>
std::ostream& operator<<(std::ostream& o, const SharedHandle<T>& sp) {
  o << *sp.obj;
  return o;
}

template<class T1, class T2>
bool operator==(const SharedHandle<T1>& t1, const SharedHandle<T2>& t2) {
  return *t1.obj == *t2.obj;
}

template<class T1, class T2>
bool operator!=(const SharedHandle<T1>& t1, const SharedHandle<T2>& t2) {
  return *t1.obj != *t2.obj;
}

template<class T1, class T2>
bool operator<(const SharedHandle<T1>& t1, const SharedHandle<T2>& t2) {
  return *t1.obj < *t2.obj;
}

#endif // _D_SHARED_HANDLE_H_
