#include "elegoo_car_pkg/elegoo_car_node.hpp"
#include "elegoo_car_pkg/tcp_transport.hpp"
#include "elegoo_car_pkg/elegoo_protocol.hpp"
#include <algorithm>

namespace elegoo_car_pkg {

ElegooCarNode::ElegooCarNode()
: ElegooCarNode(std::make_unique<elegoo_car_pkg::TcpTransport>("192.168.4.1", 100)) 
{
}  

ElegooCarNode::ElegooCarNode(std::unique_ptr<elegoo_car_pkg::ITransport> transport)
: Node("elegoo_car_node"), car_ip_("192.168.4.1"), car_port_(100),
  obstacle_avoidance_enabled_(true), safety_distance_cm_(25.0f),
  transport_(std::move(transport)) {
  
  this->declare_parameter<std::string>("car_ip", "192.168.4.1");
  this->declare_parameter<int>("car_port", 100);
  this->declare_parameter<bool>("obstacle_avoidance_enabled", true);
  this->declare_parameter<float>("safety_distance_cm", 25.0f);

  car_ip_ = this->get_parameter("car_ip").as_string();
  car_port_ = this->get_parameter("car_port").as_int();
  obstacle_avoidance_enabled_ = this->get_parameter("obstacle_avoidance_enabled").as_bool();
  safety_distance_cm_ = this->get_parameter("safety_distance_cm").as_double();

  transport_ = std::make_unique<TcpTransport>(car_ip_, car_port_);
  if (transport_->connect()) {
    RCLCPP_INFO(this->get_logger(), "Connected to Elegoo Car at %s:%d", car_ip_.c_str(), car_port_);
  } else {
    RCLCPP_WARN(this->get_logger(), "Connection failed. Retrying in background control loop.");
  }

  cmd_vel_sub_ = this->create_subscription<geometry_msgs::msg::Twist>(
    "/cmd_vel", 10, std::bind(&ElegooCarNode::cmd_vel_callback, this, std::placeholders::_1));

  sonar_pub_ = this->create_publisher<std_msgs::msg::Float32>("/sonar_distance", 10);

  control_timer_ = this->create_wall_timer(
    std::chrono::milliseconds(100), std::bind(&ElegooCarNode::control_loop, this));
}

ElegooCarNode::~ElegooCarNode() {
  if (transport_ && transport_->is_connected()) {
    transport_->send(ElegooProtocol::create_stop_command());
    transport_->disconnect();
  }
}

void ElegooCarNode::cmd_vel_callback(const geometry_msgs::msg::Twist::SharedPtr msg) {
  if (!transport_->is_connected()) return;

  double linear = msg->linear.x;
  double angular = msg->angular.z;
  int speed = static_cast<int>(std::clamp(std::abs(linear) * 150.0, 0.0, 255.0));

  std::string cmd_str;
  if (std::abs(linear) < 0.01 && std::abs(angular) < 0.01) {
    cmd_str = ElegooProtocol::create_stop_command();
  } else if (linear >= 0.0) {
    if (angular > 0.5) cmd_str = ElegooProtocol::create_motion_command(1, speed);
    else if (angular < -0.5) cmd_str = ElegooProtocol::create_motion_command(2, speed);
    else cmd_str = ElegooProtocol::create_motion_command(3, speed);
  } else {
    cmd_str = ElegooProtocol::create_motion_command(4, speed);
  }
  transport_->send(cmd_str);
}

void ElegooCarNode::control_loop() {
  if (!transport_->is_connected()) {
    if (transport_->connect()) RCLCPP_INFO(this->get_logger(), "Reconnected to Elegoo Car!");
    else return;
  }

  // On-demand ultrasonic query
  if (transport_->send(ElegooProtocol::create_sonar_query())) {
    std::string response;
    if (transport_->receive(response, 40)) {
      float distance = 0.0f;
      if (ElegooProtocol::parse_sonar_response(response, distance)) {
        std_msgs::msg::Float32 sonar_msg;
        sonar_msg.data = distance;
        sonar_pub_->publish(sonar_msg);

        // ROS-side obstacle override
        if (obstacle_avoidance_enabled_ && distance > 0.0f && distance < safety_distance_cm_) {
          RCLCPP_WARN_THROTTLE(this->get_logger(), *this->get_clock(), 1000,
            "Obstacle at %.1f cm! Safety backup triggered.", distance);
          transport_->send(ElegooProtocol::create_motion_command(4, 120)); // Back up
        }
      }
    }
  }
}

} // namespace elegoo_car_pkg
