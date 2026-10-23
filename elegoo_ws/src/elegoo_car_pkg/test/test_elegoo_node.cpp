#include <gtest/gtest.h>
#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/twist.hpp"
#include "std_msgs/msg/float32.hpp"
#include "sensor_msgs/msg/range.hpp"
#include "elegoo_car_pkg/elegoo_car_node.hpp"
#include "mock_transport.hpp"

class ElegooNodeTestFixture : public ::testing::Test {
protected:
  static void SetUpTestSuite() { rclcpp::init(0, nullptr); }
  static void TearDownTestSuite() { rclcpp::shutdown(); }

  void SetUp() override {
    mock_transport_ = std::make_unique<elegoo_car_pkg::test::MockTransport>();
    mock_ptr_ = mock_transport_.get();
    mock_transport_->connect();

    node_ = std::make_shared<elegoo_car_pkg::ElegooCarNode>(std::move(mock_transport_));
  }

  void TearDown() override {
    node_.reset();
  }

  std::unique_ptr<elegoo_car_pkg::test::MockTransport> mock_transport_;
  elegoo_car_pkg::test::MockTransport* mock_ptr_;
  std::shared_ptr<elegoo_car_pkg::ElegooCarNode> node_;
};

TEST_F(ElegooNodeTestFixture, CmdVelZeroSendsStop) {
  auto pub = node_->create_publisher<geometry_msgs::msg::Twist>("/cmd_vel", 10);
  geometry_msgs::msg::Twist msg;
  msg.linear.x = 0.0;
  msg.angular.z = 0.0;

  pub->publish(msg);
  rclcpp::spin_some(node_);

  ASSERT_FALSE(mock_ptr_->sent_messages.empty());
  EXPECT_NE(mock_ptr_->sent_messages.back().find("\"N\":1"), std::string::npos);
}

TEST_F(ElegooNodeTestFixture, CmdVelForwardSendsMotionCommand) {
  auto pub = node_->create_publisher<geometry_msgs::msg::Twist>("/cmd_vel", 10);
  geometry_msgs::msg::Twist msg;
  msg.linear.x = 0.5; // Forward
  msg.angular.z = 0.0;

  pub->publish(msg);
  rclcpp::spin_some(node_);

  ASSERT_FALSE(mock_ptr_->sent_messages.empty());
  EXPECT_NE(mock_ptr_->sent_messages.back().find("\"N\":3"), std::string::npos);
  EXPECT_NE(mock_ptr_->sent_messages.back().find("\"D1\":3"), std::string::npos); // Direction 3 = Forward
}

TEST_F(ElegooNodeTestFixture, SensorQueryPublishesRangeAndTriggersAvoidance) {
  // Queue a sensor response showing an obstacle at 15 cm (below default 25 cm safety threshold)
  mock_ptr_->queue_response("{\"N\":21,\"D1\":15.0}");

  float received_sonar = -1.0f;
  auto sub = node_->create_subscription<std_msgs::msg::Float32>(
    "/sonar_distance", 10, [&received_sonar](const std_msgs::msg::Float32::SharedPtr msg) {
      received_sonar = msg->data;
    });

  // Spin control loop iteration
  rclcpp::sleep_for(std::chrono::milliseconds(120));
  rclcpp::spin_some(node_);

  EXPECT_FLOAT_EQ(received_sonar, 15.0f);

  // Verify that safety backup command (Direction 4) was triggered due to obstacle at 15cm
  bool backup_triggered = false;
  for (const auto& msg : mock_ptr_->sent_messages) {
    if (msg.find("\"N\":3") != std::string::npos && msg.find("\"D1\":4") != std::string::npos) {
      backup_triggered = true;
      break;
    }
  }
  EXPECT_TRUE(backup_triggered);
}