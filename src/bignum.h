/***
 * This software was written by Nils Maier. No copyright is claimed, and the
 * software is hereby placed in the public domain.
 *
 * In case this attempt to disclaim copyright and place the software in the
 * public domain is deemed null and void in your jurisdiction, then the
 * software is Copyright 2004,2013 Nils Maier and it is hereby released to the
 * general public under the following terms:
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted.
 * There's ABSOLUTELY NO WARRANTY, express or implied.
 */

#ifndef BIGNUM_H
#define BIGNUM_H

#include <cstring>
#include <algorithm>
#include <memory>
#include <stdint.h>

#include "a2functional.h"

namespace bignum {

template <size_t dim> class ulong {

public:
  typedef char char_t;
  typedef std::make_unsigned<char_t>::type uchar_t;

private:
  std::unique_ptr<char_t[]> buf_;

public:
  inline ulong() : buf_(aria2::make_unique<char_t[]>(dim)) {}
  inline ulong(size_t t) : buf_(aria2::make_unique<char_t[]>(dim))
  {
    memcpy(buf_.get(), (char_t*)&t, sizeof(t));
  }
  inline ulong(const ulong<dim>& rhs) : buf_(aria2::make_unique<char_t[]>(dim))
  {
    memcpy(buf_.get(), rhs.buf_.get(), dim);
  }
  explicit inline ulong(const char_t* data, size_t size)
      : buf_(aria2::make_unique<char_t[]>(dim))
  {
    if (size > dim) {
      throw std::bad_alloc();
    }
    memcpy(buf_.get(), data, size);
  }

  virtual ~ulong() = default;

  ulong<dim>& operator=(const ulong<dim>& rhs)
  {
    memcpy(buf_.get(), rhs.buf_.get(), dim);
    return *this;
  }

  bool operator==(const ulong<dim>& rhs) const
  {
    return memcmp(buf_.get(), rhs.buf_.get(), dim) == 0;
  }
  bool operator!=(const ulong<dim>& rhs) const
  {
    return memcmp(buf_.get(), rhs.buf_.get(), dim) != 0;
  }
  bool operator>(const ulong<dim>& rhs) const
  {
    const auto b1 = buf_.get();
    const auto b2 = rhs.buf_.get();
    for (ssize_t i = dim - 1; i >= 0; --i) {
      for (ssize_t j = 1; j >= 0; --j) {
        char_t t = ((uchar_t)(b1[i] << 4 * (1 - j))) >> 4;
        char_t r = ((uchar_t)(b2[i] << 4 * (1 - j))) >> 4;
        if (t != r) {
          return t > r;
        }
      }
    }
    return false;
  }
  bool operator>=(const ulong<dim>& rhs) const
  {
    return *this == rhs || *this > rhs;
  }
  bool operator<(const ulong<dim>& rhs) const { return !(*this >= rhs); }
  bool operator<=(const ulong<dim>& rhs) const
  {
    return *this == rhs || *this < rhs;
  }

  ulong<dim> operator+(const ulong<dim>& rhs) const
  {
    ulong<dim> rv;
    const auto b1 = buf_.get();
    const auto b2 = rhs.buf_.get();
    const auto rb = rv.buf_.get();
    bool base = false;
    for (size_t i = 0; i < dim; ++i) {
      for (ssize_t j = 0; j < 2; ++j) {
        char_t t = ((uchar_t)(b1[i] << 4 * (1 - j))) >> 4;
        char_t r = ((uchar_t)(b2[i] << 4 * (1 - j))) >> 4;
        if (base) {
          t++;
        }
        if (r + t >= 16) {
          rb[i] += (t + r - 16) << j * 4;
          base = true;
        }
        else {
          rb[i] += (t + r) << j * 4;
          base = false;
        }
      }
    }
    return rv;
  }
  ulong<dim>& operator+=(const ulong<dim>& rhs)
  {
    *this = *this + rhs;
    return *this;
  }
  ulong<dim>& operator++()
  {
    *this = *this + 1;
    return *this;
  }
  ulong<dim> operator++(int)
  {
    ulong<dim> tmp = *this;
    *this = *this + 1;
    return tmp;
  }

  ulong<dim> operator-(const ulong<dim>& rhs) const
  {
    ulong<dim> rv;
    const auto b1 = buf_.get();
    const auto b2 = rhs.buf_.get();
    const auto rb = rv.buf_.get();
    bool base = false;
    for (size_t i = 0; i < dim; ++i) {
      for (ssize_t j = 0; j < 2; ++j) {
        char_t t = ((uchar_t)(b1[i] << 4 * (1 - j))) >> 4;
        char_t r = ((uchar_t)(b2[i] << 4 * (1 - j))) >> 4;
        if (base) {
          t--;
        }
        if (t >= r) {
          rb[i] += (t - r) << j * 4;
          base = false;
        }
        else {
          rb[i] += (t + 16 - r) << j * 4;
          base = true;
        }
      }
    }
    return rv;
  }
  ulong<dim>& operator-=(const ulong<dim>& rhs)
  {
    *this = *this - rhs;
    return *this;
  }
  ulong<dim>& operator--()
  {
    *this = *this - 1;
    return *this;
  }
  ulong<dim> operator--(int)
  {
    ulong<dim> tmp = *this;
    *this = *this - 1;
    return tmp;
  }

  ulong<dim> operator*(const ulong<dim>& rhs) const
  {
    ulong<dim> c = rhs, rv;
    const ulong<dim> null;
    size_t cap = c.capacity();
    while (c != null) {
      ulong<dim> tmp = *this;
      tmp.mul(cap - 1);
      rv += tmp;

      ulong<dim> diff(1);
      diff.mul(cap - 1);
      c -= diff;

      cap = c.capacity();
    }
    return rv;
  }
  ulong<dim>& operator*=(const ulong<dim>& rhs)
  {
    *this = *this * rhs;
    return *this;
  }

  ulong<dim> operator/(const ulong<dim>& rhs) const
  {
    ulong<dim> quotient, remainder;
    div(rhs, quotient, remainder);
    return quotient;
  }
  ulong<dim>& operator/=(const ulong<dim>& rhs)
  {
    *this = *this / rhs;
    return *this;
  }

  ulong<dim> operator%(const ulong<dim>& rhs) const
  {
    ulong<dim> quotient, remainder;
    div(rhs, quotient, remainder);
    return remainder;
  }
  ulong<dim>& operator%=(const ulong<dim>& rhs)
  {
    *this = *this % rhs;
    return *this;
  }

  ulong<dim> mul_mod(const ulong<dim>& mul, const ulong<dim>& mod) const
  {
    if (capacity() + mul.capacity() <= dim) {
      return (*this * mul) % mod;
    }
    ulong<dim * 2> et(buf_.get(), dim), emul(mul.buf_.get(), dim),
        emod(mod.buf_.get(), dim), erv = (et * emul) % emod;
    ulong<dim> rv;
    erv.binary(rv.buf_.get(), dim);
    return rv;
  }

  std::unique_ptr<char_t[]> binary() const
  {
    ulong<dim> c = *this;
    std::unique_ptr<char_t[]> rv;
    rv.swap(c.buf_);
    return rv;
  }
  void binary(char_t* buf, size_t len) const
  {
    memcpy(buf, buf_.get(), std::min(dim, len));
  }

  size_t length() const { return dim; }

private:
  size_t capacity() const
  {
    size_t rv = dim * 2;
    const auto b = buf_.get();
    for (ssize_t i = dim - 1; i >= 0; --i) {
      char_t f = b[i] >> 4;
      char_t s = (b[i] << 4) >> 4;
      if (!f && !s) {
        rv -= 2;
        continue;
      }
      if (!f) {
        --rv;
      }
      return rv;
    }
    return rv;
  }

  void mul(size_t digits)
  {
    ulong<dim> tmp = *this;
    auto bt = tmp.buf_.get();
    auto b = buf_.get();
    memset(b, 0, dim);
    const size_t npar = digits % 2;
    const size_t d2 = digits / 2;
    for (size_t i = d2; i < dim; ++i) {
      for (size_t j = 0; j < 2; ++j) {
        char_t c = ((uchar_t)(bt[(dim - 1) - i] << 4 * (1 - j))) >> 4;
        char_t r = c << (npar * (1 - j) * 4 + (1 - npar) * j * 4);
        ssize_t idx = i - d2 - npar * j;
        if (idx >= 0) {
          b[(dim - 1) - idx] += r;
        }
      }
    }
  }

  void div(const ulong<dim>& d, ulong<dim>& q, ulong<dim>& r) const
  {
    ulong<dim> tmp = d;
    r = *this;
    q = 0;
    size_t cr = r.capacity();
    const size_t cd = d.capacity();
    while (cr > cd) {
      tmp = d;
      tmp.mul(cr - cd - 1);
      ulong<dim> qt(1);
      qt.mul(cr - cd - 1);
      ulong<dim> t = tmp;
      t.mul(1);
      if (r >= t) {
        tmp = t;
        qt.mul(1);
      }
      while (r >= tmp) {
        r -= tmp;
        q += qt;
      }
      cr = r.capacity();
    }
    while (r >= d) {
      r -= d;
      ++q;
    }
  }
};

} // namespace bignum

#endif
