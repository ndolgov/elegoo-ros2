#include <gtest/gtest.h>
#include "elegoo_car_pkg/elegoo_protocol.hpp"

using namespace elegoo_car_pkg;

TEST(ElegooProtocolTest, CreateMotionCommandForward) {
  std::string cmd = ElegooProtocol::create_motion_command(3, 150);
  EXPECT_NE(cmd.find("\"H\":1"), std::string::npos);
  EXPECT_NE(cmd.find("\"N\":3"), std::string::npos);
  EXPECT_NE(cmd.find("\"D1\":3"), std::string::npos);
  EXPECT_NE(cmd.find("\"D2\":150"), std::string::npos);
}

TEST(ElegooProtocolTest, CreateMotionCommandTurnLeft) {
  std::string cmd = ElegooProtocol::create_motion_command(1, 200);
  EXPECT_NE(cmd.find("\"D1\":1"), std::string::npos);
  EXPECT_NE(cmd.find("\"D2\":200"), std::string::npos);
}

TEST(ElegooProtocolTest, CreateStopCommand) {
  std::string cmd = ElegooProtocol::create_stop_command();
  EXPECT_NE(cmd.find("\"H\":1"), std::string::npos);
  EXPECT_NE(cmd.find("\"N\":1"), std::string::npos);
  EXPECT_NE(cmd.find("\"D3\":1"), std::string::npos);
}

TEST(ElegooProtocolTest, CreateSonarQuery) {
  std::string cmd = ElegooProtocol::create_sonar_query();
  EXPECT_NE(cmd.find("\"H\":1"), std::string::npos);
  EXPECT_NE(cmd.find("\"N\":21"), std::string::npos);
}

TEST(ElegooProtocolTest, ParseValidSonarResponseJSON) {
  std::string json_resp = "{\"N\":21,\"D1\":45.5}";
  float distance = 0.0f;
  bool success = ElegooProtocol::parse_sonar_response(json_resp, distance);
  EXPECT_TRUE(success);
  EXPECT_FLOAT_EQ(distance, 45.5f);
}

TEST(ElegooProtocolTest, ParseRawStringSonarResponse) {
  std::string raw_resp = "23.4";
  float distance = 0.0f;
  bool success = ElegooProtocol::parse_sonar_response(raw_resp, distance);
  EXPECT_TRUE(success);
  EXPECT_FLOAT_EQ(distance, 23.4f);
}

TEST(ElegooProtocolTest, ParseInvalidSonarResponse) {
  std::string invalid_resp = "INVALID_DATA";
  float distance = 0.0f;
  bool success = ElegooProtocol::parse_sonar_response(invalid_resp, distance);
  EXPECT_FALSE(success);
}