#ifndef _D_MOCK_BT_CONTEXT_H_
#define _D_MOCK_BT_CONTEXT_H_

#include "BtContext.h"
#include "Util.h"
#include "AnnounceTier.h"
#include <cstring>

namespace aria2 {

class MockBtContext : public BtContext  {
private:
  unsigned char infoHash[20];
  std::deque<std::string> pieceHashes;
  uint64_t totalLength;
  FILE_MODE fileMode;
  std::string name;
  size_t pieceLength;
  size_t numPieces;
  unsigned char peerId[20];
  std::deque<SharedHandle<FileEntry> > fileEntries;
  std::deque<SharedHandle<AnnounceTier> > announceTiers;
  std::deque<std::pair<std::string, uint16_t> > _nodes;
  std::deque<size_t> fastSet;
public:
  MockBtContext():totalLength(0),
		  pieceLength(0),
		  numPieces(0) {}

  virtual ~MockBtContext() {}

  virtual const unsigned char* getInfoHash() const {
    return infoHash;
  }

  void setInfoHash(const unsigned char* infoHash) {
    memcpy(this->infoHash, infoHash, sizeof(this->infoHash));
  }

  virtual size_t getInfoHashLength() const {
    return sizeof(infoHash);
  }

  virtual std::string getInfoHashAsString() const {
    return Util::toHex(infoHash, sizeof(infoHash));
  }

  virtual std::string getPieceHash(size_t index) const {
    return pieceHashes.at(index);
  }
  
  virtual const std::deque<std::string>& getPieceHashes() const {
    return pieceHashes;
  }

  void addPieceHash(const std::string& pieceHash) {
    pieceHashes.push_back(pieceHash);
  }

  virtual uint64_t getTotalLength() const {
    return totalLength;
  }

  void setTotalLength(uint64_t length) {
    this->totalLength = length;
  }

  virtual FILE_MODE getFileMode() const {
    return fileMode;
  }

  void setFileMode(FILE_MODE fileMode) {
    this->fileMode = fileMode;
  }

  virtual std::deque<SharedHandle<FileEntry> > getFileEntries() const {
    return fileEntries;
  }

  void addFileEntry(const SharedHandle<FileEntry>& fileEntry) {
    fileEntries.push_back(fileEntry);
  }

  virtual std::deque<SharedHandle<AnnounceTier> > getAnnounceTiers() const {
    return announceTiers;
  }

  void addAnnounceTier(const SharedHandle<AnnounceTier>& announceTier) {
    announceTiers.push_back(announceTier);
  }

  virtual void load(const std::string& torrentFile) {}

  virtual std::string getName() const {
    return name;
  }

  void setName(const std::string& name) {
    this->name = name;
  }
  
  virtual size_t getPieceLength() const {
    return pieceLength;
  }

  void setPieceLength(size_t pieceLength) {
    this->pieceLength = pieceLength;
  }

  virtual size_t getNumPieces() const {
    return numPieces;
  }

  void setNumPieces(size_t numPieces) {
    this->numPieces = numPieces;
  }

  virtual const unsigned char* getPeerId() {
    return peerId;
  }

  void setPeerId(const unsigned char* peerId) {
    memcpy(this->peerId, peerId, sizeof(this->peerId));
  }

  virtual std::deque<size_t> computeFastSet(const std::string& ipaddr, size_t fastSetSize)
  {
    return fastSet;
  }

  void setFastSet(const std::deque<size_t>& fastSet)
  {
    this->fastSet = fastSet;
  }

  virtual std::string getPieceHashAlgo() const
  {
    return "sha1";
  }

  virtual std::string getActualBasePath() const
  {
    return _dir+"/"+name;
  }

  virtual RequestGroup* getOwnerRequestGroup()
  {
    return 0;
  }

  virtual std::deque<std::pair<std::string, uint16_t> >& getNodes()
  {
    return _nodes;
  }

  void setNodes(const std::deque<std::pair<std::string, uint16_t> >& nodes)
  {
    _nodes = nodes;
  }

};

} // namespace aria2

#endif // _D_MOCK_BT_CONTEXT_H_
