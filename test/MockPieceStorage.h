#ifndef D_MOCK_PIECE_STORAGE_H
#define D_MOCK_PIECE_STORAGE_H

#include "PieceStorage.h"

#include <algorithm>

#include "BitfieldMan.h"
#include "Piece.h"
#include "DiskAdaptor.h"

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
  bool downloadFinished_;
  bool allDownloadFinished_;
public:
  MockPieceStorage():totalLength(0),
                     filteredTotalLength(0),
                     completedLength(0),
                     filteredCompletedLength(0),
                     bitfieldMan(0),
                     selectiveDownloadingMode(false),
                     endGame(false),
                     downloadFinished_(false),
                     allDownloadFinished_(false) {}

  virtual ~MockPieceStorage() {}

#ifdef ENABLE_BITTORRENT

  virtual bool hasMissingPiece(const SharedHandle<Peer>& peer) {
    return false;
  }

  virtual void getMissingPiece
  (std::vector<SharedHandle<Piece> >& pieces,
   size_t minMissingBlocks,
   const SharedHandle<Peer>& peer,
   cuid_t cuid)
  {}

  virtual void getMissingPiece
  (std::vector<SharedHandle<Piece> >& pieces,
   size_t minMissingBlocks,
   const SharedHandle<Peer>& peer,
   const std::vector<size_t>& excludedIndexes,
   cuid_t cuid)
  {}

  virtual void getMissingFastPiece
  (std::vector<SharedHandle<Piece> >& pieces,
   size_t minMissingBlocks,
   const SharedHandle<Peer>& peer,
   cuid_t cuid)
  {}

  virtual void getMissingFastPiece
  (std::vector<SharedHandle<Piece> >& pieces,
   size_t minMissingBlocks,
   const SharedHandle<Peer>& peer,
   const std::vector<size_t>& excludedIndexes,
   cuid_t cuid)
  {}

  virtual SharedHandle<Piece> getMissingPiece
  (const SharedHandle<Peer>& peer,
   cuid_t cuid)
  {
    return SharedHandle<Piece>(new Piece());
  }

  virtual SharedHandle<Piece> getMissingPiece
  (const SharedHandle<Peer>& peer,
   const std::vector<size_t>& excludedIndexes,
   cuid_t cuid)
  {
    return SharedHandle<Piece>(new Piece());
  }

#endif // ENABLE_BITTORRENT

  virtual bool hasMissingUnusedPiece()
  {
    return false;
  }

  virtual SharedHandle<Piece> getMissingPiece
  (size_t minSplitSize,
   const unsigned char* ignoreBitfield,
   size_t length,
   cuid_t cuid)
  {
    return SharedHandle<Piece>(new Piece());
  }

  virtual SharedHandle<Piece> getMissingPiece(size_t index, cuid_t cuid)
  {
    return SharedHandle<Piece>(new Piece());
  }

  virtual bool isPieceUsed(size_t index)
  {
    return false;
  }

  virtual void markPieceMissing(size_t index) {}

  virtual void markPiecesDone(int64_t) {}

  virtual SharedHandle<Piece> getPiece(size_t index) {
    return SharedHandle<Piece>(new Piece());
  }

  virtual void completePiece(const SharedHandle<Piece>& piece) {}

  virtual void cancelPiece(const SharedHandle<Piece>& piece, cuid_t cuid) {}

  virtual bool hasPiece(size_t index) {
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
  
  virtual void setupFileFilter() {}

  virtual void clearFileFilter() {}

  virtual bool downloadFinished() {
    return downloadFinished_;
  }

  void setDownloadFinished(bool f)
  {
    downloadFinished_ = f;
  }

  virtual bool allDownloadFinished() {
    return allDownloadFinished_;
  }

  void setAllDownloadFinished(bool f)
  {
    allDownloadFinished_ = f;
  }

  virtual void initStorage() {}

  virtual const unsigned char* getBitfield() {
    return bitfieldMan->getBitfield();
  }

  virtual void setBitfield(const unsigned char* bitfield,
                           size_t bitfieldLength) {
    bitfieldMan->setBitfield(bitfield, bitfieldLength);
  }
  
  virtual size_t getBitfieldLength() {
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

  virtual bool isEndGame() {
    return endGame;
  }

  virtual void setEndGamePieceNum(size_t num) {}

  virtual void enterEndGame() {
    this->endGame = true;
  }

  virtual SharedHandle<DiskAdaptor> getDiskAdaptor() {
    return diskAdaptor;
  }

  void setDiskAdaptor(const SharedHandle<DiskAdaptor>& adaptor) {
    this->diskAdaptor = adaptor;
  }
  
  virtual int32_t getPieceLength(size_t index) {
    return pieceLengthList.at(index);
  }

  void addPieceLengthList(int32_t length) {
    pieceLengthList.push_back(length);
  }

  virtual void advertisePiece(cuid_t cuid, size_t index) {}

  virtual void getAdvertisedPieceIndexes(std::vector<size_t>& indexes,
                                         cuid_t myCuid,
                                         const Timer& lastCheckTime)
  {}

  virtual void removeAdvertisedPiece(time_t elapsed) {}

  virtual void markAllPiecesDone() {}

  virtual void addInFlightPiece(const std::vector<SharedHandle<Piece> >& pieces)
  {
    std::copy(pieces.begin(), pieces.end(), back_inserter(inFlightPieces));
  }

  virtual size_t countInFlightPiece()
  {
    return inFlightPieces.size();
  }

  virtual void getInFlightPieces(std::vector<SharedHandle<Piece> >& pieces)
  {
    pieces.insert(pieces.end(), inFlightPieces.begin(), inFlightPieces.end());
  }

  virtual void addPieceStats(size_t index) {}

  virtual void addPieceStats(const unsigned char* bitfield,
                             size_t bitfieldLength) {}

  virtual void subtractPieceStats(const unsigned char* bitfield,
                                  size_t bitfieldLength) {}

  virtual void updatePieceStats(const unsigned char* newBitfield,
                                size_t newBitfieldLength,
                                const unsigned char* oldBitfield) {}

  virtual size_t getNextUsedIndex(size_t index)
  {
    return 0;
  }

  virtual void onDownloadIncomplete() {}
};

} // namespace aria2

#endif // D_MOCK_PIECE_STORAGE_H
