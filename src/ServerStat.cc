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
#include "ServerStat.h"

#include <ostream>
#include <algorithm>

#include "array_fun.h"
#include "LogFactory.h"

namespace aria2 {

const std::string ServerStat::STATUS_STRING[] = {
  "OK",
  "ERROR"
};

ServerStat::ServerStat(const std::string& hostname, const std::string& protocol)
  :
  _hostname(hostname),
  _protocol(protocol),
  _downloadSpeed(0),
  _singleConnectionAvgSpeed(0),
  _multiConnectionAvgSpeed(0),
  _counter(0),
  _logger(LogFactory::getInstance()),
  _status(OK)
{}

ServerStat::~ServerStat() {}

const std::string& ServerStat::getHostname() const
{
  return _hostname;
}

const std::string& ServerStat::getProtocol() const
{
  return _protocol;
}

const Time& ServerStat::getLastUpdated() const
{
  return _lastUpdated;
}

void ServerStat::setLastUpdated(const Time& time)
{
  _lastUpdated = time;
}

unsigned int ServerStat::getDownloadSpeed() const
{
  return _downloadSpeed;
}

void ServerStat::setDownloadSpeed(unsigned int downloadSpeed)
{
  _downloadSpeed = downloadSpeed;
}

void ServerStat::updateDownloadSpeed(unsigned int downloadSpeed)
{
  _downloadSpeed = downloadSpeed;
  if(downloadSpeed > 0) {
    _status = OK;
  }
  _lastUpdated.reset();
}

unsigned int ServerStat::getSingleConnectionAvgSpeed() const
{
  return _singleConnectionAvgSpeed;
}

void ServerStat::setSingleConnectionAvgSpeed
(unsigned int singleConnectionAvgSpeed)
{
  _singleConnectionAvgSpeed = singleConnectionAvgSpeed;
}

void ServerStat::updateSingleConnectionAvgSpeed(unsigned int downloadSpeed)
{
  float avgDownloadSpeed;
  if(_counter == 0)
    return;
  if(_counter < 5) {
    avgDownloadSpeed =
      ((((float)_counter-1)/(float)_counter)*(float)_singleConnectionAvgSpeed)+ 
      ((1.0/(float)_counter)*(float)downloadSpeed);
  }
  else {
    avgDownloadSpeed = ((4.0/5.0)*(float)_singleConnectionAvgSpeed) +
      ((1.0/5.0)*(float)downloadSpeed);
  }
  if(avgDownloadSpeed < (int)(0.80*_singleConnectionAvgSpeed)) {
    _logger->debug("ServerStat:%s: resetting counter since single connection"
		   " speed dropped", getHostname().c_str());
    _counter = 0;
  }
  _logger->debug("ServerStat:%s: _singleConnectionAvgSpeed old:%.2fKB/s"
		 " new:%.2fKB/s last:%.2fKB/s",
		 getHostname().c_str(),
		 (float) _singleConnectionAvgSpeed/1024,
		 (float) avgDownloadSpeed/1024,
		 (float) downloadSpeed / 1024);
  _singleConnectionAvgSpeed = (int)avgDownloadSpeed;
}

unsigned int ServerStat::getMultiConnectionAvgSpeed() const
{
  return _multiConnectionAvgSpeed;
}

void ServerStat::setMultiConnectionAvgSpeed
(unsigned int multiConnectionAvgSpeed)
{
  _multiConnectionAvgSpeed = multiConnectionAvgSpeed;
}

void ServerStat::updateMultiConnectionAvgSpeed(unsigned int downloadSpeed)
{
  float avgDownloadSpeed;
  if(_counter == 0)
    return;
  if(_counter < 5) {
    avgDownloadSpeed =
      ((((float)_counter-1)/(float)_counter)*(float)_multiConnectionAvgSpeed) + 
      ((1.0/(float)_counter)*(float)downloadSpeed);
  }
  else {
    avgDownloadSpeed = ((4.0/5.0)*(float)_multiConnectionAvgSpeed) +
      ((1.0/5.0)*(float)downloadSpeed);
  }
  _logger->debug("ServerStat:%s: _multiConnectionAvgSpeed old:%.2fKB/s"
		 " new:%.2fKB/s last:%.2fKB/s",
		 getHostname().c_str(),
		 (float) _multiConnectionAvgSpeed/1024,
		 (float) avgDownloadSpeed/1024,
		 (float) downloadSpeed / 1024);
  _multiConnectionAvgSpeed = (int)avgDownloadSpeed;
}

unsigned int ServerStat::getCounter() const
{
  return _counter;
}

void ServerStat::increaseCounter()
{
  _counter++;
}

void ServerStat::setCounter(unsigned int value)
{
  _counter = value;
}

void ServerStat::setStatus(STATUS status)
{
  _status = status;
}

void ServerStat::setStatus(const std::string& status)
{
  size_t len = arrayLength(STATUS_STRING);
  const std::string* p = std::find(&STATUS_STRING[0],
				   &STATUS_STRING[len],
				   status);
  if(p != &STATUS_STRING[len]) {
    _status = static_cast<STATUS>(ServerStat::OK+
				  std::distance(&STATUS_STRING[0], p));
  }
}

void ServerStat::setStatusInternal(STATUS status)
{
  _status = status;
  _lastUpdated.reset();
}

ServerStat::STATUS ServerStat::getStatus() const
{
  return _status;
}

bool ServerStat::isOK() const
{
  return _status == OK;
}

void ServerStat::setOK()
{
  setStatusInternal(OK);
}

bool ServerStat::isError() const
{
  return _status == ERROR;
}

void ServerStat::setError()
{
  setStatusInternal(ERROR);
}

bool ServerStat::operator<(const ServerStat& serverStat) const
{
  int c = _hostname.compare(serverStat._hostname);
  if(c == 0) {
    return _protocol < serverStat._protocol;
  } else {
    return c < 0;
  }
}

bool ServerStat::operator==(const ServerStat& serverStat) const
{
  return _hostname == serverStat._hostname && _protocol == serverStat._protocol;
}

std::ostream& operator<<(std::ostream& o, const ServerStat& serverStat)
{
  o << "host=" << serverStat.getHostname() << ", "
    << "protocol=" << serverStat.getProtocol() << ", "
    << "dl_speed=" << serverStat.getDownloadSpeed() << ", "
    << "sc_avg_speed=" << serverStat.getSingleConnectionAvgSpeed() << ", "
    << "mc_avg_speed=" << serverStat.getMultiConnectionAvgSpeed() << ", "
    << "last_updated=" << serverStat.getLastUpdated().getTime() << ", "
    << "counter=" << serverStat.getCounter() << ", "
    << "status=" << ServerStat::STATUS_STRING[serverStat.getStatus()];
  return o;
}

} // namespace aria2
