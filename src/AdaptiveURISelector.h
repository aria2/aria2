/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2006 Tatsuhiro Tsujikawa
 * Copyright (C) 2008 Aurelien Lefebvre, Mandriva
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
#ifndef D_ADAPTIVE_URI_SELECTOR_H
#define D_ADAPTIVE_URI_SELECTOR_H
#include "URISelector.h"

#include <memory>
#include <chrono>

namespace aria2 {

class ServerStatMan;
class RequestGroup;
class ServerStat;

class AdaptiveURISelector : public URISelector {
private:
  std::shared_ptr<ServerStatMan> serverStatMan_;
  // No need to delete requestGroup_
  RequestGroup* requestGroup_;
  int nbServerToEvaluate_;
  int nbConnections_;

  void mayRetryWithIncreasedTimeout(FileEntry* fileEntry);

  std::string selectOne(const std::deque<std::string>& uris);
  void adjustLowestSpeedLimit(const std::deque<std::string>& uris,
                              DownloadCommand* command) const;
  int getMaxDownloadSpeed(const std::deque<std::string>& uris) const;
  std::string getMaxDownloadSpeedUri(const std::deque<std::string>& uris) const;
  std::deque<std::string> getUrisBySpeed(const std::deque<std::string>& uris,
                                         int min) const;
  std::string selectRandomUri(const std::deque<std::string>& uris) const;
  std::string getFirstNotTestedUri(const std::deque<std::string>& uris) const;
  std::string getFirstToTestUri(const std::deque<std::string>& uris) const;
  std::shared_ptr<ServerStat> getServerStats(const std::string& uri) const;
  int getNbTestedServers(const std::deque<std::string>& uris) const;
  std::string getBestMirror(const std::deque<std::string>& uris) const;

public:
  AdaptiveURISelector(std::shared_ptr<ServerStatMan> serverStatMan,
                      RequestGroup* requestGroup);

  virtual ~AdaptiveURISelector();

  virtual std::string
  select(FileEntry* fileEntry,
         const std::vector<std::pair<size_t, std::string>>& usedHosts)
      CXX11_OVERRIDE;

  virtual void tuneDownloadCommand(const std::deque<std::string>& uris,
                                   DownloadCommand* command) CXX11_OVERRIDE;

  virtual void resetCounters() CXX11_OVERRIDE;
};

} // namespace aria2
#endif // D_ADAPTIVE_URI_SELECTOR_H
