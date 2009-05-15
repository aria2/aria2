#include "common.h"
#include <string>

namespace aria2 {

void createFile(const std::string& filename, size_t length);

std::string readFile(const std::string& path);

} // namespace aria2
