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
#ifndef _D_SERVER_STAT_H_
#define _D_SERVER_STAT_H_
#include "common.h"

#include <string>
#include <iosfwd>

#include "TimeA2.h"

namespace aria2 {

class Logger;

// ServerStatMan: has many ServerStat
// URISelector: interface
// ServerStatURISelector: Has a reference of ServerStatMan
// InOrderURISelector: this is default.
class ServerStat {
public:
  enum STATUS {
    OK = 0,
    ERROR
  };
  
  static const std::string STATUS_STRING[];

  ServerStat(const std::string& hostname, const std::string& protocol);

  ~ServerStat();

  const std::string& getHostname() const
  {
    return _hostname;
  }

  const std::string& getProtocol() const
  {
    return _protocol;
  }

  const Time& getLastUpdated() const
  {
    return _lastUpdated;
  }

  // This method doesn't update _lastUpdate.
  void setLastUpdated(const Time& time);

  unsigned int getDownloadSpeed() const
  {
    return _downloadSpeed;
  }

  // update download speed and update _lastUpdated
  void updateDownloadSpeed(unsigned int downloadSpeed);

  // set download speed. This method doesn't update _lastUpdate.
  void setDownloadSpeed(unsigned int downloadSpeed);

  unsigned int getSingleConnectionAvgSpeed() const
  {
    return _singleConnectionAvgSpeed;
  }

  void updateSingleConnectionAvgSpeed(unsigned int downloadSpeed);
  void setSingleConnectionAvgSpeed(unsigned int singleConnectionAvgSpeed);

  unsigned int getMultiConnectionAvgSpeed() const
  {
    return _multiConnectionAvgSpeed;
  }

  void updateMultiConnectionAvgSpeed(unsigned int downloadSpeed);
  void setMultiConnectionAvgSpeed(unsigned int singleConnectionAvgSpeed);

  unsigned int getCounter() const
  {
    return _counter;
  }

  void increaseCounter();
  void setCounter(unsigned int value);

  // This method doesn't update _lastUpdate.
  void setStatus(STATUS status);

  // status should be one of the followings: "OK", "ERROR".
  // Giving other string will not change the status of this object.
  // This method doesn't update _lastUpdate.
  void setStatus(const std::string& status);

  STATUS getStatus() const
  {
    return _status;
  }

  bool isOK() const
  {
    return _status == OK;
  }

  // set status OK and update _lastUpdated
  void setOK();

  bool isError() const
  {
    return _status == ERROR;
  }

  // set status ERROR and update _lastUpdated
  void setError();

  bool operator<(const ServerStat& serverStat) const;

  bool operator==(const ServerStat& serverStat) const;
private:
  std::string _hostname;
  
  std::string _protocol;

  unsigned int _downloadSpeed;
  
  unsigned int _singleConnectionAvgSpeed;
  
  unsigned int _multiConnectionAvgSpeed;

  unsigned int _counter;

  Logger* _logger;

  STATUS _status;

  Time _lastUpdated;

  void setStatusInternal(STATUS status);
};

std::ostream& operator<<(std::ostream& o, const ServerStat& serverStat);

} // namespace aria2

#endif // _D_SERVER_STAT_H_
