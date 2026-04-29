#include "camera_intrinsics_parser.hpp"
#include <camera_calibration_parsers/parse_yml.hpp>
#include <rclcpp/rclcpp.hpp>
#include <iostream> // 用于基础报错输出，不依赖 ROS logger

namespace rm_vision {

bool CameraIntrinsicsParser::parseYaml(const std::string& yaml_path, CameraIntrinsics& intrinsics) {
    sensor_msgs::msg::CameraInfo camera_info;
    std::string camera_name;

    // 1. 调用 ROS 2 标准的 readCalibration 函数
    bool success = camera_calibration_parsers::readCalibrationYml(yaml_path, camera_name, camera_info);
    
    if (!success) {
        std::cerr << "[CameraIntrinsicsParser] 无法解析标定文件: " << yaml_path << std::endl;
        return false;
    }

    // 2. 提取数据并克隆（保证内存独立）
    // camera_info.k 是 std::array<double, 9>
    intrinsics.K = cv::Mat(3, 3, CV_64F, camera_info.k.data()).clone();
    
    // camera_info.d 是 std::vector<double>
    intrinsics.D = cv::Mat(camera_info.d.size(), 1, CV_64F, camera_info.d.data()).clone();

    // 3. 提取其他必要信息
    intrinsics.image_size = cv::Size(camera_info.width, camera_info.height);
    intrinsics.distortion_model = camera_info.distortion_model;
    intrinsics.camera_name = camera_name;

    return true;
}

} // namespace rm_vision