#ifndef _D_MOCK_EXTENSION_MESSAGE_H_
#define _D_MOCK_EXTENSION_MESSAGE_H_

#include "ExtensionMessage.h"

namespace aria2 {

class MockExtensionMessage:public ExtensionMessage {
public:
  std::string _extensionName;
  uint8_t _extensionMessageID;
  std::string _data;
  bool _doReceivedActionCalled;
public:
  MockExtensionMessage(const std::string& extensionName,
		       uint8_t extensionMessageID,
		       const char* data,
		       size_t length):_extensionName(extensionName),
				      _extensionMessageID(extensionMessageID),
				      _data(&data[0], &data[length]),
				      _doReceivedActionCalled(false) {}

  virtual ~MockExtensionMessage() {}

  virtual std::string getBencodedData()
  {
    return _data;
  }

  virtual uint8_t getExtensionMessageID()
  {
    return _extensionMessageID;
  }
  
  virtual const std::string& getExtensionName() const
  {
    return _extensionName;
  }

  virtual std::string toString() const
  {
    return _extensionName;
  }

  virtual void doReceivedAction()
  {
    _doReceivedActionCalled = true;
  }
};

} // namespace aria2

#endif // _D_MOCK_EXTENSION_MESSAGE_H_
