#include "elegoo_car_pkg/elegoo_car_node.hpp"
#include "rclcpp/rclcpp.hpp"

int main(int argc, char* argv[]) {
  rclcpp::init(argc, argv);
  auto node = std::make_shared<elegoo_car_pkg::ElegooCarNode>();
  rclcpp::spin(node);
  rclcpp::shutdown();
  return 0;
}
