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
#ifndef D_RPC_METHOD_IMPL_H
#define D_RPC_METHOD_IMPL_H

#include "RpcMethod.h"

#include <cassert>
#include <deque>
#include <algorithm>

#include "RpcRequest.h"
#include "ValueBase.h"
#include "TorrentAttribute.h"
#include "DlAbortEx.h"
#include "fmt.h"
#include "IndexedList.h"
#include "GroupId.h"
#include "RequestGroupMan.h"

namespace aria2 {

struct DownloadResult;
class RequestGroup;

namespace rpc {

template<typename T>
const T* checkParam(const RpcRequest& req, size_t index, bool required = false)
{
  const T* p = 0;
  if(req.params->size() > index) {
    if((p = downcast<T>(req.params->get(index))) == 0) {
      throw DL_ABORT_EX(fmt("The parameter at %lu has wrong type.",
                            static_cast<unsigned long>(index)));
    }
  } else if(required) {
    throw DL_ABORT_EX(fmt("The parameter at %lu is required but missing.",
                          static_cast<unsigned long>(index)));
  }
  return p;
}

template<typename T>
const T* checkRequiredParam(const RpcRequest& req, size_t index)
{
  return checkParam<T>(req, index, true);
}

struct IntegerGE {
  IntegerGE(int32_t min):min(min) {}

  bool operator()(const Integer* param, std::string* error) const
  {
    if(min <= param->i()) {
      return true;
    } else {
      if(error) {
        *error = fmt("the value must be greater than or equal to %d.", min);
      }
      return false;
    }
  }

