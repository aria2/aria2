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
#ifndef _D_FILL_REQUEST_GROUP_COMMAND_H_
#define _D_FILL_REQUEST_GROUP_COMMAND_H_

#include "Command.h"
#include "TimeA2.h"
#include "RequestGroup.h"
#include "DownloadEngine.h"

class FillRequestGroupCommand : public Command {
private:
  RequestGroups _reservedRequestGroups;
  DownloadEngine* _e;
  int32_t _interval;
  Time _checkPoint;
public:
  FillRequestGroupCommand(int cuid, DownloadEngine* e, int32_t interval):
    Command(cuid),
    _e(e),
    _interval(interval)
  {
    setStatusRealtime();
  }

  virtual bool execute();

  void setInterval(int32_t interval)
  {
    _interval = interval;
  }
};

#endif // _D_FILL_REQUEST_GROUP_COMMAND_H_
