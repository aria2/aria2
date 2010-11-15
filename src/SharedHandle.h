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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
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
#ifndef D_SHARED_HANDLE_H
#define D_SHARED_HANDLE_H

#include <cassert>
#include <iosfwd>
#include <algorithm>

// To Use std::tr1::shared_ptr uncomment following few lines and
// comment out SharedHandle stuff.
// 
// #include <tr1/memory>
// #define SharedHandle std::tr1::shared_ptr
// #define WeakHandle std::tr1::weak_ptr
// using std::tr1::static_pointer_cast;
// using std::tr1::dynamic_pointer_cast;

namespace aria2 {

typedef struct StrongRef {} StrongRef;
typedef struct WeakRef {} WeakRef;

class RefCount {
private:
  size_t strongRefCount_;
  size_t weakRefCount_;
public:
  RefCount():strongRefCount_(1), weakRefCount_(1) {}

  RefCount(const WeakRef&):strongRefCount_(0), weakRefCount_(1) {}

  inline void addRefCount() {
    ++strongRefCount_;
    ++weakRefCount_;
  }

  inline void addWeakRefCount() {
    ++weakRefCount_;
  }

  inline void releaseRefCount() {
    --strongRefCount_;
    --weakRefCount_;
  }

  inline void releaseWeakRefCount() {
    --weakRefCount_;
  }

  inline size_t getStrongRefCount() { return strongRefCount_; }

  inline size_t getWeakRefCount() { return weakRefCount_; }
};

class WeakCount;

class SharedCount {
private:
  friend class WeakCount;

  RefCount* refCount_;
public:
  SharedCount():refCount_(new RefCount()) {}

  SharedCount(const SharedCount& s):refCount_(s.refCount_)
  {
    refCount_->addRefCount();
  }

  ~SharedCount() {
    refCount_->releaseRefCount();
    if(refCount_->getWeakRefCount() == 0) {
      delete refCount_;
    }
  }

  bool reassign(const SharedCount& s) {
    s.refCount_->addRefCount();
    refCount_->releaseRefCount();
    if(refCount_->getWeakRefCount() == 0) {
      delete refCount_;
      refCount_ = s.refCount_;
      return true;
    }
    size_t thisCount = refCount_->getStrongRefCount();
    refCount_ = s.refCount_;
    return thisCount == 0;
  }

  inline size_t getRefCount() const { return refCount_->getStrongRefCount(); }

  void swap(SharedCount& r)
  {
    std::swap(refCount_, r.refCount_);
  }
};

class WeakCount {
private:
  RefCount* refCount_;
public:
  WeakCount(const WeakRef& t):refCount_(new RefCount(t)) {}

  WeakCount(const StrongRef& t):refCount_(new RefCount()) {}

  WeakCount(const WeakCount& w):refCount_(w.refCount_)
  {
    refCount_->addWeakRefCount();
  }

  WeakCount(const SharedCount& s):refCount_(s.refCount_)
  {
    refCount_->addWeakRefCount();
  }

  ~WeakCount()
  {
    refCount_->releaseWeakRefCount();
    if(refCount_->getWeakRefCount() == 0) {
      delete refCount_;
    }
  }

  bool reassign(const SharedCount& s) {
    s.refCount_->addWeakRefCount();
    refCount_->releaseWeakRefCount();
    if(refCount_->getWeakRefCount() == 0) {
      delete refCount_;
      refCount_ = s.refCount_;
      return true;
    }
    refCount_ = s.refCount_;
    return false;
  }

  bool reassign(const WeakCount& s) {
    s.refCount_->addWeakRefCount();
    refCount_->releaseWeakRefCount();
    if(refCount_->getWeakRefCount() == 0) {
      delete refCount_;
      refCount_ = s.refCount_;
      return true;
    }
    refCount_ = s.refCount_;
    return false;
  }

  inline size_t getRefCount() const { return refCount_->getStrongRefCount(); }

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

  T* obj_;

  SharedCount ucount_;

public:
  SharedHandle():obj_(0), ucount_() {}

  explicit SharedHandle(T* obj):obj_(obj), ucount_() {}

  SharedHandle(const SharedHandle& t):obj_(t.obj_), ucount_(t.ucount_) {}

  template<typename S>
  SharedHandle(const SharedHandle<S>& t):obj_(t.obj_), ucount_(t.ucount_) {}

  template<typename S>
  SharedHandle(const SharedHandle<S>& t, T* p):
    obj_(p), ucount_(t.ucount_) {}

  ~SharedHandle() {
    if(ucount_.getRefCount() == 1) {
      delete obj_;
    }
  }

  SharedHandle& operator=(const SharedHandle& t) {
    if(ucount_.reassign(t.ucount_)) {
      delete obj_;
    }
    obj_ = t.obj_;
    return *this;
  }

