#ifndef D_EXTENSION_MESSAGE_TEST_HELPER_H
#define D_EXTENSION_MESSAGE_TEST_HELPER_H

#include "MockBtMessage.h"
#include "MockBtMessageFactory.h"

namespace aria2 {

typedef WrapBtMessage<ExtensionMessage> WrapExtBtMessage;

class WrapExtBtMessageFactory:public MockBtMessageFactory {
public:
  virtual SharedHandle<BtMessage>
  createBtExtendedMessage(const SharedHandle<ExtensionMessage>& extmsg)
  {
    return SharedHandle<BtMessage>(new WrapExtBtMessage(extmsg));
  }
};

} // namespace aria2

#endif // D_EXTENSION_MESSAGE_TEST_HELPER_H
