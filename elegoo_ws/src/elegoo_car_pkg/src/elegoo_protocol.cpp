#include "elegoo_car_pkg/elegoo_protocol.hpp"

namespace elegoo_car_pkg {

std::string ElegooProtocol::create_motion_command(int direction, int speed) {
  nlohmann::json j;
  j["H"] = 1;
  j["N"] = 3;
  j["D1"] = direction; // 1: Left, 2: Right, 3: Forward, 4: Backward
  j["D2"] = speed;     // 0 - 255
  return j.dump();
}

std::string ElegooProtocol::create_stop_command() {
  nlohmann::json j;
  j["H"] = 1;
  j["N"] = 1;
  j["D1"] = 0;
  j["D2"] = 0;
  j["D3"] = 1;
  return j.dump();
}

std::string ElegooProtocol::create_sonar_query() {
  nlohmann::json j;
  j["H"] = 1;
  j["N"] = 21;
  j["D1"] = 0;
  return j.dump();
}

bool ElegooProtocol::parse_sonar_response(const std::string& response, float& distance_cm) {
  try {
    auto j = nlohmann::json::parse(response);
    for (auto it = j.begin(); it != j.end(); ++it) {
      if (it.value().is_number()) {
        distance_cm = it.value().get<float>();
        return true;
      }
    }
  } catch (...) {
    try {
      distance_cm = std::stof(response);
      return true;
    } catch (...) {
      return false;
    }
  }
  return false;
}

} // namespace elegoo_car_pkg