  int32_t min;
};

template<typename Validator>
const Integer* checkRequiredInteger(const RpcRequest& req, size_t index,
                                    Validator validator)
{
  const Integer* param = checkRequiredParam<Integer>(req, index);
  std::string error;
  if(!validator(param, &error)) {
    throw DL_ABORT_EX(fmt("The integer parameter at %lu has invalid value: %s",
                          static_cast<unsigned long>(index), error.c_str()));
  }
  return param;
}

template<typename OutputIterator>
void toStringList(OutputIterator out, const List* src)
{
  if(!src) {
    return;
  }
  for(List::ValueType::const_iterator i = src->begin(), eoi = src->end();
      i != eoi; ++i) {
    const String* s = downcast<String>(*i);
    if(s) {
      *out++ = s->s();
    }
  }
}

class AddUriRpcMethod:public RpcMethod {
protected:
  virtual SharedHandle<ValueBase> process
  (const RpcRequest& req, DownloadEngine* e);
public:
  static const char* getMethodName()
  {
    return "aria2.addUri";
  }
};

class RemoveRpcMethod:public RpcMethod {
protected:
  virtual SharedHandle<ValueBase> process
  (const RpcRequest& req, DownloadEngine* e);
public:
  static const char* getMethodName()
  {
    return "aria2.remove";
  }
};

class ForceRemoveRpcMethod:public RpcMethod {
protected:
  virtual SharedHandle<ValueBase> process
  (const RpcRequest& req, DownloadEngine* e);
public:
  static const char* getMethodName()
  {
    return "aria2.forceRemove";
  }
};

class PauseRpcMethod:public RpcMethod {
protected:
  virtual SharedHandle<ValueBase> process
  (const RpcRequest& req, DownloadEngine* e);
public:
  static const char* getMethodName()
  {
    return "aria2.pause";
  }
};

class ForcePauseRpcMethod:public RpcMethod {
protected:
  virtual SharedHandle<ValueBase> process
  (const RpcRequest& req, DownloadEngine* e);
public:
  static const char* getMethodName()
  {
    return "aria2.forcePause";
  }
};

class PauseAllRpcMethod:public RpcMethod {
protected:
  virtual SharedHandle<ValueBase> process
  (const RpcRequest& req, DownloadEngine* e);
public:
  static const char* getMethodName()
  {
    return "aria2.pauseAll";
  }
};

class ForcePauseAllRpcMethod:public RpcMethod {
protected:
  virtual SharedHandle<ValueBase> process
  (const RpcRequest& req, DownloadEngine* e);
public:
  static const char* getMethodName()
  {
    return "aria2.forcePauseAll";
  }
};

class UnpauseRpcMethod:public RpcMethod {
protected:
  virtual SharedHandle<ValueBase> process
  (const RpcRequest& req, DownloadEngine* e);
public:
  static const char* getMethodName()
  {
    return "aria2.unpause";
  }
};

class UnpauseAllRpcMethod:public RpcMethod {
protected:
  virtual SharedHandle<ValueBase> process
  (const RpcRequest& req, DownloadEngine* e);
public:
  static const char* getMethodName()
  {
    return "aria2.unpauseAll";
  }
};

#ifdef ENABLE_BITTORRENT
class AddTorrentRpcMethod:public RpcMethod {
protected:
  virtual SharedHandle<ValueBase> process
  (const RpcRequest& req, DownloadEngine* e);
public:
  static const char* getMethodName()
  {
    return "aria2.addTorrent";
  }
};
#endif // ENABLE_BITTORRENT

#ifdef ENABLE_METALINK
class AddMetalinkRpcMethod:public RpcMethod {
protected:
  virtual SharedHandle<ValueBase> process
  (const RpcRequest& req, DownloadEngine* e);
public:
  static const char* getMethodName()
  {
    return "aria2.addMetalink";
  }
};
#endif // ENABLE_METALINK

class PurgeDownloadResultRpcMethod:public RpcMethod {
protected:
  virtual SharedHandle<ValueBase> process
  (const RpcRequest& req, DownloadEngine* e);
public:
  static const char* getMethodName()
  {
    return "aria2.purgeDownloadResult";
  }
};

class RemoveDownloadResultRpcMethod:public RpcMethod {
protected:
  virtual SharedHandle<ValueBase> process
  (const RpcRequest& req, DownloadEngine* e);
public:
  static const char* getMethodName()
  {
    return "aria2.removeDownloadResult";
  }
};

class GetUrisRpcMethod:public RpcMethod {
protected:
  virtual SharedHandle<ValueBase> process
  (const RpcRequest& req, DownloadEngine* e);
public:
  static const char* getMethodName()
  {
    return "aria2.getUris";
  }
};

class GetFilesRpcMethod:public RpcMethod {
protected:
  virtual SharedHandle<ValueBase> process
  (const RpcRequest& req, DownloadEngine* e);
public:
  static const char* getMethodName()
  {
    return "aria2.getFiles";
  }
};

#ifdef ENABLE_BITTORRENT
class GetPeersRpcMethod:public RpcMethod {
protected:
  virtual SharedHandle<ValueBase> process
  (const RpcRequest& req, DownloadEngine* e);
public:
  static const char* getMethodName()
  {
    return "aria2.getPeers";
  }
};
#endif // ENABLE_BITTORRENT

class GetServersRpcMethod:public RpcMethod {
protected:
  virtual SharedHandle<ValueBase> process
  (const RpcRequest& req, DownloadEngine* e);
public:
  static const char* getMethodName()
  {
    return "aria2.getServers";
  }
};

class TellStatusRpcMethod:public RpcMethod {
protected:
  virtual SharedHandle<ValueBase> process
  (const RpcRequest& req, DownloadEngine* e);
public:
  static const char* getMethodName()
  {
    return "aria2.tellStatus";
  }
};

class TellActiveRpcMethod:public RpcMethod {
protected:
  virtual SharedHandle<ValueBase> process
  (const RpcRequest& req, DownloadEngine* e);
public:
  static const char* getMethodName()
  {
    return "aria2.tellActive";
  }
};

template<typename T>
class AbstractPaginationRpcMethod:public RpcMethod {
private:
  template<typename InputIterator>
  std::pair<InputIterator, InputIterator>
  getPaginationRange
  (int64_t offset, int64_t num, InputIterator first, InputIterator last)
  {
    int64_t size = std::distance(first, last);
    if(offset < 0) {
      int64_t tempoffset = offset+size;
      if(tempoffset < 0) {
        return std::make_pair(last, last);
      }
      offset = tempoffset-(num-1);
      if(offset < 0) {
        offset = 0;
        num = tempoffset+1;
      }
    } else if(size <= offset) {
      return std::make_pair(last, last);
    }
    int64_t lastDistance;
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
protected:
  typedef IndexedList<a2_gid_t, SharedHandle<T> > ItemListType;

  virtual SharedHandle<ValueBase> process
  (const RpcRequest& req, DownloadEngine* e)
  {
    const Integer* offsetParam = checkRequiredParam<Integer>(req, 0);
    const Integer* numParam = checkRequiredInteger(req, 1, IntegerGE(0));
    const List* keysParam = checkParam<List>(req, 2);

    int64_t offset = offsetParam->i();
    int64_t num = numParam->i();
    std::vector<std::string> keys;
    toStringList(std::back_inserter(keys), keysParam);
    const ItemListType& items = getItems(e);
    std::pair<typename ItemListType::const_iterator,
              typename ItemListType::const_iterator> range =
      getPaginationRange(offset, num, items.begin(), items.end());
    SharedHandle<List> list = List::g();
    for(; range.first != range.second; ++range.first) {
      SharedHandle<Dict> entryDict = Dict::g();
      createEntry(entryDict, *range.first, e, keys);
      list->append(entryDict);
    }
    if(offset < 0) {
      std::reverse(list->begin(), list->end());
    }
    return list;
  }

  virtual const ItemListType& getItems(DownloadEngine* e) const = 0;

  virtual void createEntry
  (const SharedHandle<Dict>& entryDict,
   const SharedHandle<T>& item,
   DownloadEngine* e,
   const std::vector<std::string>& keys) const = 0;
};

class TellWaitingRpcMethod:
    public AbstractPaginationRpcMethod<RequestGroup> {
protected:
  virtual const RequestGroupList& getItems(DownloadEngine* e) const;

  virtual void createEntry
  (const SharedHandle<Dict>& entryDict,
   const SharedHandle<RequestGroup>& item,
   DownloadEngine* e,
   const std::vector<std::string>& keys) const;
public:
  static const char* getMethodName()
  {
    return "aria2.tellWaiting";
  }
};

class TellStoppedRpcMethod:
    public AbstractPaginationRpcMethod<DownloadResult> {
protected:
  virtual const DownloadResultList& getItems(DownloadEngine* e) const;

  virtual void createEntry
  (const SharedHandle<Dict>& entryDict,
   const SharedHandle<DownloadResult>& item,
   DownloadEngine* e,
   const std::vector<std::string>& keys) const;
public:
  static const char* getMethodName()
  {
    return "aria2.tellStopped";
  }
};

class ChangeOptionRpcMethod:public RpcMethod {
protected:
  virtual SharedHandle<ValueBase> process
  (const RpcRequest& req, DownloadEngine* e);
public:
  static const char* getMethodName()
  {
    return "aria2.changeOption";
  }
};

class ChangeGlobalOptionRpcMethod:public RpcMethod {
protected:
  virtual SharedHandle<ValueBase> process
  (const RpcRequest& req, DownloadEngine* e);
public:
  static const char* getMethodName()
  {
    return "aria2.changeGlobalOption";
  }
};

class GetVersionRpcMethod:public RpcMethod {
protected:
  virtual SharedHandle<ValueBase> process
  (const RpcRequest& req, DownloadEngine* e);
public:
  static const char* getMethodName()
  {
    return "aria2.getVersion";
  }
};

class GetOptionRpcMethod:public RpcMethod {
protected:
  virtual SharedHandle<ValueBase> process
  (const RpcRequest& req, DownloadEngine* e);
public:
  static const char* getMethodName()
  {
    return "aria2.getOption";
  }
};

class GetGlobalOptionRpcMethod:public RpcMethod {
protected:
  virtual SharedHandle<ValueBase> process
  (const RpcRequest& req, DownloadEngine* e);
public:
  static const char* getMethodName()
  {
    return "aria2.getGlobalOption";
  }
};

class ChangePositionRpcMethod:public RpcMethod {
protected:
  virtual SharedHandle<ValueBase> process
  (const RpcRequest& req, DownloadEngine* e);
public:
  static const char* getMethodName()
  {
    return "aria2.changePosition";
  }
};

class ChangeUriRpcMethod:public RpcMethod {
protected:
  virtual SharedHandle<ValueBase> process
  (const RpcRequest& req, DownloadEngine* e);
public:
  static const char* getMethodName()
  {
    return "aria2.changeUri";
  }
};

class GetSessionInfoRpcMethod:public RpcMethod {
protected:
  virtual SharedHandle<ValueBase> process
  (const RpcRequest& req, DownloadEngine* e);
public:
  static const char* getMethodName()
  {
    return "aria2.getSessionInfo";
  }
};

class ShutdownRpcMethod:public RpcMethod {
protected:
  virtual SharedHandle<ValueBase> process
  (const RpcRequest& req, DownloadEngine* e);
public:
  static const char* getMethodName()
  {
    return "aria2.shutdown";
  }
};

class GetGlobalStatRpcMethod:public RpcMethod {
protected:
  virtual SharedHandle<ValueBase> process
  (const RpcRequest& req, DownloadEngine* e);
public:
  static const char* getMethodName()
  {
    return "aria2.getGlobalStat";
  }
};

class ForceShutdownRpcMethod:public RpcMethod {
protected:
  virtual SharedHandle<ValueBase> process
  (const RpcRequest& req, DownloadEngine* e);
public:
  static const char* getMethodName()
  {
    return "aria2.forceShutdown";
  }
};

class SystemMulticallRpcMethod:public RpcMethod {
protected:
  virtual SharedHandle<ValueBase> process
  (const RpcRequest& req, DownloadEngine* e);
public:
  static const char* getMethodName()
  {
    return "system.multicall";
  }
};

class NoSuchMethodRpcMethod:public RpcMethod {
protected:
  virtual SharedHandle<ValueBase> process
  (const RpcRequest& req, DownloadEngine* e);
};

// Helper function to store data to entryDict from ds. This function
// is used by tellStatus method.
void gatherStoppedDownload
(const SharedHandle<Dict>& entryDict, const SharedHandle<DownloadResult>& ds,
 const std::vector<std::string>& keys);

// Helper function to store data to entryDict from group. This
// function is used by tellStatus/tellActive/tellWaiting method
void gatherProgressCommon
(const SharedHandle<Dict>& entryDict, const SharedHandle<RequestGroup>& group,
 const std::vector<std::string>& keys);

#ifdef ENABLE_BITTORRENT
// Helper function to store BitTorrent metadata from torrentAttrs.
void gatherBitTorrentMetadata
(const SharedHandle<Dict>& btDict,
 const SharedHandle<TorrentAttribute>& torrentAttrs);
#endif // ENABLE_BITTORRENT

} // namespace rpc

bool pauseRequestGroup
(const SharedHandle<RequestGroup>& group, bool reserved,  bool forcePause);

void changeOption
(const SharedHandle<RequestGroup>& group,
 const Option& option,
 DownloadEngine* e);

void changeGlobalOption(const Option& option, DownloadEngine* e);

} // namespace aria2

#endif // D_RPC_METHOD_IMPL_H
