#ifndef D_EXTENSION_MESSAGE_TEST_HELPER_H
#define D_EXTENSION_MESSAGE_TEST_HELPER_H

#include "MockBtMessageFactory.h"

namespace aria2 {

class WrapExtBtMessageFactory:public MockBtMessageFactory {
public:
  virtual std::unique_ptr<BtExtendedMessage>
  createBtExtendedMessage(const std::shared_ptr<ExtensionMessage>& extmsg)
    override
  {
    return make_unique<BtExtendedMessage>(extmsg);
  }
};

} // namespace aria2

#endif // D_EXTENSION_MESSAGE_TEST_HELPER_H
