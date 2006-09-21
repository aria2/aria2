/* <!-- copyright */
/*
 * aria2 - a simple utility for downloading files faster
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
/* copyright --> */
#ifndef _D_URL_REQUEST_INFO_H_
#define _D_URL_REQUEST_INFO_H_

#include "RequestInfo.h"

class UrlRequestInfo : public RequestInfo {
private:
  Strings urls;
  int maxConnections;
  DownloadEngine* e;

  RequestInfo* createNextRequestInfo() const;
  void adjustRequestSize(Requests& requests,
			 Requests& reserved,
			 int maxConnections) const;
public:
  UrlRequestInfo(const Strings& urls, int maxConnections, Option* op):
    RequestInfo(op),
    urls(urls),
    maxConnections(maxConnections),
    e(0) {}
  virtual ~UrlRequestInfo() {}

  virtual RequestInfo* execute();

  virtual DownloadEngine* getDownloadEngine() {
    return e;
  }
};

#endif // _D_URL_REQUEST_INFO_H_
