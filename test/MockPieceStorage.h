#ifndef _D_MOCK_PIECE_STORAGE_H_
#define _D_MOCK_PIECE_STORAGE_H_

#include "PieceStorage.h"
#include "BitfieldMan.h"
#include "Piece.h"
#include "DiskAdaptor.h"
#include <algorithm>

namespace aria2 {

class MockPieceStorage : public PieceStorage {
private:
  int64_t totalLength;
  int64_t filteredTotalLength;
  int64_t completedLength;
  int64_t filteredCompletedLength;
  BitfieldMan* bitfieldMan;
  bool selectiveDownloadingMode;
  bool endGame;
  SharedHandle<DiskAdaptor> diskAdaptor;
  std::deque<int32_t> pieceLengthList;
  std::deque<SharedHandle<Piece> > inFlightPieces;
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

  virtual bool hasMissingPiece(const SharedHandle<Peer>& peer) {
    return false;
  }

  virtual SharedHandle<Piece> getMissingPiece(const SharedHandle<Peer>& peer) {
    return new Piece();
  }

  virtual SharedHandle<Piece> getMissingFastPiece(const SharedHandle<Peer>& peer) {
    return new Piece();
  }

  virtual SharedHandle<Piece> getMissingPiece()
  {
    return new Piece();
  }

  virtual SharedHandle<Piece> getMissingPiece(int32_t index)
  {
    return new Piece();
  }

  virtual bool isPieceUsed(int32_t index)
  {
    return false;
  }

  virtual void markPieceMissing(int32_t index) {}

  virtual void markPiecesDone(int64_t) {}

  virtual SharedHandle<Piece> getPiece(int32_t index) {
    return new Piece();
  }

  virtual void completePiece(const SharedHandle<Piece>& piece) {}

  virtual void cancelPiece(const SharedHandle<Piece>& piece) {}

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
  
  virtual void setFileFilter(const std::deque<std::string>& filePaths) {}

  virtual void setFileFilter(IntSequence seq) {}

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

  virtual SharedHandle<DiskAdaptor> getDiskAdaptor() {
    return diskAdaptor;
  }

  void setDiskAdaptor(const SharedHandle<DiskAdaptor> adaptor) {
    this->diskAdaptor = adaptor;
  }
  
  virtual int32_t getPieceLength(int32_t index) {
    return pieceLengthList.at(index);
  }

  void addPieceLengthList(int32_t length) {
    pieceLengthList.push_back(length);
  }

  virtual void advertisePiece(int32_t cuid, int32_t index) {}

  virtual std::deque<int32_t> getAdvertisedPieceIndexes(int32_t myCuid,
							const Time& lastCheckTime) {
    return std::deque<int32_t>();
  }

  virtual void removeAdvertisedPiece(int32_t elapsed) {}

  virtual void markAllPiecesDone() {}

  virtual void addInFlightPiece(const std::deque<SharedHandle<Piece> >& pieces)
  {
    std::copy(pieces.begin(), pieces.end(), back_inserter(inFlightPieces));
  }

  virtual int32_t countInFlightPiece()
  {
    return inFlightPieces.size();
  }

  virtual std::deque<SharedHandle<Piece> > getInFlightPieces()
  {
    return inFlightPieces;
  }

};

} // namespace aria2

#endif // _D_MOCK_PIECE_STORAGE_H_
