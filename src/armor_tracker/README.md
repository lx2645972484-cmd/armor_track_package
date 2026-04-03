cd ros2
cd src
cd armor_detect_ros2-main
colcon build
. install/setup.bash
ros2 launch armor_tracker_node 1.launch.py

ros2 param set /armor_tracker setting 0