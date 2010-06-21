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
  hostname_(hostname),
  protocol_(protocol),
  downloadSpeed_(0),
  singleConnectionAvgSpeed_(0),
  multiConnectionAvgSpeed_(0),
  counter_(0),
  logger_(LogFactory::getInstance()),
  status_(OK)
{}

ServerStat::~ServerStat() {}

void ServerStat::setLastUpdated(const Time& time)
{
  lastUpdated_ = time;
}

void ServerStat::setDownloadSpeed(unsigned int downloadSpeed)
{
  downloadSpeed_ = downloadSpeed;
}

void ServerStat::updateDownloadSpeed(unsigned int downloadSpeed)
{
  downloadSpeed_ = downloadSpeed;
  if(downloadSpeed > 0) {
    status_ = OK;
  }
  lastUpdated_.reset();
}

void ServerStat::setSingleConnectionAvgSpeed
(unsigned int singleConnectionAvgSpeed)
{
  singleConnectionAvgSpeed_ = singleConnectionAvgSpeed;
}

void ServerStat::updateSingleConnectionAvgSpeed(unsigned int downloadSpeed)
{
  float avgDownloadSpeed;
  if(counter_ == 0)
    return;
  if(counter_ < 5) {
    avgDownloadSpeed =
      ((((float)counter_-1)/(float)counter_)*(float)singleConnectionAvgSpeed_)+ 
      ((1.0/(float)counter_)*(float)downloadSpeed);
  }
  else {
    avgDownloadSpeed = ((4.0/5.0)*(float)singleConnectionAvgSpeed_) +
      ((1.0/5.0)*(float)downloadSpeed);
  }
  if(avgDownloadSpeed < (int)(0.80*singleConnectionAvgSpeed_)) {
    if(logger_->debug()) {
      logger_->debug("ServerStat:%s: resetting counter since single connection"
                     " speed dropped", getHostname().c_str());
    }
    counter_ = 0;
  }
  if(logger_->debug()) {
    logger_->debug("ServerStat:%s: singleConnectionAvgSpeed_ old:%.2fKB/s"
                   " new:%.2fKB/s last:%.2fKB/s",
                   getHostname().c_str(),
                   (float) singleConnectionAvgSpeed_/1024,
                   (float) avgDownloadSpeed/1024,
                   (float) downloadSpeed / 1024);
  }
  singleConnectionAvgSpeed_ = (int)avgDownloadSpeed;
}

void ServerStat::setMultiConnectionAvgSpeed
(unsigned int multiConnectionAvgSpeed)
{
  multiConnectionAvgSpeed_ = multiConnectionAvgSpeed;
}

void ServerStat::updateMultiConnectionAvgSpeed(unsigned int downloadSpeed)
{
  float avgDownloadSpeed;
  if(counter_ == 0)
    return;
  if(counter_ < 5) {
    avgDownloadSpeed =
      ((((float)counter_-1)/(float)counter_)*(float)multiConnectionAvgSpeed_) + 
      ((1.0/(float)counter_)*(float)downloadSpeed);
  }
  else {
    avgDownloadSpeed = ((4.0/5.0)*(float)multiConnectionAvgSpeed_) +
      ((1.0/5.0)*(float)downloadSpeed);
  }
  if(logger_->debug()) {
    logger_->debug("ServerStat:%s: multiConnectionAvgSpeed_ old:%.2fKB/s"
                   " new:%.2fKB/s last:%.2fKB/s",
                   getHostname().c_str(),
                   (float) multiConnectionAvgSpeed_/1024,
                   (float) avgDownloadSpeed/1024,
                   (float) downloadSpeed / 1024);
  }
  multiConnectionAvgSpeed_ = (int)avgDownloadSpeed;
}

void ServerStat::increaseCounter()
{
  ++counter_;
}

void ServerStat::setCounter(unsigned int value)
{
  counter_ = value;
}

void ServerStat::setStatus(STATUS status)
{
  status_ = status;
}

void ServerStat::setStatus(const std::string& status)
{
  const std::string* p = std::find(vbegin(STATUS_STRING), vend(STATUS_STRING),
                                   status);
  if(p != vend(STATUS_STRING)) {
    status_ = static_cast<STATUS>(ServerStat::OK+
                                  std::distance(vbegin(STATUS_STRING), p));
  }
}

void ServerStat::setStatusInternal(STATUS status)
{
  if(logger_->debug()) {
    logger_->debug("ServerStat: set status %s for %s (%s)",
                   STATUS_STRING[status].c_str(),
                   hostname_.c_str(), protocol_.c_str());
  }
  status_ = status;
  lastUpdated_.reset();
}

void ServerStat::setOK()
{
  setStatusInternal(OK);
}

void ServerStat::setError()
{
  setStatusInternal(ERROR);
}

bool ServerStat::operator<(const ServerStat& serverStat) const
{
  int c = hostname_.compare(serverStat.hostname_);
  if(c == 0) {
    return protocol_ < serverStat.protocol_;
  } else {
    return c < 0;
  }
}

bool ServerStat::operator==(const ServerStat& serverStat) const
{
  return hostname_ == serverStat.hostname_ && protocol_ == serverStat.protocol_;
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
