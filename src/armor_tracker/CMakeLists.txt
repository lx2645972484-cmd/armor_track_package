cmake_minimum_required(VERSION 3.8)
project(armor_tracker_node)

# 编译选项（可选）
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

add_definitions(-DARMOR_TRACKER_USE_MVSDK)

# C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 查找依赖包
find_package(ament_cmake REQUIRED)
find_package(armor_interfaces REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(tf2 REQUIRED)
find_package(tf2_ros REQUIRED)
find_package(tf2_geometry_msgs REQUIRED)
find_package(OpenCV REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(cv_bridge REQUIRED)
find_package(image_transport REQUIRED)
find_package(Eigen3 REQUIRED)
message(STATUS "Eigen3_INCLUDE_DIRS = ${Eigen3_INCLUDE_DIRS}")


# 定义可执行文件的源文件列表
# ⚠️ 请根据你的实际源码位置和文件名修改这里
set(SOURCES
  src/main1.cpp 
  src/KalmanFilter.cpp 
  src/LightBarProcessor.cpp
  src/MathTool.cpp
  src/Preprocessing.cpp
  src/RotationCenterSolver.cpp
  src/Vedio.cpp
  src/ArmorTracker.cpp
  src/armor_camera_capture.cpp
  src/galaxy_camera.cpp
              # 主程序入口
                # 主程序入口
  # 添加其他需要的源文件...
)

# 添加可执行文件
add_executable(armor_tracker_node ${SOURCES})

# 包含头文件目录（假设头文件在 include/ 下）
target_include_directories(armor_tracker_node
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${OpenCV_INCLUDE_DIRS}
    ${Eigen3_INCLUDE_DIRS}
  PUBLIC
    /home/eee/Galaxy_Linux-x86_Gige-U3_32bits-64bits_2.4.2507.9231/Galaxy_camera/inc
)

target_link_directories(armor_tracker_node PRIVATE /home/eee/linux_camara/lib/x64)

# 链接 OpenCV 库（非 ROS 依赖需要手动链接）
target_link_libraries(armor_tracker_node
  ${OpenCV_LIBS}
  Eigen3::Eigen
  MVSDK
  gxiapi
  tf2_geometry_msgs::tf2_geometry_msgs
)

# 处理 ROS 依赖（自动处理包含目录和链接）
ament_target_dependencies(armor_tracker_node
  armor_interfaces
  geometry_msgs
  rclcpp
  std_msgs
  tf2
  tf2_ros
  tf2_geometry_msgs
  sensor_msgs
  cv_bridge
  image_transport
)

# 安装可执行文件
install(TARGETS armor_tracker_node
  DESTINATION lib/${PROJECT_NAME}
)

# 安装其他资源（如 launch、config 等，根据你的需要）
install(DIRECTORY launch config rviz
  DESTINATION share/${PROJECT_NAME}
  OPTIONAL  # 如果目录不存在也不会报错
)

# 必须的 ament 打包
ament_package()