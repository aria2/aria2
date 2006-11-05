#ifndef _D_MOCK_BT_PROGRESS_INFO_FILE_H_
#define _D_MOCK_BT_PROGRESS_INFO_FILE_H_

#include "BtProgressInfoFile.h"

class MockBtProgressInfoFile : public BtProgressInfoFile {
private:
  string filename;
public:
  MockBtProgressInfoFile() {}
  virtual ~MockBtProgressInfoFile() {}

  virtual string getFilename() {
    return filename;
  }

  virtual void setFilename(const string& filename) {
    this->filename = filename;
  }

  virtual bool exists() {
    return false;
  }

  virtual void save() {}

  virtual void load() {}

  virtual void removeFile() {}
};

#endif // _D_MOCK_BT_PROGRESS_INFO_FILE_H_
