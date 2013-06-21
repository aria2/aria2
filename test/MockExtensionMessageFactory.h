#ifndef D_MOCK_EXTENSION_MESSAGE_FACTORY_H
#define D_MOCK_EXTENSION_MESSAGE_FACTORY_H

#include "ExtensionMessageFactory.h"
#include "MockExtensionMessage.h"

namespace aria2 {

class MockExtensionMessageFactory:public ExtensionMessageFactory {
public:
  virtual ~MockExtensionMessageFactory() {}

  virtual std::shared_ptr<ExtensionMessage> createMessage(const unsigned char* data,
                                                       size_t length)
  {
    return std::shared_ptr<ExtensionMessage>
      (new MockExtensionMessage("a2_mock", *data, data+1, length-1));
  }
};

} // namespace aria2

#endif // D_MOCK_EXTENSION_MESSAGE_FACTORY_H
