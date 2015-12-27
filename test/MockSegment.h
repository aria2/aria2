#ifndef D_MOCK_SEGMENT_H
#define D_MOCK_SEGMENT_H

#include "Segment.h"
#include "Piece.h"
#include "A2STR.h"

namespace aria2 {

class MockSegment : public Segment {
public:
  virtual bool complete() const CXX11_OVERRIDE { return false; }

  virtual size_t getIndex() const CXX11_OVERRIDE { return 0; }

  virtual int64_t getPosition() const CXX11_OVERRIDE { return 0; }

  virtual int64_t getPositionToWrite() const CXX11_OVERRIDE { return 0; }

  virtual int64_t getLength() const CXX11_OVERRIDE { return 0; }

  virtual int64_t getSegmentLength() const CXX11_OVERRIDE { return 0; }

  virtual int64_t getWrittenLength() const CXX11_OVERRIDE { return 0; }

  virtual void updateWrittenLength(int64_t bytes) CXX11_OVERRIDE {}

  // `begin' is a offset inside this segment.
  virtual bool updateHash(int64_t begin, const unsigned char* data,
                          size_t dataLength) CXX11_OVERRIDE
  {
    return false;
  }

  virtual bool isHashCalculated() const CXX11_OVERRIDE { return false; }

  virtual std::string getDigest() CXX11_OVERRIDE { return A2STR::NIL; }

  virtual void clear(WrDiskCache* diskCache) CXX11_OVERRIDE {}

  virtual std::shared_ptr<Piece> getPiece() const CXX11_OVERRIDE
  {
    return std::shared_ptr<Piece>(new Piece());
  }
};

} // namespace aria2

#endif // D_MOCK_SEGMENT_H
