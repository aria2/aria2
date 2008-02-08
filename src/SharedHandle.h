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

#include <iosfwd>

namespace aria2 {

class RefCount {
public:
  RefCount():totalRefCount(0), strongRefCount(0) {}

  RefCount(int32_t totalRefCount, int32_t strongRefCount)
    :totalRefCount(totalRefCount), strongRefCount(strongRefCount) {}

  int32_t totalRefCount;
  int32_t strongRefCount;
};

template<typename T>
class SharedHandle;

template<typename T>
class WeakHandle;

template<typename T>
class SharedHandle {
private:
  template<typename T1>
  friend std::ostream& operator<<(std::ostream& o, const SharedHandle<T1>& sp);

  template<typename T1, typename T2>
  friend bool operator==(const SharedHandle<T1>& t1,
			 const SharedHandle<T2>& t2);

  template<typename T1, typename T2>
  friend bool operator!=(const SharedHandle<T1>& t1,
			 const SharedHandle<T2>& t2);

  template<typename T1, typename T2>
  friend bool operator<(const SharedHandle<T1>& t1, const SharedHandle<T2>& t2);

  T* obj;

  RefCount* ucount;

  void releaseReference() {
    if(--ucount->strongRefCount == 0) {
      delete obj;
      obj = 0;
    }
    if(--ucount->totalRefCount == 0) {
      delete ucount;
      ucount = 0;
    }
  }

public:
  SharedHandle():obj(new T()), ucount(new RefCount(1, 1)) {}

  SharedHandle(T* obj):obj(obj), ucount(new RefCount(1, 1)) {}

  SharedHandle(const SharedHandle& t):obj(t.get()), ucount(t.getRefCount()) {
    ++ucount->totalRefCount;
    ++ucount->strongRefCount;
  }

  template<typename S>
  SharedHandle(const SharedHandle<S>& t) {
    obj = dynamic_cast<T*>(t.get());
    if(obj) {
      ucount = t.getRefCount();
      ++ucount->totalRefCount;
      ++ucount->strongRefCount;
    } else {
      ucount = new RefCount(1, 1);
    }
  }

  ~SharedHandle() {
    releaseReference();
  }

  SharedHandle& operator=(const SharedHandle& t) { 
    ++t.getRefCount()->totalRefCount;
    ++t.getRefCount()->strongRefCount;
    releaseReference();
    obj = t.get();
    ucount = t.getRefCount();
    return *this;
  }

  template<typename S>
  SharedHandle& operator=(const SharedHandle<S>& t) {
    T* to = dynamic_cast<T*>(t.get());
    if(to) {
      ++t.getRefCount()->totalRefCount;
      ++t.getRefCount()->strongRefCount;
      releaseReference();
      obj = to;
      ucount = t.getRefCount();
    } else {
      releaseReference();
      obj = 0;
      ucount = new RefCount(1, 1);
    }
    return *this;
  }

  T* operator->() { return obj; }

  T* operator->() const { return obj; }

  T* get() const {
    return obj;
  }

  RefCount* getRefCount() const {
    return ucount;
  }

  bool isNull() const {
    return obj == 0;
  }
};

template<typename T>
std::ostream& operator<<(std::ostream& o, const SharedHandle<T>& sp) {
  o << *sp.obj;
  return o;
}

template<typename T1, typename T2>
bool operator==(const SharedHandle<T1>& t1, const SharedHandle<T2>& t2) {
  return *t1.obj == *t2.obj;
}

template<typename T1, typename T2>
bool operator!=(const SharedHandle<T1>& t1, const SharedHandle<T2>& t2) {
  return *t1.obj != *t2.obj;
}

template<typename T1, typename T2>
bool operator<(const SharedHandle<T1>& t1, const SharedHandle<T2>& t2) {
  return *t1.obj < *t2.obj;
}

template<typename T>
class WeakHandle {
private:
  template<typename T1>
  friend std::ostream& operator<<(std::ostream& o, const WeakHandle<T1>& sp);

