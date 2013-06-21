#ifndef D_EXTENSION_MESSAGE_TEST_HELPER_H
#define D_EXTENSION_MESSAGE_TEST_HELPER_H

#include "MockBtMessage.h"
#include "MockBtMessageFactory.h"

namespace aria2 {

typedef WrapBtMessage<ExtensionMessage> WrapExtBtMessage;

class WrapExtBtMessageFactory:public MockBtMessageFactory {
public:
  virtual std::shared_ptr<BtMessage>
  createBtExtendedMessage(const std::shared_ptr<ExtensionMessage>& extmsg)
  {
    return std::shared_ptr<BtMessage>(new WrapExtBtMessage(extmsg));
  }
};

} // namespace aria2

#endif // D_EXTENSION_MESSAGE_TEST_HELPER_H
