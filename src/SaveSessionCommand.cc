/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2013 Tatsuhiro Tsujikawa
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
#include "SaveSessionCommand.h"
#include "DownloadEngine.h"
#include "RequestGroupMan.h"
#include "SessionSerializer.h"
#include "prefs.h"
#include "fmt.h"
#include "LogFactory.h"
#include "Option.h"

namespace aria2 {

SaveSessionCommand::SaveSessionCommand(cuid_t cuid, DownloadEngine* e,
                                       std::chrono::seconds interval)
    : TimeBasedCommand(cuid, e, std::move(interval), true)
{
}

SaveSessionCommand::~SaveSessionCommand() = default;

void SaveSessionCommand::preProcess()
{
  if (getDownloadEngine()->getRequestGroupMan()->downloadFinished() ||
      getDownloadEngine()->isHaltRequested()) {
    enableExit();
  }
}

void SaveSessionCommand::process()
{
  const std::string& filename =
      getDownloadEngine()->getOption()->get(PREF_SAVE_SESSION);
  if (!filename.empty()) {
    auto& rgman = getDownloadEngine()->getRequestGroupMan();

    SessionSerializer sessionSerializer(rgman.get());

    auto sessionHash = sessionSerializer.calculateHash();
    if (rgman->getLastSessionHash() == sessionHash) {
      A2_LOG_INFO("No change since last serialization or startup. "
                  "No serialization is necessary this time.");
      return;
    }

    rgman->setLastSessionHash(std::move(sessionHash));

    if (sessionSerializer.save(filename)) {
      A2_LOG_NOTICE(
          fmt(_("Serialized session to '%s' successfully."), filename.c_str()));
    }
    else {
      A2_LOG_ERROR(
          fmt(_("Failed to serialize session to '%s'."), filename.c_str()));
    }
  }
}

} // namespace aria2
