/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2009 Tatsuhiro Tsujikawa
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
#ifndef D_SEQUENTIAL_PICKER_H
#define D_SEQUENTIAL_PICKER_H

#include "common.h"

#include <deque>
#include <memory>
#include <functional>

namespace aria2 {

template <typename T> class SequentialPicker {
private:
  std::deque<std::unique_ptr<T>> entries_;
  std::unique_ptr<T> pickedEntry_;

public:
  bool isPicked() const { return pickedEntry_.get(); }

  const std::unique_ptr<T>& getPickedEntry() const { return pickedEntry_; }

  void dropPickedEntry() { pickedEntry_.reset(); }

  bool hasNext() const { return !entries_.empty(); }

  T* pickNext()
  {
    if (hasNext()) {
      pickedEntry_ = std::move(entries_.front());
      entries_.pop_front();
      return pickedEntry_.get();
    }
    return nullptr;
  }

  void pushEntry(std::unique_ptr<T> entry)
  {
    entries_.push_back(std::move(entry));
  }

  size_t countEntryInQueue() const { return entries_.size(); }

  bool isPicked(const std::function<bool(const T&)>& pred) const
  {
    return pickedEntry_ && pred(*pickedEntry_);
  }

  bool isQueued(const std::function<bool(const T&)>& pred) const
  {
    for (auto& e : entries_) {
      if (pred(*e)) {
        return true;
      }
    }
    return false;
  }
};

} // namespace aria2

#endif // D_SEQUENTIAL_PICKER_H