  template<typename S>
  SharedHandle& operator=(const SharedHandle<S>& t) {
    if(ucount_.reassign(t.ucount_)) {
      delete obj_;
    }
    obj_ = t.obj_;
    return *this;
  }

private:
  typedef T* SharedHandle::*unspecified_bool_type;
public:
  operator unspecified_bool_type() const {
    return obj_ == 0 ? 0 : &SharedHandle::obj_;
  }

  T* operator->() const { return obj_; }

  T& operator*() const {
    assert(obj_);
    return *obj_;
  }

  T* get() const {
    return obj_;
  }

  size_t getRefCount() const {
    return ucount_.getRefCount();
  }

  void reset() {
    *this = SharedHandle();
  }

  void reset(T* t) {
    *this = SharedHandle(t);
  }

  void swap(SharedHandle& other)
  {
    std::swap(obj_, other.obj_);
    ucount_.swap(other.ucount_);
  }
};

template<typename T>
void swap(SharedHandle<T>& a, SharedHandle<T>& b)
{
  a.swap(b);
}

template<typename T, typename S>
SharedHandle<T>
dynamic_pointer_cast(const SharedHandle<S>& t) {
  if(T* p = dynamic_cast<T*>(t.get())) {
    return SharedHandle<T>(t, p);
  } else {
    return SharedHandle<T>();
  }
}

template<typename T, typename S>
SharedHandle<T>
static_pointer_cast(const SharedHandle<S>& t) {
  return SharedHandle<T>(t, static_cast<T*>(t.get()));
}

// Intentionally renamed obj_ as obj_x to cause error
template<typename T>
std::ostream& operator<<(std::ostream& o, const SharedHandle<T>& sp) {
  o << *sp.obj_x;
  return o;
}

template<typename T1, typename T2>
bool operator==(const SharedHandle<T1>& t1, const SharedHandle<T2>& t2) {
  return *t1.obj_x == *t2.obj_;
}

template<typename T1, typename T2>
bool operator!=(const SharedHandle<T1>& t1, const SharedHandle<T2>& t2) {
  return *t1.obj_x != *t2.obj_;
}

template<typename T1, typename T2>
bool operator<(const SharedHandle<T1>& t1, const SharedHandle<T2>& t2) {
  return *t1.obj_x < *t2.obj_;
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

  T* obj_;

  WeakCount ucount_;

public:
  WeakHandle():obj_(0), ucount_(WeakRef()) {}

  explicit WeakHandle(T* obj):obj_(obj), ucount_(StrongRef()) {}

  WeakHandle(const WeakHandle& t):obj_(t.obj_), ucount_(t.ucount_) {}

  template<typename S>
  WeakHandle(const SharedHandle<S>& t):obj_(t.obj_), ucount_(t.ucount_) {}

  template<typename S>
  WeakHandle(const WeakHandle<S>& t, T* p):
    obj_(p), ucount_(t.ucount_) {}

  ~WeakHandle() {}

  WeakHandle& operator=(const WeakHandle& t) { 
    ucount_.reassign(t.ucount_);
    obj_ = t.obj_;
    return *this;
  }

  template<typename S>
  WeakHandle& operator=(const SharedHandle<S>& t) {
    ucount_.reassign(t.ucount_);
    obj_ = t.obj_;
    return *this;
  }

  template<typename S>
  WeakHandle& operator=(const WeakHandle<S>& t) { 
    ucount_.reassign(t.ucount_);
    obj_ = t.obj_;
    return *this;
  }

  T* operator->() { return obj_; }

  T* operator->() const { return obj_; }

  T& operator*() const {
    assert(obj_);
    return *obj_;
  }

  T* get() const {
    if(ucount_.getRefCount() == 0 || obj_ == 0) {
      return 0;
    } else {
      return obj_;
    }
  }

  size_t getRefCount() const {
    return ucount_.getRefCount();
  }

  void reset() {
    *this = WeakHandle();
  }
};

// Intentionally renamed obj_ as obj_x to cause error
template<typename T>
std::ostream& operator<<(std::ostream& o, const WeakHandle<T>& sp) {
  o << *sp.obj_x;
  return o;
}

template<typename T1, typename T2>
bool operator==(const WeakHandle<T1>& t1, const WeakHandle<T2>& t2) {
  return *t1.obj_x == *t2.obj_;
}

template<typename T1, typename T2>
bool operator!=(const WeakHandle<T1>& t1, const WeakHandle<T2>& t2) {
  return *t1.obj_x != *t2.obj_;
}

template<typename T1, typename T2>
bool operator<(const WeakHandle<T1>& t1, const WeakHandle<T2>& t2) {
  return *t1.obj_x < *t2.obj_;
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

#endif // D_SHARED_HANDLE_H
