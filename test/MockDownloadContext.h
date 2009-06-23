#ifndef _D_MOCK_DOWNLOAD_CONTEXT_H_
#define _D_MOCK_DOWNLOAD_CONTEXT_H_

#include "DownloadContext.h"
#include "A2STR.h"

namespace aria2 {

class MockDownloadContext:public DownloadContext
{
private:
  std::deque<std::string> _pieceHashes;
public:
  virtual const std::string& getPieceHash(size_t index) const
  {
    return A2STR::NIL;
  }
  
  virtual const std::deque<std::string>& getPieceHashes() const
  {
    return _pieceHashes;
  }

  virtual uint64_t getTotalLength() const
  {
    return 0;
  }

  virtual bool knowsTotalLength() const
  {
    return false;
  }

  virtual FILE_MODE getFileMode() const
  {
    return MULTI;
  }

  virtual size_t getPieceLength() const
  {
    return 0;
  }

  virtual size_t getNumPieces() const
  {
    return 0;
  }

  virtual const std::string& getPieceHashAlgo() const
  {
    return A2STR::NIL;
  }

  virtual std::string getActualBasePath() const
  {
    return A2STR::NIL;
  }

  void addFileEntry(const SharedHandle<FileEntry>& fileEntry)
  {
    _fileEntries.push_back(fileEntry);
  }
};

} // namespace aria2

#endif // _D_MOCK_DOWNLOAD_CONTEXT_H_
