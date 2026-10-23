#pragma once
#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/twist.hpp"
#include "std_msgs/msg/float32.hpp"
#include "elegoo_car_pkg/transport.hpp"
#include <memory>

namespace elegoo_car_pkg {
class ElegooCarNode : public rclcpp::Node {
public:
  ElegooCarNode();
  ~ElegooCarNode() override;

private:
  void cmd_vel_callback(const geometry_msgs::msg::Twist::SharedPtr msg);
  void control_loop();

  std::unique_ptr<ITransport> transport_;
  rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr cmd_vel_sub_;
  rclcpp::Publisher<std_msgs::msg::Float32>::SharedPtr sonar_pub_;
  rclcpp::TimerBase::SharedPtr control_timer_;

  std::string car_ip_;
  int car_port_;
  bool obstacle_avoidance_enabled_;
  float safety_distance_cm_;
};
} // namespace elegoo_car_pkg
