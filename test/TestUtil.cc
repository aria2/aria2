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
#include "RequestGroupMan.h"
#include "RequestGroup.h"
#include "DownloadContext.h"
#include "Option.h"
#include "FileEntry.h"
#include "DownloadResult.h"
#include "message_digest_helper.h"

namespace aria2 {

void createFile(const std::string& path, size_t length)
{
  File(File(path).getDirname()).mkdirs();
  DefaultDiskWriter dw(path);
  dw.initAndOpenFile();
  dw.truncate(length);
}

std::string readFile(const std::string& path)
{
  std::stringstream ss;
  std::ifstream in(path.c_str(), std::ios::binary);
  char buf[4_k];
  while (1) {
    in.read(buf, sizeof(buf));
    ss.write(buf, in.gcount());
    if (in.gcount() != sizeof(buf)) {
      break;
    }
  }
  return ss.str();
}

std::unique_ptr<Cookie> createCookie(const std::string& name,
                                     const std::string& value,
                                     const std::string& domain, bool hostOnly,
                                     const std::string& path, bool secure)
{
  return make_unique<Cookie>(name, value, 0, false, domain, hostOnly, path,
                             secure, false, 0);
}

std::unique_ptr<Cookie> createCookie(const std::string& name,
                                     const std::string& value,
                                     time_t expiryTime,
                                     const std::string& domain, bool hostOnly,
                                     const std::string& path, bool secure)
{
  return make_unique<Cookie>(name, value, expiryTime, true, domain, hostOnly,
                             path, secure, false, 0);
}

std::string fromHex(const std::string& s)
{
  return util::fromHex(s.begin(), s.end());
}

std::string fileHexDigest(MessageDigest* ctx, const std::string& filename)
{
  std::shared_ptr<DiskWriter> writer(new DefaultDiskWriter(filename));
  writer->openExistingFile();
  return util::toHex(message_digest::digest(ctx, writer, 0, writer->size()));
}

WrDiskCacheEntry::DataCell* createDataCell(int64_t goff, const char* data,
                                           size_t offset)
{
  WrDiskCacheEntry::DataCell* cell = new WrDiskCacheEntry::DataCell();
  cell->goff = goff;
  size_t len = strlen(data);
  cell->data = new unsigned char[len];
  memcpy(cell->data, data, len);
  cell->offset = offset;
  cell->len = cell->capacity = len - offset;
  return cell;
}

std::shared_ptr<RequestGroup> findReservedGroup(RequestGroupMan* rgman,
                                                a2_gid_t gid)
{
  auto rg = rgman->findGroup(gid);
  if (rg) {
    if (rg->getState() == RequestGroup::STATE_WAITING) {
      return rg;
    }
    else {
      rg.reset();
    }
  }
  return rg;
}

std::shared_ptr<RequestGroup> getReservedGroup(RequestGroupMan* rgman,
                                               size_t index)
{
  assert(rgman->getReservedGroups().size() > index);
  auto i = rgman->getReservedGroups().begin();
  std::advance(i, index);
  return *i;
}

std::shared_ptr<RequestGroup>
createRequestGroup(int32_t pieceLength, int64_t totalLength,
                   const std::string& path, const std::string& uri,
                   const std::shared_ptr<Option>& opt)
{
  std::shared_ptr<DownloadContext> dctx(
      new DownloadContext(pieceLength, totalLength, path));
  std::vector<std::string> uris;
  uris.push_back(uri);
  dctx->getFirstFileEntry()->addUris(uris.begin(), uris.end());
  std::shared_ptr<RequestGroup> group(new RequestGroup(GroupId::create(), opt));
  group->setDownloadContext(dctx);
  return group;
}

std::shared_ptr<DownloadResult> createDownloadResult(error_code::Value result,
                                                     const std::string& uri)
{
  std::vector<std::string> uris;
  uris.push_back(uri);
  std::shared_ptr<FileEntry> entry(new FileEntry("/tmp/path", 1, 0, uris));
  std::vector<std::shared_ptr<FileEntry>> entries;
  entries.push_back(entry);
  std::shared_ptr<DownloadResult> dr(new DownloadResult());
  dr->gid = GroupId::create();
  dr->fileEntries = entries;
  dr->result = result;
  dr->belongsTo = 0;
  dr->inMemoryDownload = false;
  dr->option = std::shared_ptr<Option>(new Option());
  return dr;
}

} // namespace aria2
