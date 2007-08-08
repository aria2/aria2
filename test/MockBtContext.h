#ifndef _D_MOCK_BT_CONTEXT_H_
#define _D_MOCK_BT_CONTEXT_H_

#include "BtContext.h"
#include "Util.h"

class MockBtContext : public BtContext  {
private:
  unsigned char infoHash[20];
  Strings pieceHashes;
  int64_t totalLength;
  FILE_MODE fileMode;
  string name;
  int32_t pieceLength;
  int32_t numPieces;
  unsigned char peerId[20];
  FileEntries fileEntries;
  AnnounceTiers announceTiers;
  Integers fastSet;
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

  virtual int32_t getInfoHashLength() const {
    return sizeof(infoHash);
  }

  virtual string getInfoHashAsString() const {
    return Util::toHex(infoHash, sizeof(infoHash));
  }

  virtual string getPieceHash(int32_t index) const {
    return pieceHashes.at(index);
  }
  
  virtual const Strings& getPieceHashes() const {
    return pieceHashes;
  }

  void addPieceHash(const string pieceHash) {
    pieceHashes.push_back(pieceHash);
  }

  virtual int64_t getTotalLength() const {
    return totalLength;
  }

  void setTotalLength(int64_t length) {
    this->totalLength = length;
  }

  virtual FILE_MODE getFileMode() const {
    return fileMode;
  }

  void setFileMode(FILE_MODE fileMode) {
    this->fileMode = fileMode;
  }

  virtual FileEntries getFileEntries() const {
    return fileEntries;
  }

  void addFileEntry(const FileEntryHandle& fileEntry) {
    fileEntries.push_back(fileEntry);
  }

  virtual AnnounceTiers getAnnounceTiers() const {
    return announceTiers;
  }

  void addAnnounceTier(const AnnounceTierHandle& announceTier) {
    announceTiers.push_back(announceTier);
  }

  virtual void load(const string& torrentFile) {}

  virtual string getName() const {
    return name;
  }

  void setName(const string& name) {
    this->name = name;
  }
  
  virtual int32_t getPieceLength() const {
    return pieceLength;
  }

  void setPieceLength(int32_t pieceLength) {
    this->pieceLength = pieceLength;
  }

  virtual int32_t getNumPieces() const {
    return numPieces;
  }

  void setNumPieces(int32_t numPieces) {
    this->numPieces = numPieces;
  }

  virtual const unsigned char* getPeerId() {
    return peerId;
  }

  void setPeerId(const unsigned char* peerId) {
    memcpy(this->peerId, peerId, sizeof(this->peerId));
  }

  virtual Integers computeFastSet(const string& ipaddr, int32_t fastSetSize)
  {
    return fastSet;
  }

  void setFastSet(const Integers& fastSet)
  {
    this->fastSet = fastSet;
  }

};

typedef SharedHandle<MockBtContext> MockBtContextHandle;

#endif // _D_MOCK_BT_CONTEXT_H_
