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
#ifndef _D_XML_RPC_METHOD_IMPL_H_
#define _D_XML_RPC_METHOD_IMPL_H_

#include "XmlRpcMethod.h"

#include <cassert>
#include <deque>
#include <algorithm>

#include "BDE.h"
#include "XmlRpcRequest.h"

namespace aria2 {

class DownloadResult;
class RequestGroup;

namespace xmlrpc {

class AddUriXmlRpcMethod:public XmlRpcMethod {
protected:
  virtual BDE process(const XmlRpcRequest& req, DownloadEngine* e);
public:
  static const std::string& getMethodName()
  {
    static std::string methodName = "aria2.addUri";
    return methodName;
  }
};

class RemoveXmlRpcMethod:public XmlRpcMethod {
protected:
  virtual BDE process(const XmlRpcRequest& req, DownloadEngine* e);
public:
  static const std::string& getMethodName()
  {
    static std::string methodName = "aria2.remove";
    return methodName;
  }
};

class ForceRemoveXmlRpcMethod:public XmlRpcMethod {
protected:
  virtual BDE process(const XmlRpcRequest& req, DownloadEngine* e);
public:
  static const std::string& getMethodName()
  {
    static std::string methodName = "aria2.forceRemove";
    return methodName;
  }
};

#ifdef ENABLE_BITTORRENT
class AddTorrentXmlRpcMethod:public XmlRpcMethod {
protected:
  virtual BDE process(const XmlRpcRequest& req, DownloadEngine* e);
public:
  static const std::string& getMethodName()
  {
    static std::string methodName = "aria2.addTorrent";
    return methodName;
  }
};
#endif // ENABLE_BITTORRENT

#ifdef ENABLE_METALINK
class AddMetalinkXmlRpcMethod:public XmlRpcMethod {
protected:
  virtual BDE process(const XmlRpcRequest& req, DownloadEngine* e);
public:
  static const std::string& getMethodName()
  {
    static std::string methodName = "aria2.addMetalink";
    return methodName;
  }
};
#endif // ENABLE_METALINK

class PurgeDownloadResultXmlRpcMethod:public XmlRpcMethod {
protected:
  virtual BDE process(const XmlRpcRequest& req, DownloadEngine* e);
public:
  static const std::string& getMethodName()
  {
    static std::string methodName = "aria2.purgeDownloadResult";
    return methodName;
  }
};

class GetUrisXmlRpcMethod:public XmlRpcMethod {
protected:
  virtual BDE process(const XmlRpcRequest& req, DownloadEngine* e);
public:
  static const std::string& getMethodName()
  {
    static std::string methodName = "aria2.getUris";
    return methodName;
  }
};

class GetFilesXmlRpcMethod:public XmlRpcMethod {
protected:
  virtual BDE process(const XmlRpcRequest& req, DownloadEngine* e);
public:
  static const std::string& getMethodName()
  {
    static std::string methodName = "aria2.getFiles";
    return methodName;
  }
};

#ifdef ENABLE_BITTORRENT
class GetPeersXmlRpcMethod:public XmlRpcMethod {
protected:
  virtual BDE process(const XmlRpcRequest& req, DownloadEngine* e);
public:
  static const std::string& getMethodName()
  {
    static std::string methodName = "aria2.getPeers";
    return methodName;
  }
};
#endif // ENABLE_BITTORRENT

class GetServersXmlRpcMethod:public XmlRpcMethod {
protected:
  virtual BDE process(const XmlRpcRequest& req, DownloadEngine* e);
public:
  static const std::string& getMethodName()
  {
    static std::string methodName = "aria2.getServers";
    return methodName;
  }
};

class TellStatusXmlRpcMethod:public XmlRpcMethod {
protected:
  virtual BDE process(const XmlRpcRequest& req, DownloadEngine* e);
public:
  static const std::string& getMethodName()
  {
    static std::string methodName = "aria2.tellStatus";
    return methodName;
  }
};

class TellActiveXmlRpcMethod:public XmlRpcMethod {
protected:
  virtual BDE process(const XmlRpcRequest& req, DownloadEngine* e);
public:
  static const std::string& getMethodName()
  {
    static std::string methodName = "aria2.tellActive";
    return methodName;
  }
};

template<typename T>
class AbstractPaginationXmlRpcMethod:public XmlRpcMethod {
private:
  template<typename InputIterator>
  std::pair<InputIterator, InputIterator>
  getPaginationRange
  (ssize_t offset, size_t num, InputIterator first, InputIterator last)
  {
    size_t size = std::distance(first, last);
    if(offset < 0) {
      ssize_t tempoffset = offset+size;
      if(tempoffset < 0) {
        return std::make_pair(last, last);
      }
      offset = tempoffset-(num-1);
      if(offset < 0) {
        offset = 0;
        num = tempoffset+1;
      }
    } else if(size <= (size_t)offset) {
      return std::make_pair(last, last);
    }
    BDE list = BDE::list();
    size_t lastDistance;
    if(size < offset+num) {
      lastDistance = size;
    } else {
      lastDistance = offset+num;
    }
    last = first;
    std::advance(first, offset);
    std::advance(last, lastDistance);
    return std::make_pair(first, last);
  }

