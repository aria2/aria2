#ifndef D_EXTENSION_MESSAGE_TEST_HELPER_H
#define D_EXTENSION_MESSAGE_TEST_HELPER_H

#include "MockBtMessageFactory.h"

namespace aria2 {

class WrapExtBtMessageFactory : public MockBtMessageFactory {
public:
  virtual std::unique_ptr<BtExtendedMessage> createBtExtendedMessage(
      std::unique_ptr<ExtensionMessage> extmsg) CXX11_OVERRIDE
  {
    return make_unique<BtExtendedMessage>(std::move(extmsg));
  }
};

} // namespace aria2

#endif // D_EXTENSION_MESSAGE_TEST_HELPER_H
