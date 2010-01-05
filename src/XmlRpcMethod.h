/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2009 Tatsuhiro Tsujikawa
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
#ifndef _D_XML_RPC_METHOD_H_
#define _D_XML_RPC_METHOD_H_

#include "common.h"

#include <string>

#include "SharedHandle.h"

namespace aria2 {

class DownloadEngine;
class OptionParser;
class BDE;
class Logger;
class Option;
class Exception;

namespace xmlrpc {

struct XmlRpcRequest;
struct XmlRpcResponse;

// This class offers abstract implementation of processing XML-RPC
// request. You have to inherit this class and implement process()
// method to add new XML-RPC API.
//
// There is XmlRpcMethodFactory class which instantiates XmlRpcMethod
// subclass. If you add new XmlRpcMethod subclass, don't forget to add
// it to XmlRpcMethodFactory.
class XmlRpcMethod {
protected:
  SharedHandle<OptionParser> _optionParser;

  Logger* _logger;

  // Subclass must implement this function to fulfil XmlRpcRequest
  // req.  The return value of this method is used as a return value
  // of XML-RPC request.
  virtual BDE process(const XmlRpcRequest& req, DownloadEngine* e) = 0;

  void gatherRequestOption(const SharedHandle<Option>& option,
                           const BDE& optionsDict);

  void gatherChangeableOption(const SharedHandle<Option>& option,
                              const BDE& optionDict);

  // Copy options which is changeable in XML-RPC changeOption command
  // to dest.
  void applyChangeableOption(Option* dest, Option* src) const;

  void gatherChangeableGlobalOption(const SharedHandle<Option>& option,
                                    const BDE& optionDict);

  // Copy options which is changeable in XML-RPC changeGlobalOption
  // command to dest.
  void applyChangeableGlobalOption(Option* dest, Option* src) const;

  BDE createErrorResponse(const Exception& e);
public:
  XmlRpcMethod();

  virtual ~XmlRpcMethod() {}

  // Do work to fulfill XmlRpcRequest req and returns its result as
  // XmlRpcResponse. This method delegates to process() method.
  XmlRpcResponse execute(const XmlRpcRequest& req, DownloadEngine* e);
};

} // namespace xmlrpc

} // namespace aria2

#endif // _D_XML_RPC_METHOD_H_
