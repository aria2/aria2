#ifndef D_MOCK_EXTENSION_MESSAGE_H
#define D_MOCK_EXTENSION_MESSAGE_H

#include "ExtensionMessage.h"

namespace aria2 {

class MockExtensionMessage:public ExtensionMessage {
public:
  std::string extensionName_;
  uint8_t extensionMessageID_;
  std::string data_;
  bool doReceivedActionCalled_;
public:
  MockExtensionMessage(const std::string& extensionName,
                       uint8_t extensionMessageID,
                       const unsigned char* data,
                       size_t length):extensionName_(extensionName),
                                      extensionMessageID_(extensionMessageID),
                                      data_(&data[0], &data[length]),
                                      doReceivedActionCalled_(false) {}

  MockExtensionMessage(const std::string& extensionName,
                       uint8_t extensionMessageID,
                       const std::string& data):
    extensionName_(extensionName),
    extensionMessageID_(extensionMessageID),
    data_(data),
    doReceivedActionCalled_(false) {}

  virtual ~MockExtensionMessage() {}

  virtual std::string getPayload()
  {
    return data_;
  }

  virtual uint8_t getExtensionMessageID()
  {
    return extensionMessageID_;
  }
  
  virtual const std::string& getExtensionName() const
  {
    return extensionName_;
  }

  virtual std::string toString() const
  {
    return extensionName_;
  }

  virtual void doReceivedAction()
  {
    doReceivedActionCalled_ = true;
  }
};

} // namespace aria2

#endif // D_MOCK_EXTENSION_MESSAGE_H
