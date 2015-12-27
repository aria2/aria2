#ifndef D_MOCK_EXTENSION_MESSAGE_H
#define D_MOCK_EXTENSION_MESSAGE_H

#include "ExtensionMessage.h"

namespace aria2 {

struct MockExtensionMessageEventCheck {
  MockExtensionMessageEventCheck() : doReceivedActionCalled{false} {}
  bool doReceivedActionCalled;
};

class MockExtensionMessage : public ExtensionMessage {
public:
  std::string extensionName_;
  uint8_t extensionMessageID_;
  std::string data_;
  MockExtensionMessageEventCheck* evcheck_;

  MockExtensionMessage(const std::string& extensionName,
                       uint8_t extensionMessageID, const unsigned char* data,
                       size_t length, MockExtensionMessageEventCheck* evcheck)
      : extensionName_{extensionName},
        extensionMessageID_{extensionMessageID},
        data_{&data[0], &data[length]},
        evcheck_{evcheck}
  {
  }

  MockExtensionMessage(const std::string& extensionName,
                       uint8_t extensionMessageID, const std::string& data,
                       MockExtensionMessageEventCheck* evcheck)
      : extensionName_{extensionName},
        extensionMessageID_{extensionMessageID},
        data_{data},
        evcheck_{evcheck}
  {
  }

  virtual std::string getPayload() CXX11_OVERRIDE { return data_; }

  virtual uint8_t getExtensionMessageID() const CXX11_OVERRIDE
  {
    return extensionMessageID_;
  }

  virtual const char* getExtensionName() const CXX11_OVERRIDE
  {
    return extensionName_.c_str();
  }

  virtual std::string toString() const CXX11_OVERRIDE { return extensionName_; }

  virtual void doReceivedAction() CXX11_OVERRIDE
  {
    if (evcheck_) {
      evcheck_->doReceivedActionCalled = true;
    }
  }
};

} // namespace aria2

#endif // D_MOCK_EXTENSION_MESSAGE_H
