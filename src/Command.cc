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
#include "Command.h"
#include "LogFactory.h"
#include "Logger.h"

namespace aria2 {

int32_t Command::uuidGen = 0;

Command::Command(cuid_t cuid):uuid(uuidGen++),
                              _status(STATUS_INACTIVE),
                              _cuid(cuid),
                              _logger(LogFactory::getInstance()),
                              _readEvent(false),
                              _writeEvent(false),
                              _errorEvent(false),
                              _hupEvent(false) {}

void Command::transitStatus()
{
  switch(_status) {
  case STATUS_REALTIME:
    break;
  default:
    _status = STATUS_INACTIVE;
  }
}

void Command::setStatus(STATUS status)
{
  _status = status;
}

void Command::readEventReceived()
{
  _readEvent = true;
}

void Command::writeEventReceived()
{
  _writeEvent = true;
}

void Command::errorEventReceived()
{
  _errorEvent = true;
}

void Command::hupEventReceived()
{
  _hupEvent = true;
}

void Command::clearIOEvents()
{
  _readEvent = false;
  _writeEvent = false;
  _errorEvent = false;
  _hupEvent = false;
}

} // namespace aria2