  template<typename T1, typename T2>
  friend bool operator==(const WeakHandle<T1>& t1,
			 const WeakHandle<T2>& t2);

  template<typename T1, typename T2>
  friend bool operator!=(const WeakHandle<T1>& t1,
			 const WeakHandle<T2>& t2);

  template<typename T1, typename T2>
  friend bool operator<(const WeakHandle<T1>& t1, const WeakHandle<T2>& t2);

  T* obj;

  RefCount* ucount;

  void releaseReference() {
    if(--ucount->totalRefCount == 0) {
      delete ucount;
      ucount = 0;
    }
  }
public:
  WeakHandle():obj(0), ucount(new RefCount(1, 0)) {}

  WeakHandle(T* obj):obj(obj), ucount(new RefCount(1, 1)) {}

  WeakHandle(const WeakHandle& t):obj(t.get()), ucount(t.getRefCount()) {
    ++ucount->totalRefCount;
  }

  template<typename S>
  WeakHandle(const SharedHandle<S>& t):obj(t.get()), ucount(t.getRefCount()) {
    obj = dynamic_cast<T*>(t.get());
    if(obj) {
      ucount = t.getRefCount();
      ++ucount->totalRefCount;
    } else {
      ucount = new RefCount(1, 0);
    }
  }

  template<typename S>
  WeakHandle(const WeakHandle<S>& t) {
    obj = dynamic_cast<T*>(t.get());
    if(obj) {
      ucount = t.getRefCount();
      ++ucount->totalRefCount;
    } else {
      ucount = new RefCount(1, 0);
    }
  }

  ~WeakHandle() {
    releaseReference();
  }

  WeakHandle& operator=(const WeakHandle& t) { 
    ++t.getRefCount()->totalRefCount;
    releaseReference();
    obj = t.get();
    ucount = t.getRefCount();
    return *this;
  }

  template<typename S>
  WeakHandle& operator=(const SharedHandle<S>& t) {
    T* to = dynamic_cast<T*>(t.get());
    if(to) {
      ++t.getRefCount()->totalRefCount;
      releaseReference();
      obj = to;
      ucount = t.getRefCount();
    } else {
      releaseReference();
      obj = 0;
      ucount = new RefCount(1, 0);
    }
    return *this;
  }

  template<typename S>
  WeakHandle& operator=(const WeakHandle<S>& t) { 
    T* to = dynamic_cast<T*>(t.get());
    if(to) {
      ++t.getRefCount()->totalRefCount;
      releaseReference();
      obj = to;
      ucount = t.getRefCount();
    } else {
      releaseReference();
      obj = 0;
      ucount = new RefCount(1, 0);
    }
    return *this;
  }

  T* operator->() { return obj; }

  T* operator->() const { return obj; }

  T* get() const {
    if(isNull()) {
      return 0;
    } else {
      return obj;
    }
  }

  RefCount* getRefCount() const {
    return ucount;
  }

  bool isNull() const {
    return ucount->strongRefCount == 0 || obj == 0;
  }
};

template<typename T>
std::ostream& operator<<(std::ostream& o, const WeakHandle<T>& sp) {
  o << *sp.obj;
  return o;
}

template<typename T1, typename T2>
bool operator==(const WeakHandle<T1>& t1, const WeakHandle<T2>& t2) {
  return *t1.obj == *t2.obj;
}

template<typename T1, typename T2>
bool operator!=(const WeakHandle<T1>& t1, const WeakHandle<T2>& t2) {
  return *t1.obj != *t2.obj;
}

template<typename T1, typename T2>
bool operator<(const WeakHandle<T1>& t1, const WeakHandle<T2>& t2) {
  return *t1.obj < *t2.obj;
}

} // namespace aria2

#endif // _D_SHARED_HANDLE_H_
