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
  SharedHandle(const SharedHandle& t):obj(t.get()), ucount(t.getRefCount()) {
    ++*ucount;
  }
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

  SharedHandle& operator=(const SharedHandle& t) { 
    ++*t.getRefCount();
    if(--*ucount == 0) {
      delete obj;
      delete ucount;
    }
    obj = t.get();
    ucount = t.getRefCount();
    return *this;
  }
  template<class S>
  SharedHandle& operator=(const SharedHandle<S>& t) { 
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