  void checkPaginationParams(const BDE& params) const
  {
    assert(params.isList());
    if(params.size() != 2 ||
       !params[0].isInteger() || !params[1].isInteger() ||
       params[1].i() < 0) {
      throw DL_ABORT_EX("Invalid argument. Specify offset and num in integer.");
    }
  }
protected:
  virtual BDE process(const XmlRpcRequest& req, DownloadEngine* e)
  {
    const BDE& params = req._params;
    checkPaginationParams(params);
    ssize_t offset = params[0].i();
    size_t num = params[1].i();
    const std::deque<SharedHandle<T> >& items = getItems(e);
    std::pair<typename std::deque<SharedHandle<T> >::const_iterator,
      typename std::deque<SharedHandle<T> >::const_iterator> range =
      getPaginationRange(offset, num, items.begin(), items.end());
    BDE list = BDE::list();
    for(; range.first != range.second; ++range.first) {
      BDE entryDict = BDE::dict();
      createEntry(entryDict, *range.first, e);
      list << entryDict;
    }
    if(offset < 0) {
      std::reverse(list.listBegin(), list.listEnd());
    }
    return list;
  }

  virtual const std::deque<SharedHandle<T> >&
  getItems(DownloadEngine* e) const = 0;

  virtual void createEntry
  (BDE& entryDict, const SharedHandle<T>& item, DownloadEngine* e) const = 0;
};

class TellWaitingXmlRpcMethod:
    public AbstractPaginationXmlRpcMethod<RequestGroup> {
protected:
  virtual const std::deque<SharedHandle<RequestGroup> >&
  getItems(DownloadEngine* e) const;

  virtual void createEntry
  (BDE& entryDict, const SharedHandle<RequestGroup>& item,
   DownloadEngine* e) const;
public:
  static const std::string& getMethodName()
  {
    static std::string methodName = "aria2.tellWaiting";
    return methodName;
  }
};

class TellStoppedXmlRpcMethod:
    public AbstractPaginationXmlRpcMethod<DownloadResult> {
protected:
   virtual const std::deque<SharedHandle<DownloadResult> >&
   getItems(DownloadEngine* e) const;

  virtual void createEntry
  (BDE& entryDict, const SharedHandle<DownloadResult>& item,
   DownloadEngine* e) const;
public:
  static const std::string& getMethodName()
  {
    static std::string methodName = "aria2.tellStopped";
    return methodName;
  }
};

class ChangeOptionXmlRpcMethod:public XmlRpcMethod {
protected:
  virtual BDE process(const XmlRpcRequest& req, DownloadEngine* e);
public:
  static const std::string& getMethodName()
  {
    static std::string methodName = "aria2.changeOption";
    return methodName;
  }
};

class ChangeGlobalOptionXmlRpcMethod:public XmlRpcMethod {
protected:
  virtual BDE process(const XmlRpcRequest& req, DownloadEngine* e);
public:
  static const std::string& getMethodName()
  {
    static std::string methodName = "aria2.changeGlobalOption";
    return methodName;
  }
};

class GetVersionXmlRpcMethod:public XmlRpcMethod {
protected:
  virtual BDE process(const XmlRpcRequest& req, DownloadEngine* e);
public:
  static const std::string& getMethodName()
  {
    static std::string methodName = "aria2.getVersion";
    return methodName;
  }
};

class GetOptionXmlRpcMethod:public XmlRpcMethod {
protected:
  virtual BDE process(const XmlRpcRequest& req, DownloadEngine* e);
public:
  static const std::string& getMethodName()
  {
    static std::string methodName = "aria2.getOption";
    return methodName;
  }
};

class GetGlobalOptionXmlRpcMethod:public XmlRpcMethod {
protected:
  virtual BDE process(const XmlRpcRequest& req, DownloadEngine* e);
public:
  static const std::string& getMethodName()
  {
    static std::string methodName = "aria2.getGlobalOption";
    return methodName;
  }
};

class ChangePositionXmlRpcMethod:public XmlRpcMethod {
protected:
  virtual BDE process(const XmlRpcRequest& req, DownloadEngine* e);
public:
  static const std::string& getMethodName()
  {
    static std::string methodName = "aria2.changePosition";
    return methodName;
  }
};

class ChangeUriXmlRpcMethod:public XmlRpcMethod {
protected:
  virtual BDE process(const XmlRpcRequest& req, DownloadEngine* e);
public:
  static const std::string& getMethodName()
  {
    static std::string methodName = "aria2.changeUri";
    return methodName;
  }
};

class GetSessionInfoXmlRpcMethod:public XmlRpcMethod {
protected:
  virtual BDE process(const XmlRpcRequest& req, DownloadEngine* e);
public:
  static const std::string& getMethodName()
  {
    static std::string methodName = "aria2.getSessionInfo";
    return methodName;
  }
};

class ShutdownXmlRpcMethod:public XmlRpcMethod {
protected:
  virtual BDE process(const XmlRpcRequest& req, DownloadEngine* e);
public:
  static const std::string& getMethodName()
  {
    static std::string methodName = "aria2.shutdown";
    return methodName;
  }
};

class ForceShutdownXmlRpcMethod:public XmlRpcMethod {
protected:
  virtual BDE process(const XmlRpcRequest& req, DownloadEngine* e);
public:
  static const std::string& getMethodName()
  {
    static std::string methodName = "aria2.forceShutdown";
    return methodName;
  }
};

class SystemMulticallXmlRpcMethod:public XmlRpcMethod {
protected:
  virtual BDE process(const XmlRpcRequest& req, DownloadEngine* e);
public:
  static const std::string& getMethodName()
  {
    static std::string methodName = "system.multicall";
    return methodName;
  }
};

class NoSuchMethodXmlRpcMethod:public XmlRpcMethod {
protected:
  virtual BDE process(const XmlRpcRequest& req, DownloadEngine* e);
};

// Helper function to store data to entryDict from ds. This function
// is used by tellStatus method.
void gatherStoppedDownload
(BDE& entryDict, const SharedHandle<DownloadResult>& ds);

// Helper function to store data to entryDict from group. This
// function is used by tellStatus/tellActive/tellWaiting method
void gatherProgressCommon
(BDE& entryDict, const SharedHandle<RequestGroup>& group);

#ifdef ENABLE_BITTORRENT
// Helper function to store BitTorrent metadata from torrentAttrs in
// btDict. btDict must be an BDE::Dict.
void gatherBitTorrentMetadata(BDE& btDict, const BDE& torrentAttrs);
#endif // ENABLE_BITTORRENT

} // namespace xmlrpc

} // namespace aria2

#endif // _D_XML_RPC_METHOD_IMPL_H_
