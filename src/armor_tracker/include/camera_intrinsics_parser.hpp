#ifndef CAMERA_INTRINSICS_PARSER_HPP_
#define CAMERA_INTRINSICS_PARSER_HPP_

#include <string>
#include <opencv2/opencv.hpp>
#include <sensor_msgs/msg/camera_info.hpp>

namespace rm_vision {

// 相机内参结构体，方便在 PnP 解算和图像处理中统一传递
struct CameraIntrinsics {
    cv::Mat K;
    cv::Mat D;
    cv::Size image_size;
    std::string distortion_model;
    std::string camera_name;
    
    // 判断内参是否为空
    bool isValid() const {
        return !K.empty() && !D.empty();
    }
};

class CameraIntrinsicsParser {
public:
    /**
     * @brief 解析相机内参 YAML 文件
     * @param yaml_path 标定文件路径
     * @param intrinsics 输出的内参结构体
     * @return true 成功, false 失败
     */
    static bool parseYaml(const std::string& yaml_path, CameraIntrinsics& intrinsics);
};

} // namespace rm_vision

#endif // CAMERA_INTRINSICS_PARSER_HPP_