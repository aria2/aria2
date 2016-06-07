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
#ifndef D_OPTION_H
#define D_OPTION_H

#include "common.h"

#include <string>
#include <vector>
#include <memory>

#include "prefs.h"

namespace aria2 {

class Option {
private:
  std::vector<std::string> table_;
  std::vector<unsigned char> use_;
  std::shared_ptr<Option> parent_;

public:
  Option();
  ~Option();
  Option(const Option& option);
  Option& operator=(const Option& option);

  void put(PrefPtr pref, const std::string& value);
  // Returns true if name is defined. Otherwise returns false.  Note
  // that even if the value is a empty string, this method returns
  // true.  If option is not defined in this object and parent_ is not
  // NULL, lookup parent_ to check |pref| is defined.
  bool defined(PrefPtr pref) const;
  // Just like defined(), but this function does not lookup parent_.
  bool definedLocal(PrefPtr pref) const;
  // Returns true if name is not defined or the value is a empty string.
  // Otherwise returns false.
  bool blank(PrefPtr pref) const;
  // Returns option value for |pref|. If the |pref| is not defined in
  // this object, parent_ is looked up.
  const std::string& get(PrefPtr pref) const;
  int32_t getAsInt(PrefPtr pref) const;
  int64_t getAsLLInt(PrefPtr pref) const;
  bool getAsBool(PrefPtr pref) const;
  double getAsDouble(PrefPtr pref) const;
  // Removes |pref| from this object. This function does not modify
  // parent_.
  void removeLocal(PrefPtr pref);
  // Removes |pref| from this object from all option hierarchy.
  void remove(PrefPtr pref);
  // Removes all option values from this object. This function does
  // not modify parent_.
  void clear();
  // Returns the option value table of this object. It does not
  // contain option values in parent_ and so forth.
  const std::vector<std::string>& getTable() const { return table_; }
  // Copy option values defined in option to this option. parent_ is
  // left unmodified for this object.
  void merge(const Option& option);
  // Sets parent Option object for this object.
  void setParent(const std::shared_ptr<Option>& parent);
  const std::shared_ptr<Option>& getParent() const;
  // Returns true if there is no option stored.
  bool emptyLocal() const;
};

} // namespace aria2

#endif // D_OPTION_H
