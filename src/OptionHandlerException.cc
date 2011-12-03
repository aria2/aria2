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
#include "OptionHandlerException.h"
#include "fmt.h"
#include "prefs.h"

namespace aria2 {

namespace {
const char* MESSAGE =
  _("We encountered a problem while processing the option '--%s'.");
} // namespace

OptionHandlerException::OptionHandlerException
(const char* file, int line,
 const Pref* pref)
  : RecoverableException
    (file, line, fmt(MESSAGE, pref->k), error_code::OPTION_ERROR),
    pref_(pref)
{}

OptionHandlerException::OptionHandlerException
(const char* file, int line,
 const Pref* pref,
 const Exception& cause)
  : RecoverableException
    (file, line, fmt(MESSAGE, pref->k), error_code::OPTION_ERROR,
     cause),
    pref_(pref)
{}

OptionHandlerException::~OptionHandlerException() throw() {}

SharedHandle<Exception> OptionHandlerException::copy() const
{
  SharedHandle<Exception> e(new OptionHandlerException(*this));
  return e;
}

} // namespace aria2
