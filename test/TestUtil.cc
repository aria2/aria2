#include "TestUtil.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cerrno>
#include <cstring>
#include <sstream>
#include <fstream>

#include "a2io.h"
#include "File.h"
#include "FatalException.h"
#include "Cookie.h"
#include "DefaultDiskWriter.h"
#include "fmt.h"
#include "util.h"
#ifdef ENABLE_MESSAGE_DIGEST
# include "message_digest_helper.h"
#endif // ENABLE_MESSAGE_DIGEST

namespace aria2 {

void createFile(const std::string& path, size_t length)
{
  File(File(path).getDirname()).mkdirs();
  int fd = creat(path.c_str(), OPEN_MODE);
  if(fd == -1) {
    throw FATAL_EXCEPTION(fmt("Could not create file=%s. cause:%s",
                              path.c_str(),
                              strerror(errno)));
  }
  if(-1 == ftruncate(fd, length)) {
    throw FATAL_EXCEPTION(fmt("ftruncate failed. cause:%s", strerror(errno)));
  }
  close(fd);
}

std::string readFile(const std::string& path)
{
  std::stringstream ss;
  std::ifstream in(path.c_str(), std::ios::binary);
  char buf[4096];
  while(1) {
    in.read(buf, sizeof(buf));
    ss.write(buf, in.gcount());
    if(in.gcount() != sizeof(buf)) {
      break;
    }
  }
  return ss.str();
}

Cookie createCookie
(const std::string& name,
 const std::string& value,
 const std::string& domain,
 bool hostOnly,
 const std::string& path,
 bool secure)
{
  return Cookie
    (name, value, 0, false, domain, hostOnly, path, secure, false, 0);
}

Cookie createCookie
(const std::string& name,
 const std::string& value,
 time_t expiryTime,
 const std::string& domain,
 bool hostOnly,
 const std::string& path,
 bool secure)
{
  return Cookie
    (name, value, expiryTime, true, domain, hostOnly, path, secure, false, 0);
}

std::string fromHex(const std::string& s)
{
  return util::fromHex(s.begin(), s.end());
}

#ifdef ENABLE_MESSAGE_DIGEST
std::string fileHexDigest
(const SharedHandle<MessageDigest>& ctx, const std::string& filename)
{
  SharedHandle<DiskWriter> writer(new DefaultDiskWriter(filename));
  writer->openExistingFile();
  return util::toHex(message_digest::digest(ctx, writer, 0, writer->size()));
}
#endif // ENABLE_MESSAGE_DIGEST

} // namespace aria2
