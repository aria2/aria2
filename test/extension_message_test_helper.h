#ifndef _D_EXTENSION_MESSAGE_TEST_HELPER_H_
#define _D_EXTENSION_MESSAGE_TEST_HELPER_H_

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

#endif // _D_EXTENSION_MESSAGE_TEST_HELPER_H_
