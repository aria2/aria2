#ifndef _D_MOCK_BT_CONTEXT_H_
#define _D_MOCK_BT_CONTEXT_H_

#include "BtContext.h"
#include "Util.h"

class MockBtContext : public BtContext  {
private:
  unsigned char infoHash[20];
  Strings pieceHashes;
  long long int totalLength;
  FILE_MODE fileMode;
  string name;
  int pieceLength;
  int numPieces;
  FileEntries fileEntries;
  AnnounceTiers announceTiers;
public:
  MockBtContext() {}
  virtual ~MockBtContext() {}

  virtual const unsigned char* getInfoHash() const {
    return infoHash;
  }

  void setInfoHash(const unsigned char* infoHash) {
    memcpy(this->infoHash, infoHash, sizeof(this->infoHash));
  }

  virtual int getInfoHashLength() const {
    return sizeof(infoHash);
  }

  virtual string getInfoHashAsString() const {
    return Util::toHex(infoHash, sizeof(infoHash));
  }

  virtual string getPieceHash(int index) const {
    return pieceHashes.at(index);
  }
  
  void addPieceHash(const string pieceHash) {
    pieceHashes.push_back(pieceHash);
  }

  virtual long long int getTotalLength() const {
    return totalLength;
  }

  void setTotalLength(long long int length) {
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
  
  virtual int getPieceLength() const {
    return pieceLength;
  }

  void setPieceLength(int pieceLength) {
    this->pieceLength = pieceLength;
  }

  virtual int getNumPieces() const {
    return numPieces;
  }

  void setNumPieces(int numPieces) {
    this->numPieces = numPieces;
  }
};

#endif // _D_MOCK_BT_CONTEXT_H_
