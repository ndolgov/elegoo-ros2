#pragma once
#include <string>
#include <nlohmann/json.hpp>

namespace elegoo_car_pkg {
class ElegooProtocol {
public:
  static std::string create_motion_command(int direction, int speed); // N=3
  static std::string create_stop_command();                          // N=1
  static std::string create_sonar_query();                           // N=21
  static bool parse_sonar_response(const std::string& response, float& distance_cm);
};
} // namespace elegoo_car_pkg
