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
#ifndef _D_HTTP_REQUEST_H_
#define _D_HTTP_REQUEST_H_

#include "common.h"
#include "Segment.h"
#include "Range.h"
#include "Request.h"
#include "Option.h"
#include <netinet/in.h>

class HttpRequest {
private:

  RequestHandle request;

  SegmentHandle segment;

  int64_t entityLength;

  bool authEnabled;

  bool proxyEnabled;

  bool proxyAuthEnabled;

  string userAgent;

  string getHostText(const string& host, in_port_t port) const;

  string getProxyAuthString() const;

public:
  HttpRequest():request(0),
		segment(0),
		entityLength(0),
		authEnabled(false),
		proxyEnabled(false),
		proxyAuthEnabled(false),
		userAgent(USER_AGENT)
  {}

  SegmentHandle getSegment() const
  {
    return segment;
  }

  void setSegment(const SegmentHandle& segment)
  {
    this->segment = segment;
  }

  void setRequest(const RequestHandle& request)
  {
    this->request = request;
  }

  /**
   * entityLength is used in isRangeSatisfied() method.
   */
  void setEntityLength(int64_t entityLength)
  {
    this->entityLength = entityLength;
  }

  int64_t getEntityLength() const
  {
    return entityLength;
  }

  string getHost() const
  {
    return request->getHost();
  }

  in_port_t getPort() const
  {
    return request->getPort();
  }

  string getMethod() const
  {
    return request->getMethod();
  }

  string getProtocol() const
  {
    return request->getProtocol();
  }

  string getCurrentURI() const
  {
    return request->getCurrentUrl();
  }
  
  string getDir() const
  {
    return request->getDir();
  }

  string getFile() const
  {
    return request->getFile();
  }

  string getPreviousURI() const
  {
    return request->getPreviousUrl();
  }

  RangeHandle getRange() const;

  /**
   * Inspects whether the specified response range is satisfiable
   * with request range.
   */
  bool isRangeSatisfied(const RangeHandle& range) const;

  RequestHandle getRequest() const
  {
    return request;
  }

  int64_t getStartByte() const
  {
    if(segment.isNull()) {
      return 0;
    } else {
      return segment->getPositionToWrite();
    }
  }

  int64_t getEndByte() const
  {
    if(segment.isNull() || request.isNull()) {
      return 0;
    } else {
      if(request->isKeepAlive()) {
	return segment->getPosition()+segment->length-1;
      } else {
	return 0;
      }
    }
  }

  /**
   * Returns string representation of http request.
   * It usually starts with "GET ..." and ends with "\r\n".
   */
  string createRequest() const;

  /**
   * Returns string representation of http tunnel request.
   * It usually starts with "CONNECT ..." and ends with "\r\n".
   */
  string createProxyRequest() const;

  /**
   * Configures this object with option.
   * Following values are evaluated:
   * PREF_HTTP_AUTH_ENABLED, PREF_HTTP_PROXY_ENABLED,
   * PREF_HTTP_PROXY_METHOD, PREF_HTTP_PROXY_AUTH_ENABLED,
   * PREF_HTTP_USER, PREF_HTTP_PASSWD,
   * PREF_HTTP_PROXY_USER, PREF_HTTP_PROXY_PASSWD
   * The evaluation results are stored in instance variables.
   */
  void configure(const Option* option);

  void setProxyEnabled(bool proxyEnabled)
  {
    this->proxyEnabled = proxyEnabled;
  }

  void setProxyAuthEnabled(bool proxyAuthEnabled)
  {
    this->proxyAuthEnabled = proxyAuthEnabled;
  }

  void setAuthEnabled(bool authEnabled)
  {
    this->authEnabled = authEnabled;
  }

  void setUserAgent(const string& userAgent)
  {
    this->userAgent = userAgent;
  }
};

typedef SharedHandle<HttpRequest> HttpRequestHandle;
typedef deque<HttpRequestHandle> HttpRequests;

#endif // _D_HTTP_REQUEST_H_
