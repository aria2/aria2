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
#ifndef D_SERVER_STAT_H
#define D_SERVER_STAT_H
#include "common.h"

#include <string>
#include <iosfwd>
#include <memory>

#include "TimeA2.h"

namespace aria2 {

// ServerStatMan: has many ServerStat
// URISelector: interface
// ServerStatURISelector: Has a reference of ServerStatMan
// InOrderURISelector: this is default.
class ServerStat {
public:
  enum STATUS { OK = 0, A2_ERROR, MAX_STATUS };

  ServerStat(const std::string& hostname, const std::string& protocol);

  ~ServerStat();

  const std::string& getHostname() const { return hostname_; }

  const std::string& getProtocol() const { return protocol_; }

  const Time& getLastUpdated() const { return lastUpdated_; }

  // This method doesn't update _lastUpdate.
  void setLastUpdated(const Time& time);

  int getDownloadSpeed() const { return downloadSpeed_; }

  // update download speed and update lastUpdated_
  void updateDownloadSpeed(int downloadSpeed);

  // set download speed. This method doesn't update _lastUpdate.
  void setDownloadSpeed(int downloadSpeed);

  int getSingleConnectionAvgSpeed() const { return singleConnectionAvgSpeed_; }

  void updateSingleConnectionAvgSpeed(int downloadSpeed);
  void setSingleConnectionAvgSpeed(int singleConnectionAvgSpeed);

  int getMultiConnectionAvgSpeed() const { return multiConnectionAvgSpeed_; }

  void updateMultiConnectionAvgSpeed(int downloadSpeed);
  void setMultiConnectionAvgSpeed(int singleConnectionAvgSpeed);

  int getCounter() const { return counter_; }

  void increaseCounter();
  void setCounter(int value);

  // This method doesn't update _lastUpdate.
  void setStatus(STATUS status);

  // status should be one of the followings: "OK", "ERROR".
  // Giving other string will not change the status of this object.
  // This method doesn't update _lastUpdate.
  void setStatus(const std::string& status);

  STATUS getStatus() const { return status_; }

  bool isOK() const { return status_ == OK; }

  // set status OK and update lastUpdated_
  void setOK();

  bool isError() const { return status_ == A2_ERROR; }

  // set status ERROR and update lastUpdated_
  void setError();

  bool operator<(const ServerStat& serverStat) const;

  bool operator==(const ServerStat& serverStat) const;

  std::string toString() const;

private:
  std::string hostname_;

  std::string protocol_;

  int downloadSpeed_;

  int singleConnectionAvgSpeed_;

  int multiConnectionAvgSpeed_;

  int counter_;

  STATUS status_;

  Time lastUpdated_;

  void setStatusInternal(STATUS status);
};

class ServerStatFaster {
public:
  bool operator()(
      const std::pair<std::shared_ptr<ServerStat>, std::string> lhs,
      const std::pair<std::shared_ptr<ServerStat>, std::string> rhs) const
  {
    return lhs.first->getDownloadSpeed() > rhs.first->getDownloadSpeed();
  }
};

} // namespace aria2

#endif // D_SERVER_STAT_H
