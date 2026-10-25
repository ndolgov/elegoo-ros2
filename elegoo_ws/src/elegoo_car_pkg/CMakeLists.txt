cmake_minimum_required(VERSION 3.8)
project(elegoo_car_pkg)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic -Wno-unused-parameter)
endif()

# Find Dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(sensor_msgs REQUIRED) # Added: required by tests
find_package(std_msgs REQUIRED)
find_package(nlohmann_json REQUIRED)

# Define Package Shared/Static Library
add_library(elegoo_car_pkg
  src/tcp_transport.cpp
  src/elegoo_protocol.cpp
  src/elegoo_car_node.cpp
)
target_include_directories(elegoo_car_pkg PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)
ament_target_dependencies(elegoo_car_pkg
  rclcpp
  geometry_msgs
  std_msgs
)
target_link_libraries(elegoo_car_pkg nlohmann_json::nlohmann_json)

# Define Node Executable
add_executable(elegoo_car_pkg_node src/main.cpp)
target_link_libraries(elegoo_car_pkg_node elegoo_car_pkg)
ament_target_dependencies(elegoo_car_pkg_node
  rclcpp
  geometry_msgs
  std_msgs
)

# Install Rules
install(DIRECTORY include/ DESTINATION include)

# Fixed: Libraries go to lib/, executables go to lib/${PROJECT_NAME}
install(TARGETS elegoo_car_pkg
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)
install(TARGETS elegoo_car_pkg_node
  DESTINATION lib/${PROJECT_NAME}
)

# Testing Section (CRITICAL: Must be placed BEFORE ament_package())
if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  set(ament_cmake_copyright_FOUND TRUE)
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()

  find_package(ament_cmake_gtest REQUIRED)

  # 1. Protocol Unit Tests
  ament_add_gtest(test_elegoo_protocol test/test_elegoo_protocol.cpp)
  target_link_libraries(test_elegoo_protocol elegoo_car_pkg nlohmann_json::nlohmann_json)
  ament_target_dependencies(test_elegoo_protocol rclcpp)

  # 2. Node Unit Tests (with Mock Transport)
  ament_add_gtest(test_elegoo_node test/test_elegoo_node.cpp)
  target_include_directories(test_elegoo_node PRIVATE test) # Include mock_transport.hpp path
  target_link_libraries(test_elegoo_node elegoo_car_pkg nlohmann_json::nlohmann_json)
  ament_target_dependencies(test_elegoo_node rclcpp geometry_msgs sensor_msgs std_msgs)

  # 3. Integration Tests (Local TCP Socket Loopback)
  ament_add_gtest(test_integration_bridge test/test_integration_bridge.cpp)
  target_include_directories(test_integration_bridge PRIVATE test)
  target_link_libraries(test_integration_bridge elegoo_car_pkg nlohmann_json::nlohmann_json)
  ament_target_dependencies(test_integration_bridge rclcpp geometry_msgs sensor_msgs std_msgs)
endif()

# Final macro execution hook
ament_package()