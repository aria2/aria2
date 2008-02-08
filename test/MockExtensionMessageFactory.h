#ifndef _D_MOCK_EXTENSION_MESSAGE_FACTORY_H_
#define _D_MOCK_EXTENSION_MESSAGE_FACTORY_H_

#include "ExtensionMessageFactory.h"
#include "MockExtensionMessage.h"

namespace aria2 {

class MockExtensionMessageFactory:public ExtensionMessageFactory {
public:
  virtual ~MockExtensionMessageFactory() {}

  virtual SharedHandle<ExtensionMessage> createMessage(const char* data,
						       size_t length)
  {
    return new MockExtensionMessage("a2_mock", *data, data+1, length-1);
				    
  }
};

} // namespace aria2

#endif // _D_MOCK_EXTENSION_MESSAGE_FACTORY_H_
