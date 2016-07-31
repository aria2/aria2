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
#include "ServerStatMan.h"

#include <cstring>
#include <cstdio>
#include <algorithm>
#include <iterator>
#include <vector>

#include "ServerStat.h"
#include "util.h"
#include "RecoverableException.h"
#include "a2functional.h"
#include "BufferedFile.h"
#include "message.h"
#include "fmt.h"
#include "LogFactory.h"
#include "File.h"

namespace aria2 {

ServerStatMan::ServerStatMan() {}

ServerStatMan::~ServerStatMan() = default;

std::shared_ptr<ServerStat>
ServerStatMan::find(const std::string& hostname,
                    const std::string& protocol) const
{
  auto ss = std::make_shared<ServerStat>(hostname, protocol);
  auto i = serverStats_.find(ss);
  if (i == serverStats_.end()) {
    return nullptr;
  }
  else {
    return *i;
  }
}

bool ServerStatMan::add(const std::shared_ptr<ServerStat>& serverStat)
{
  auto i = serverStats_.lower_bound(serverStat);
  if (i != serverStats_.end() && *(*i) == *serverStat) {
    return false;
  }
  else {
    serverStats_.insert(i, serverStat);
    return true;
  }
}

bool ServerStatMan::save(const std::string& filename) const
{
  std::string tempfile = filename;
  tempfile += "__temp";
  {
    BufferedFile fp(tempfile.c_str(), BufferedFile::WRITE);
    if (!fp) {
      A2_LOG_ERROR(
          fmt(MSG_OPENING_WRITABLE_SERVER_STAT_FILE_FAILED, filename.c_str()));
      return false;
    }
    for (auto& e : serverStats_) {
      std::string l = e->toString();
      l += "\n";
      if (fp.write(l.data(), l.size()) != l.size()) {
        A2_LOG_ERROR(
            fmt(MSG_WRITING_SERVER_STAT_FILE_FAILED, filename.c_str()));
      }
    }
    if (fp.close() == EOF) {
      A2_LOG_ERROR(fmt(MSG_WRITING_SERVER_STAT_FILE_FAILED, filename.c_str()));
      return false;
    }
  }
  if (File(tempfile).renameTo(filename)) {
    A2_LOG_NOTICE(fmt(MSG_SERVER_STAT_SAVED, filename.c_str()));
    return true;
  }
  else {
    A2_LOG_ERROR(fmt(MSG_WRITING_SERVER_STAT_FILE_FAILED, filename.c_str()));
    return false;
  }
}

namespace {
// Field and FIELD_NAMES must have same order except for MAX_FIELD.
enum Field {
  S_COUNTER,
  S_DL_SPEED,
  S_HOST,
  S_LAST_UPDATED,
  S_MC_AVG_SPEED,
  S_PROTOCOL,
  S_SC_AVG_SPEED,
  S_STATUS,
  MAX_FIELD
};

const char* FIELD_NAMES[] = {
    "counter",      "dl_speed", "host",         "last_updated",
    "mc_avg_speed", "protocol", "sc_avg_speed", "status",
};
} // namespace

namespace {
int idField(std::string::const_iterator first, std::string::const_iterator last)
{
  int i;
  for (i = 0; i < MAX_FIELD; ++i) {
    if (util::streq(first, last, FIELD_NAMES[i])) {
      return i;
    }
  }
  return i;
}
} // namespace

bool ServerStatMan::load(const std::string& filename)
{
  BufferedFile fp(filename.c_str(), BufferedFile::READ);
  if (!fp) {
    A2_LOG_ERROR(
        fmt(MSG_OPENING_READABLE_SERVER_STAT_FILE_FAILED, filename.c_str()));
    return false;
  }
  while (1) {
    std::string line = fp.getLine();
    if (line.empty()) {
      if (fp.eof()) {
        break;
      }
      else if (!fp) {
        A2_LOG_ERROR(
            fmt(MSG_READING_SERVER_STAT_FILE_FAILED, filename.c_str()));
        return false;
      }
      else {
        continue;
      }
    }
    std::pair<std::string::const_iterator, std::string::const_iterator> p =
        util::stripIter(line.begin(), line.end());
    if (p.first == p.second) {
      continue;
    }
    std::vector<Scip> items;
    util::splitIter(p.first, p.second, std::back_inserter(items), ',');
    std::vector<std::string> m(MAX_FIELD);
    for (std::vector<Scip>::const_iterator i = items.begin(), eoi = items.end();
         i != eoi; ++i) {
      auto p = util::divide((*i).first, (*i).second, '=');
      int id = idField(p.first.first, p.first.second);
      if (id != MAX_FIELD) {
        m[id].assign(p.second.first, p.second.second);
      }
    }
    if (m[S_HOST].empty() || m[S_PROTOCOL].empty()) {
      continue;
    }
    auto sstat = std::make_shared<ServerStat>(m[S_HOST], m[S_PROTOCOL]);

    uint32_t uintval;
    if (!util::parseUIntNoThrow(uintval, m[S_DL_SPEED])) {
      continue;
    }
    sstat->setDownloadSpeed(uintval);
    // Old serverstat file doesn't contains SC_AVG_SPEED
    if (!m[S_SC_AVG_SPEED].empty()) {
      if (!util::parseUIntNoThrow(uintval, m[S_SC_AVG_SPEED])) {
        continue;
      }
      sstat->setSingleConnectionAvgSpeed(uintval);
    }
    // Old serverstat file doesn't contains MC_AVG_SPEED
    if (!m[S_MC_AVG_SPEED].empty()) {
      if (!util::parseUIntNoThrow(uintval, m[S_MC_AVG_SPEED])) {
        continue;
      }
      sstat->setMultiConnectionAvgSpeed(uintval);
    }
    // Old serverstat file doesn't contains COUNTER_SPEED
    if (!m[S_COUNTER].empty()) {
      if (!util::parseUIntNoThrow(uintval, m[S_COUNTER])) {
        continue;
      }
      sstat->setCounter(uintval);
    }
    int32_t intval;
    if (!util::parseIntNoThrow(intval, m[S_LAST_UPDATED])) {
      continue;
    }
    sstat->setLastUpdated(Time(intval));
    sstat->setStatus(m[S_STATUS]);
    add(sstat);
  }
  A2_LOG_NOTICE(fmt(MSG_SERVER_STAT_LOADED, filename.c_str()));
  return true;
}

void ServerStatMan::removeStaleServerStat(const std::chrono::seconds& timeout)
{
  auto now = Time();
  for (auto i = std::begin(serverStats_); i != std::end(serverStats_);) {
    if ((*i)->getLastUpdated().difference(now) >= timeout) {
      serverStats_.erase(i++);
    }
    else {
      ++i;
    }
  }
}

} // namespace aria2
