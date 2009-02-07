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

typedef struct StrongRef {} StrongRef;
typedef struct WeakRef {} WeakRef;

class RefCount {
private:
  size_t _strongRefCount;
  size_t _weakRefCount;
public:
  RefCount():_strongRefCount(1), _weakRefCount(1) {}

  RefCount(const WeakRef&):_strongRefCount(0), _weakRefCount(1) {}

  inline void addRefCount() {
    ++_strongRefCount;
    ++_weakRefCount;
  }

  inline void addWeakRefCount() {
    ++_weakRefCount;
  }

  inline void releaseRefCount() {
    --_strongRefCount;
    --_weakRefCount;
  }

  inline void releaseWeakRefCount() {
    --_weakRefCount;
  }

  inline size_t getStrongRefCount() { return _strongRefCount; }

  inline size_t getWeakRefCount() { return _weakRefCount; }
};

class WeakCount;

class SharedCount {
private:
  friend class WeakCount;

  RefCount* _refCount;
public:
  SharedCount():_refCount(new RefCount()) {}

  SharedCount(const SharedCount& s):_refCount(s._refCount)
  {
    _refCount->addRefCount();
  }

  ~SharedCount() {
    _refCount->releaseRefCount();
    if(_refCount->getWeakRefCount() == 0) {
      delete _refCount;
    }
  }

  bool reassign(const SharedCount& s) {
    s._refCount->addRefCount();
    _refCount->releaseRefCount();
    if(_refCount->getWeakRefCount() == 0) {
      delete _refCount;
      _refCount = s._refCount;
      return true;
    }
    size_t thisCount = _refCount->getStrongRefCount();
    _refCount = s._refCount;
    return thisCount == 0;
  }

  inline size_t getRefCount() const { return _refCount->getStrongRefCount(); }
};

class WeakCount {
private:
  RefCount* _refCount;
public:
  WeakCount(const WeakRef& t):_refCount(new RefCount(t)) {}

  WeakCount(const StrongRef& t):_refCount(new RefCount()) {}

  WeakCount(const WeakCount& w):_refCount(w._refCount)
  {
    _refCount->addWeakRefCount();
  }

  WeakCount(const SharedCount& s):_refCount(s._refCount)
  {
    _refCount->addWeakRefCount();
  }

  ~WeakCount()
  {
    _refCount->releaseWeakRefCount();
    if(_refCount->getWeakRefCount() == 0) {
      delete _refCount;
    }
  }

  bool reassign(const SharedCount& s) {
    s._refCount->addWeakRefCount();
    _refCount->releaseWeakRefCount();
    if(_refCount->getWeakRefCount() == 0) {
      delete _refCount;
      _refCount = s._refCount;
      return true;
    }
    _refCount = s._refCount;
    return false;
  }

  bool reassign(const WeakCount& s) {
    s._refCount->addWeakRefCount();
    _refCount->releaseWeakRefCount();
    if(_refCount->getWeakRefCount() == 0) {
      delete _refCount;
      _refCount = s._refCount;
      return true;
    }
    _refCount = s._refCount;
    return false;
  }

  inline size_t getRefCount() const { return _refCount->getStrongRefCount(); }

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

  template<typename S> friend class SharedHandle;

  template<typename S> friend class WeakHandle;

  T* obj;

  SharedCount ucount;

public:
  SharedHandle():obj(0), ucount() {}

  explicit SharedHandle(T* obj):obj(obj), ucount() {}

  SharedHandle(const SharedHandle& t):obj(t.obj), ucount(t.ucount) {}

  template<typename S>
  SharedHandle(const SharedHandle<S>& t):obj(t.obj), ucount(t.ucount) {}

  template<typename S>
  SharedHandle(const SharedHandle<S>& t, T* p):
    obj(p), ucount(t.ucount) {}

  ~SharedHandle() {
    if(ucount.getRefCount() == 1) {
      delete obj;
    }
  }

  SharedHandle& operator=(const SharedHandle& t) {
    if(ucount.reassign(t.ucount)) {
      delete obj;
    }
    obj = t.obj;
    return *this;
  }

  template<typename S>
  SharedHandle& operator=(const SharedHandle<S>& t) {
    if(ucount.reassign(t.ucount)) {
      delete obj;
    }
    obj = t.obj;
    return *this;
  }

  T* operator->() { return obj; }

  T* operator->() const { return obj; }

  T* get() const {
    return obj;
  }

  size_t getRefCount() const {
    return ucount.getRefCount();
  }

  void reset() {
    *this = SharedHandle();
  }

  void reset(T* t) {
    *this = SharedHandle(t);
  }

  bool isNull() const {
    return obj == 0;
  }
};

template<typename T, typename S>
SharedHandle<T>
dynamic_pointer_cast(const SharedHandle<S>& t) {
  if(T* p = dynamic_cast<T*>(t.get())) {
    return SharedHandle<T>(t, p);
  } else {
    return SharedHandle<T>();
  }
}

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

  template<typename S> friend class WeakHandle;

  T* obj;

  WeakCount ucount;

public:
  WeakHandle():obj(0), ucount(WeakRef()) {}

  explicit WeakHandle(T* obj):obj(obj), ucount(StrongRef()) {}

  WeakHandle(const WeakHandle& t):obj(t.obj), ucount(t.ucount) {}

  template<typename S>
  WeakHandle(const SharedHandle<S>& t):obj(t.obj), ucount(t.ucount) {}

  template<typename S>
  WeakHandle(const WeakHandle<S>& t, T* p):
    obj(p), ucount(t.ucount) {}

  ~WeakHandle() {}

  WeakHandle& operator=(const WeakHandle& t) { 
    ucount.reassign(t.ucount);
    obj = t.obj;
    return *this;
  }

  template<typename S>
  WeakHandle& operator=(const SharedHandle<S>& t) {
    ucount.reassign(t.ucount);
    obj = t.obj;
    return *this;
  }

  template<typename S>
  WeakHandle& operator=(const WeakHandle<S>& t) { 
    ucount.reassign(t.ucount);
    obj = t.obj;
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

  size_t getRefCount() const {
    return ucount.getRefCount();
  }

  void reset() {
    *this = WeakHandle();
  }

  bool isNull() const {
    return ucount.getRefCount() == 0 || obj == 0;
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

template<typename T, typename S>
WeakHandle<T>
dynamic_pointer_cast(const WeakHandle<S>& t) {
  if(T* p = dynamic_cast<T*>(t.get())) {
    return WeakHandle<T>(t, p);
  } else {
    return WeakHandle<T>();
  }
}

} // namespace aria2

#endif // _D_SHARED_HANDLE_H_
