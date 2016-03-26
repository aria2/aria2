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

  virtual std::string getFilename() CXX11_OVERRIDE { return filename; }

  void setFilename(const std::string& filename) { this->filename = filename; }

  virtual bool exists() CXX11_OVERRIDE { return false; }

  virtual void save() CXX11_OVERRIDE {}

  virtual void load() CXX11_OVERRIDE {}

  virtual void removeFile() CXX11_OVERRIDE {}

  virtual void updateFilename() CXX11_OVERRIDE {}
};

} // namespace aria2

#endif // D_MOCK_BT_PROGRESS_INFO_FILE_H
