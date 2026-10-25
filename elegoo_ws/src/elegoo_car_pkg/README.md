```sh
sudo apt update
sudo apt install ros-jazzy-rclcpp ros-jazzy-geometry-msgs ros-jazzy-std-msgs nlohmann-json3-dev

mkdir -p ~/elegoo_ws/src
cd ~/elegoo_ws/src/elegoo-car-pkg

colcon build --symlink-install

cd ~/elegoo_ws
colcon test --packages-select elegoo_car_pkg --event-handlers console_cohesion+
colcon test-result --verbose

source ~/elegoo_ws/install/setup.bash
ros2 run elegoo_car_pkg elegoo_car_pkg_node

ros2 topic pub /cmd_vel geometry_msgs/msg/Twist "{linear: {x: 0.2}, angular: {z: 0.0}}"

ros2 topic echo /sonar_distance
```

```sh
cd elegoo-ros2/elegoo_ws/
source /opt/ros/jazzy/setup.bash
colcon build --packages-select elegoo_car_pkg
colcon test --packages-select elegoo_car_pkg
```
