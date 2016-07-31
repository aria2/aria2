#include <iostream>
#include <algorithm>

int main() {
  std::string filepath = "/ahue/hue/hue/kuk.jpg";
  auto fn = filepath;
  std::string ext;
  auto idx = fn.find_last_of(".");
  auto slash = fn.find_last_of("\\/");
  if (idx != std::string::npos && (slash == std::string::npos || slash < idx)) {
    ext = fn.substr(idx);
    fn = fn.substr(0, idx);
  }
  std::cout << "fn:" << fn << " ext:" << ext;
  return 0;
}
