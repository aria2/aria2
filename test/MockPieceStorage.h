#ifndef _D_MOCK_PIECE_STORAGE_H_
#define _D_MOCK_PIECE_STORAGE_H_

#include "PieceStorage.h"
#include "BitfieldMan.h"
#include "Piece.h"
#include "DiskAdaptor.h"

class MockPieceStorage : public PieceStorage {
private:
  int64_t totalLength;
  int64_t filteredTotalLength;
  int64_t completedLength;
  int64_t filteredCompletedLength;
  BitfieldMan* bitfieldMan;
  bool selectiveDownloadingMode;
  bool endGame;
  DiskAdaptorHandle diskAdaptor;
  Integers pieceLengthList;
  Pieces inFlightPieces;
  bool _allDownloadFinished;
public:
  MockPieceStorage():totalLength(0),
		     filteredTotalLength(0),
		     completedLength(0),
		     filteredCompletedLength(0),
		     bitfieldMan(0),
		     selectiveDownloadingMode(false),
		     endGame(false),
		     diskAdaptor(0),
		     _allDownloadFinished(false) {}

  virtual ~MockPieceStorage() {}

  virtual bool hasMissingPiece(const PeerHandle& peer) {
    return false;
  }

  virtual PieceHandle getMissingPiece(const PeerHandle& peer) {
    return new Piece();
  }

  virtual PieceHandle getMissingFastPiece(const PeerHandle& peer) {
    return new Piece();
  }

  virtual PieceHandle getMissingPiece()
  {
    return new Piece();
  }

  virtual PieceHandle getMissingPiece(int32_t index)
  {
    return new Piece();
  }

  virtual bool isPieceUsed(int32_t index)
  {
    return false;
  }

  virtual void markPieceMissing(int32_t index) {}

  virtual void markPiecesDone(int64_t) {}

  virtual PieceHandle getPiece(int32_t index) {
    return new Piece();
  }

  virtual void completePiece(const PieceHandle& piece) {}

  virtual void cancelPiece(const PieceHandle& piece) {}

  virtual bool hasPiece(int32_t index) {
    return false;
  }

  virtual int64_t getTotalLength() {
    return totalLength;
  }

  void setTotalLength(int64_t totalLength) {
    this->totalLength = totalLength;
  }

  virtual int64_t getFilteredTotalLength() {
    return filteredTotalLength;
  }

  void setFilteredTotalLength(int64_t totalLength) {
    this->filteredTotalLength = totalLength;
  }

  virtual int64_t getCompletedLength() {
    return completedLength;
  }

  void setCompletedLength(int64_t completedLength) {
    this->completedLength = completedLength;
  }

  virtual int64_t getFilteredCompletedLength() {
    return filteredCompletedLength;
  }

  void setFilteredCompletedLength(int64_t completedLength) {
    this->filteredCompletedLength = completedLength;
  }
  
  virtual void setFileFilter(const Strings& filePaths) {}

  virtual void setFileFilter(const Integers& fileIndexes) {}

  virtual void clearFileFilter() {}

  virtual bool downloadFinished() {
    return false;
  }

  virtual bool allDownloadFinished() {
    return _allDownloadFinished;
  }

  void setAllDownloadFinished(bool f)
  {
    _allDownloadFinished = f;
  }

  virtual void initStorage() {}

  virtual const unsigned char* getBitfield() {
    return bitfieldMan->getBitfield();
  }

  virtual void setBitfield(const unsigned char* bitfield,
			   int32_t bitfieldLength) {
    bitfieldMan->setBitfield(bitfield, bitfieldLength);
  }
  
  virtual int32_t getBitfieldLength() {
    return bitfieldMan->getBitfieldLength();
  }

  void setBitfield(BitfieldMan* bitfieldMan) {
    this->bitfieldMan = bitfieldMan;
  }

  virtual bool isSelectiveDownloadingMode() {
    return selectiveDownloadingMode;
  }

  void setSelectiveDownloadingMode(bool flag) {
    this->selectiveDownloadingMode = flag;
  }

  virtual void finishSelectiveDownloadingMode() {}

  virtual bool isEndGame() {
    return endGame;
  }

  void setEndGame(bool flag) {
    this->endGame = flag;
  }

  virtual DiskAdaptorHandle getDiskAdaptor() {
    return diskAdaptor;
  }

  void setDiskAdaptor(const DiskAdaptorHandle adaptor) {
    this->diskAdaptor = adaptor;
  }
  
  virtual int32_t getPieceLength(int32_t index) {
    return pieceLengthList.at(index);
  }

  void addPieceLengthList(int32_t length) {
    pieceLengthList.push_back(length);
  }

  virtual void advertisePiece(int32_t cuid, int32_t index) {}

  virtual Integers getAdvertisedPieceIndexes(int32_t myCuid,
					     const Time& lastCheckTime) {
    return Integers();
  }

  virtual void removeAdvertisedPiece(int32_t elapsed) {}

  virtual void markAllPiecesDone() {}

  virtual void addInFlightPiece(const Pieces& pieces)
  {
    copy(pieces.begin(), pieces.end(), back_inserter(inFlightPieces));
  }

  virtual int32_t countInFlightPiece()
  {
    return inFlightPieces.size();
  }

  virtual Pieces getInFlightPieces()
  {
    return inFlightPieces;
  }

};

typedef SharedHandle<MockPieceStorage> MockPieceStorageHandle;

#endif // _D_MOCK_PIECE_STORAGE_H_
