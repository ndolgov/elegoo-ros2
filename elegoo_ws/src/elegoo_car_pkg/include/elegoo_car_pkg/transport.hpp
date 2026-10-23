#pragma once
#include <string>

namespace elegoo_car_pkg {
class ITransport {
public:
  virtual ~ITransport() = default;
  virtual bool connect() = 0;
  virtual void disconnect() = 0;
  virtual bool is_connected() const = 0;
  virtual bool send(const std::string& message) = 0;
  virtual bool receive(std::string& response, int timeout_ms = 1000) = 0;
};
} // namespace elegoo_car_pkg
