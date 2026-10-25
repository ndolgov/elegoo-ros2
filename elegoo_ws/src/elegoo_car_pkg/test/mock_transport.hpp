#pragma once
#include "elegoo_car_pkg/transport.hpp"
#include <string>
#include <vector>
#include <queue>

namespace elegoo_car_pkg::test {

using elegoo_car_pkg::ITransport; 

class MockTransport : public ITransport {
public:
  MockTransport() : connected_(false) {}
  ~MockTransport() override = default;

  bool connect() override {
    connected_ = true;
    return true;
  }

  void disconnect() override {
    connected_ = false;
  }

  bool is_connected() const override {
    return connected_;
  }

  bool send(const std::string& message) override {
    if (!connected_) return false;
    sent_messages.push_back(message);
    return true;
  }

  bool receive(std::string& response, int timeout_ms) override {
    if (!connected_) return false;
    if (preset_responses.empty()) return false;
    response = preset_responses.front();
    preset_responses.pop();
    return true;
  }

  // Test helper methods
  void set_connected(bool conn) { connected_ = conn; }
  void queue_response(const std::string& resp) { preset_responses.push(resp); }

  bool connected_;
  std::vector<std::string> sent_messages;
  std::queue<std::string> preset_responses;
};

} // namespace elegoo_car_pkg::test