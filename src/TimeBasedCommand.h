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
#ifndef _D_TIME_BASED_COMMAND_H_
#define _D_TIME_BASED_COMMAND_H_

#include "Command.h"
#include "TimerA2.h"

namespace aria2 {

class DownloadEngine;

class TimeBasedCommand : public Command
{
protected:
  DownloadEngine* _e;

  /**
   * setting _exit to true if this command's job has finished and you want to
   * delete this command.
   * The _exit variable is evaluated  after preProcess(), process(),
   * postProcess(), and terminate processing immediately and excute() returns
   * true.
   */
  bool _exit;

  time_t _interval; // unit: sec

  bool _routineCommand;
private:
  Timer _checkPoint;
public:
  /**
   * preProcess() is called each time when excute() is called.
   */
  virtual void preProcess() {};

  /**
   * process() is called only when excute() is called and specified time has
   * elapsed.
   */
  virtual void process() = 0;

  /**
   * postProcess() is called each time when excute() is called.
   */
  virtual void postProcess() {};

public:
  TimeBasedCommand(cuid_t cuid, DownloadEngine* e, time_t interval,
                   bool routineCommand = false);

  virtual ~TimeBasedCommand();

  virtual bool execute();
};

} // namespace aria2

#endif // _D_TIME_BASED_COMMAND_H_
