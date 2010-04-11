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
#ifndef _D_CONSOLE_STAT_CALC_H_
#define _D_CONSOLE_STAT_CALC_H_

#include "StatCalc.h"
#include "TimerA2.h"
#include "util.h"

namespace aria2 {

class SizeFormatter:public std::unary_function<int64_t, std::string> {
protected:
  virtual std::string format(int64_t size) const = 0;
public:
  virtual ~SizeFormatter() {}

  std::string operator()(int64_t size) const
  {
    return format(size);
  }
};

class AbbrevSizeFormatter:public SizeFormatter {
protected:
  virtual std::string format(int64_t size) const
  {
    return util::abbrevSize(size);
  }
};

class PlainSizeFormatter:public SizeFormatter {
protected:
  virtual std::string format(int64_t size) const
  {
    return util::itos(size);
  }
};

class ConsoleStatCalc:public StatCalc
{
private:
  Timer _cp;

  Timer _lastSummaryNotified;

  time_t _summaryInterval;

  SharedHandle<SizeFormatter> _sizeFormatter;
public:
  ConsoleStatCalc(time_t summaryInterval, bool humanReadable = true);

  virtual ~ConsoleStatCalc() {}

  virtual void calculateStat(const DownloadEngine* e);
};

typedef SharedHandle<ConsoleStatCalc> ConsoleStatCalcHandle;

} // namespace aria2

#endif // _D_CONSOLE_STAT_CALC_H_
