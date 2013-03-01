#include "common.h"

#include <string>

#include "SharedHandle.h"
#include "Cookie.h"
#include "WrDiskCacheEntry.h"
#include "GroupId.h"

namespace aria2 {

class MessageDigest;
class RequestGroupMan;
class RequestGroup;
class Option;
struct DownloadResult;

void createFile(const std::string& filename, size_t length);

std::string readFile(const std::string& path);

class CookieSorter {
public:
  bool operator()(const Cookie& lhs, const Cookie& rhs) const
  {
    if(lhs.getDomain() == rhs.getDomain()) {
      return lhs.getName() < rhs.getName();
    } else {
      return lhs.getDomain() < rhs.getDomain();
    }
  }
};

Cookie createCookie
(const std::string& name,
 const std::string& value,
 const std::string& domain,
 bool hostOnly,
 const std::string& path,
 bool secure);

Cookie createCookie
(const std::string& name,
 const std::string& value,
 time_t expiryTime,
 const std::string& domain,
 bool hostOnly,
 const std::string& path,
 bool secure);

std::string fromHex(const std::string& s);

#ifdef ENABLE_MESSAGE_DIGEST
// Returns hex digest of contents of file denoted by filename.
std::string fileHexDigest
(const SharedHandle<MessageDigest>& ctx, const std::string& filename);
#endif // ENABLE_MESSAGE_DIGEST

WrDiskCacheEntry::DataCell* createDataCell(int64_t goff,
                                           const char* data,
                                           size_t offset = 0);

SharedHandle<RequestGroup> findReservedGroup
(const SharedHandle<RequestGroupMan>& rgman, a2_gid_t gid);

SharedHandle<RequestGroup> getReservedGroup
(const SharedHandle<RequestGroupMan>& rgman, size_t index);

SharedHandle<RequestGroup> createRequestGroup(int32_t pieceLength,
                                              int64_t totalLength,
                                              const std::string& path,
                                              const std::string& uri,
                                              const SharedHandle<Option>& opt);

SharedHandle<DownloadResult> createDownloadResult
(error_code::Value result, const std::string& uri);

} // namespace aria2
