#ifndef D_MOCK_SEGMENT_H
#define D_MOCK_SEGMENT_H

#include "Segment.h"
#include "Piece.h"
#include "A2STR.h"

namespace aria2 {

class MockSegment:public Segment {
public:
  virtual bool complete() const
  {
    return false;
  }

  virtual size_t getIndex() const
  {
    return 0;
  }

  virtual off_t getPosition() const
  {
    return 0;
  }
  
  virtual off_t getPositionToWrite() const
  {
    return 0;
  }

  virtual size_t getLength() const
  {
    return 0;
  }

  virtual size_t getSegmentLength() const
  {
    return 0;
  }

  virtual size_t getWrittenLength() const
  {
    return 0;
  }

  virtual void updateWrittenLength(size_t bytes) {}

#ifdef ENABLE_MESSAGE_DIGEST

  // `begin' is a offset inside this segment.
  virtual bool updateHash
  (uint32_t begin, const unsigned char* data, size_t dataLength)
  {
    return false;
  }

  virtual bool isHashCalculated() const
  {
    return false;
  }

  virtual std::string getDigest()
  {
    return A2STR::NIL;
  }

#endif // ENABLE_MESSAGE_DIGEST

  virtual void clear() {}

  virtual SharedHandle<Piece> getPiece() const
  {
    return SharedHandle<Piece>();
  }
};

} // namespace aria2

#endif // D_MOCK_SEGMENT_H
