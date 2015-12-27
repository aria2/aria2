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
#ifndef D_SEQUENTIAL_DISPATCHER_COMMAND_H
#define D_SEQUENTIAL_DISPATCHER_COMMAND_H

#include "Command.h"

#include <memory>

#include "SequentialPicker.h"
#include "DownloadEngine.h"
#include "RequestGroupMan.h"

namespace aria2 {

class DownloadEngine;

template <typename T> class SequentialDispatcherCommand : public Command {
private:
  SequentialPicker<T>* picker_;

  DownloadEngine* e_;

protected:
  DownloadEngine* getDownloadEngine() const { return e_; }

public:
  SequentialDispatcherCommand(cuid_t cuid, SequentialPicker<T>* picker,
                              DownloadEngine* e)
      : Command{cuid}, picker_{picker}, e_{e}
  {
    setStatusRealtime();
  }

  virtual bool execute() CXX11_OVERRIDE
  {
    if (e_->getRequestGroupMan()->downloadFinished() || e_->isHaltRequested()) {
      return true;
    }
    if (picker_->hasNext() && !picker_->isPicked()) {
      e_->addCommand(createCommand(picker_->pickNext()));

      e_->setNoWait(true);
    }

    e_->addRoutineCommand(std::unique_ptr<Command>(this));
    return false;
  }

protected:
  virtual std::unique_ptr<Command> createCommand(T* entry) = 0;
};

} // namespace aria2

#endif // D_SEQUENTIAL_DISPATCHER_COMMAND_H
