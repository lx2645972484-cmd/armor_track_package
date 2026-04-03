cmake_minimum_required(VERSION 3.10)
project(rm_serial_driver)

## Use C++14
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

## By adding -Wall and -Werror, the compiler does not ignore warnings anymore,
## enforcing cleaner code.
## add_definitions(-Wall -Werror)

## Export compile commands for clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

#######################
## Find dependencies ##
#######################

find_package(ament_cmake_auto REQUIRED)
find_package(libstatistics_collector REQUIRED)
ament_auto_find_build_dependencies()

###########
## Build ##
###########

# 1. 编译共享库 (用于组件加载)
ament_auto_add_library(${PROJECT_NAME} SHARED
  src/rm_serial_driver.cpp
  src/crc.cpp
)

# 2. 注册组件 (用于 ROS2 组件管理)
rclcpp_components_register_node(${PROJECT_NAME}
  PLUGIN rm_serial_driver::RMSerialDriver
  EXECUTABLE ${PROJECT_NAME}_node
)

# 3. 【新增】定义独立的可执行文件 (解决 launch 找不到文件的问题)
# 注意：这里的源文件列表必须和上面的 ament_auto_add_library 保持一致
add_executable(rm_serial_node
  src/rm_serial_driver.cpp
  src/crc.cpp
  src/main.cpp
)

ament_target_dependencies(rm_serial_node
  rclcpp
  armor_interfaces # 确保这里包含了 armor_interfaces
  # ... 其他依赖 ...
)

# 4. 【新增】链接库
# 将可执行文件链接到共享库，这样可执行文件才能运行实际的逻辑
target_link_libraries(rm_serial_node
  ${PROJECT_NAME}
)

#############
## Testing ##
#############

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  list(APPEND AMENT_LINT_AUTO_EXCLUDE
    ament_cmake_copyright
    ament_cmake_uncrustify
  )
  ament_lint_auto_find_test_dependencies()
endif()

#############
## Install ##
#############

ament_auto_package(
  INSTALL_TO_SHARE
  config
  launch
)