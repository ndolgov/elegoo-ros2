#include "elegoo_car_pkg/tcp_transport.hpp"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <poll.h>
#include <cstring>

namespace elegoo_car_pkg {

TcpTransport::TcpTransport(std::string ip, int port)
: ip_(std::move(ip)), port_(port), sock_fd_(-1), connected_(false) {}

TcpTransport::~TcpTransport() { disconnect(); }

bool TcpTransport::connect() {
  if (connected_) return true;
  sock_fd_ = socket(AF_INET, SOCK_STREAM, 0);
  if (sock_fd_ < 0) return false;

  sockaddr_in serv_addr{};
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(port_);

  if (inet_pton(AF_INET, ip_.c_str(), &serv_addr.sin_addr) <= 0) {
    close(sock_fd_);
    sock_fd_ = -1;
    return false;
  }

  if (::connect(sock_fd_, reinterpret_cast<struct sockaddr*>(&serv_addr), sizeof(serv_addr)) < 0) {
    close(sock_fd_);
    sock_fd_ = -1;
    return false;
  }

  connected_ = true;
  return true;
}

void TcpTransport::disconnect() {
  if (sock_fd_ >= 0) {
    close(sock_fd_);
    sock_fd_ = -1;
  }
  connected_ = false;
}

bool TcpTransport::is_connected() const { return connected_; }

bool TcpTransport::send(const std::string& message) {
  if (!connected_ || sock_fd_ < 0) return false;
  std::string payload = message + "\r\n";
  if (write(sock_fd_, payload.c_str(), payload.size()) < 0) {
    connected_ = false;
    return false;
  }
  return true;
}

bool TcpTransport::receive(std::string& response, int timeout_ms) {
  if (!connected_ || sock_fd_ < 0) return false;
  struct pollfd pfd{sock_fd_, POLLIN, 0};
  if (poll(&pfd, 1, timeout_ms) <= 0 || !(pfd.revents & POLLIN)) return false;

  char buffer[256];
  std::memset(buffer, 0, sizeof(buffer));
  ssize_t bytes_read = read(sock_fd_, buffer, sizeof(buffer) - 1);
  if (bytes_read <= 0) {
    connected_ = false;
    return false;
  }
  response = std::string(buffer, bytes_read);
  return true;
}

} // namespace elegoo_car_pkg
