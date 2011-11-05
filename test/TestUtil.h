#include "common.h"

#include <string>

#include "SharedHandle.h"
#include "Cookie.h"

namespace aria2 {

class MessageDigest;

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

} // namespace aria2
