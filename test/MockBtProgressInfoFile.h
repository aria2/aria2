#ifndef D_MOCK_BT_PROGRESS_INFO_FILE_H
#define D_MOCK_BT_PROGRESS_INFO_FILE_H

#include "BtProgressInfoFile.h"

namespace aria2 {

class MockBtProgressInfoFile : public BtProgressInfoFile {
private:
  std::string filename;
public:
  MockBtProgressInfoFile() {}
  virtual ~MockBtProgressInfoFile() {}

  virtual std::string getFilename() {
    return filename;
  }

  virtual void setFilename(const std::string& filename) {
    this->filename = filename;
  }

  virtual bool exists() {
    return false;
  }

  virtual void save() {}

  virtual void load() {}

  virtual void removeFile() {}

  virtual void updateFilename() {}
};

} // namespace aria2

#endif // D_MOCK_BT_PROGRESS_INFO_FILE_H
