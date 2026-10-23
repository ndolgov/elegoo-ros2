#pragma once
#include "elegoo_car_pkg/transport.hpp"
#include <string>
#include <atomic>

namespace elegoo_car_pkg {
class TcpTransport : public ITransport {
public:
  TcpTransport(std::string ip, int port);
  ~TcpTransport() override;

  bool connect() override;
  void disconnect() override;
  bool is_connected() const override;
  bool send(const std::string& message) override;
  bool receive(std::string& response, int timeout_ms = 1000) override;

private:
  std::string ip_;
  int port_;
  int sock_fd_;
  std::atomic<bool> connected_;
};
} // namespace elegoo_car_pkg
