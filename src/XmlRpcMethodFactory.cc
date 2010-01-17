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
#include "XmlRpcMethodFactory.h"
#include "XmlRpcMethodImpl.h"
#include "OptionParser.h"
#include "OptionHandler.h"

namespace aria2 {

namespace xmlrpc {

SharedHandle<XmlRpcMethod>
XmlRpcMethodFactory::create(const std::string& methodName)
{
  if(methodName == AddUriXmlRpcMethod::getMethodName()) {
    return SharedHandle<XmlRpcMethod>(new AddUriXmlRpcMethod());
#ifdef ENABLE_BITTORRENT
  } else if(methodName == AddTorrentXmlRpcMethod::getMethodName()) {
    return SharedHandle<XmlRpcMethod>(new AddTorrentXmlRpcMethod());
#endif // ENABLE_BITTORRENT
#ifdef ENABLE_METALINK
  }
  else if(methodName == AddMetalinkXmlRpcMethod::getMethodName()) {
    return SharedHandle<XmlRpcMethod>(new AddMetalinkXmlRpcMethod());
#endif // ENABLE_METALINK
  }
  else if(methodName == RemoveXmlRpcMethod::getMethodName()) {
    return SharedHandle<XmlRpcMethod>(new RemoveXmlRpcMethod());
  } else if(methodName == ChangePositionXmlRpcMethod::getMethodName()) {
    return SharedHandle<XmlRpcMethod>(new ChangePositionXmlRpcMethod());
  } else if(methodName == TellStatusXmlRpcMethod::getMethodName()) {
    return SharedHandle<XmlRpcMethod>(new TellStatusXmlRpcMethod());
  } else if(methodName == GetUrisXmlRpcMethod::getMethodName()) {
    return SharedHandle<XmlRpcMethod>(new GetUrisXmlRpcMethod());
  } else if(methodName == GetFilesXmlRpcMethod::getMethodName()) {
    return SharedHandle<XmlRpcMethod>(new GetFilesXmlRpcMethod());
#ifdef ENABLE_BITTORRENT
  }
  else if(methodName == GetPeersXmlRpcMethod::getMethodName()) {
    return SharedHandle<XmlRpcMethod>(new GetPeersXmlRpcMethod());
#endif // ENABLE_BITTORRENT
  } else if(methodName == TellActiveXmlRpcMethod::getMethodName()) {
    return SharedHandle<XmlRpcMethod>(new TellActiveXmlRpcMethod());
  } else if(methodName == TellWaitingXmlRpcMethod::getMethodName()) {
    return SharedHandle<XmlRpcMethod>(new TellWaitingXmlRpcMethod());
  } else if(methodName == TellStoppedXmlRpcMethod::getMethodName()) {
    return SharedHandle<XmlRpcMethod>(new TellStoppedXmlRpcMethod());
  } else if(methodName == GetOptionXmlRpcMethod::getMethodName()) {
    return SharedHandle<XmlRpcMethod>(new GetOptionXmlRpcMethod());
  } else if(methodName == ChangeOptionXmlRpcMethod::getMethodName()) {
    return SharedHandle<XmlRpcMethod>(new ChangeOptionXmlRpcMethod());
  } else if(methodName == GetGlobalOptionXmlRpcMethod::getMethodName()) {
    return SharedHandle<XmlRpcMethod>(new GetGlobalOptionXmlRpcMethod());
  } else if(methodName == ChangeGlobalOptionXmlRpcMethod::getMethodName()) {
    return SharedHandle<XmlRpcMethod>(new ChangeGlobalOptionXmlRpcMethod());
  } else if(methodName == PurgeDownloadResultXmlRpcMethod::getMethodName()) {
    return SharedHandle<XmlRpcMethod>(new PurgeDownloadResultXmlRpcMethod());
  } else if(methodName == GetVersionXmlRpcMethod::getMethodName()) {
    return SharedHandle<XmlRpcMethod>(new GetVersionXmlRpcMethod());
  } else if(methodName == GetSessionInfoXmlRpcMethod::getMethodName()) {
    return SharedHandle<XmlRpcMethod>(new GetSessionInfoXmlRpcMethod());
  } else if(methodName == SystemMulticallXmlRpcMethod::getMethodName()) {
    return SharedHandle<XmlRpcMethod>(new SystemMulticallXmlRpcMethod());
  } else {
    return SharedHandle<XmlRpcMethod>(new NoSuchMethodXmlRpcMethod());
  }
}

} // namespace xmlrpc

} // namespace aria2
