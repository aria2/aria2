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
#include "XmlRpcMethodFactory.h"
#include "XmlRpcMethodImpl.h"
#include "OptionParser.h"
#include "OptionHandler.h"

namespace aria2 {

namespace xmlrpc {

SharedHandle<XmlRpcMethod>
XmlRpcMethodFactory::create(const std::string& methodName)
{
  if(methodName == "aria2.addUri") {
    return SharedHandle<XmlRpcMethod>(new AddUriXmlRpcMethod());
  } else if(methodName == "aria2.addTorrent") {
    return SharedHandle<XmlRpcMethod>(new AddTorrentXmlRpcMethod());
  } else if(methodName == "aria2.addMetalink") {
    return SharedHandle<XmlRpcMethod>(new AddMetalinkXmlRpcMethod());
  } else if(methodName == "aria2.remove") {
    return SharedHandle<XmlRpcMethod>(new RemoveXmlRpcMethod());
  } else if(methodName == "aria2.tellStatus") {
    return SharedHandle<XmlRpcMethod>(new TellStatusXmlRpcMethod());
  } else if(methodName == "aria2.getUris") {
    return SharedHandle<XmlRpcMethod>(new GetUrisXmlRpcMethod());
  } else if(methodName == "aria2.getFiles") {
    return SharedHandle<XmlRpcMethod>(new GetFilesXmlRpcMethod());
  } else if(methodName == "aria2.getPeers") {
    return SharedHandle<XmlRpcMethod>(new GetPeersXmlRpcMethod());
  } else if(methodName == "aria2.tellActive") {
    return SharedHandle<XmlRpcMethod>(new TellActiveXmlRpcMethod());
  } else if(methodName == "aria2.changeOption") {
    return SharedHandle<XmlRpcMethod>(new ChangeOptionXmlRpcMethod());
  } else if(methodName == "aria2.changeGlobalOption") {
    return SharedHandle<XmlRpcMethod>(new ChangeGlobalOptionXmlRpcMethod());
  } else if(methodName == "aria2.purgeDownloadResult") {
    return SharedHandle<XmlRpcMethod>(new PurgeDownloadResultXmlRpcMethod());
  } else {
    return SharedHandle<XmlRpcMethod>(new NoSuchMethodXmlRpcMethod());
  }
}

} // namespace xmlrpc

} // namespace aria2
