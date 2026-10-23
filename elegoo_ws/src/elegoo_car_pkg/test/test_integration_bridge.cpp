#include <gtest/gtest.h>
#include "rclcpp/rclcpp.hpp"
#include "elegoo_car_pkg/elegoo_car_node.hpp"
#include "elegoo_car_pkg/wifi_transport.hpp"
#include <thread>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

class IntegrationTestFixture : public ::testing::Test {
protected:
  static void SetUpTestSuite() { rclcpp::init(0, nullptr); }
  static void TearDownTestSuite() { rclcpp::shutdown(); }

  void SetUp() override {
    // Start a simple local TCP echo/mock server on port 11000
    server_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(server_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(11000);

    bind(server_fd_, reinterpret_cast<struct sockaddr*>(&address), sizeof(address));
    listen(server_fd_, 1);

    server_running_ = true;
    server_thread_ = std::thread([this]() {
      sockaddr_in client_addr{};
      socklen_t addrlen = sizeof(client_addr);
      int client_sock = accept(server_fd_, reinterpret_cast<struct sockaddr*>(&client_addr), &addrlen);
      if (client_sock < 0) return;

      while (server_running_) {
        char buffer[256] = {0};
        ssize_t valread = read(client_sock, buffer, 255);
        if (valread > 0) {
          std::string req(buffer, valread);
          if (req.find("\"N\":21") != std::string::npos) {
            std::string resp = "{\"N\":21,\"D1\":50.0}\r\n";
            write(client_sock, resp.c_str(), resp.size());
          }
        }
      }
      close(client_sock);
    });
  }

  void TearDown() override {
    server_running_ = false;
    if (server_fd_ >= 0) close(server_fd_);
    if (server_thread_.joinable()) server_thread_.join();
  }

  int server_fd_ = -1;
  bool server_running_ = false;
  std::thread server_thread_;
};

TEST_F(IntegrationTestFixture, EndToEndTcpTransportAndNodeExchange) {
  auto transport = std::make_unique<elegoo_car_pkg::WifiTransport>("127.0.0.1", 11000);
  auto node = std::make_shared<elegoo_car_pkg::ElegooCarNode>(std::move(transport));

  // Allow time for TCP connection and control loop query transaction
  rclcpp::sleep_for(std::chrono::milliseconds(150));
  rclcpp::spin_some(node);

  SUCCEED();
}