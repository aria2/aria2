#include "TestUtil.h"
#include "a2io.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

namespace aria2 {

void createFile(const std::string& path, size_t length)
{
  int fd = creat(path.c_str(), OPEN_MODE);
  ftruncate(fd, length);
}

};
