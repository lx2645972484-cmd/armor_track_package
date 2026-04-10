// ArmorTracker.hpp
#ifndef ARMOR_TRACKER
#define ARMOR_TRACKER

#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <string>
#include <Eigen/Dense>

#include "armor_interfaces/msg/armor_info.hpp"
#include "armor_interfaces/msg/serial_driver.hpp"
#include "geometry_msgs/msg/transform_stamped.hpp"
#include "tf2_ros/transform_broadcaster.h"
#include "geometry_msgs/msg/point_stamped.hpp"
#include <message_filters/subscriber.h>
#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/float64.hpp"
#include "tf2/LinearMath/Matrix3x3.hpp"
#include "tf2/LinearMath/Quaternion.hpp"
#include "tf2_ros/transform_listener.h"
#include "armor_interfaces/msg/joint_state.hpp"
#include <armor_interfaces/msg/serial_receive_data.hpp>
#include "tf2_ros/buffer.h"
#include <tf2_ros/create_timer_ros.h>
#include <tf2_ros/message_filter.h>

#include "RotationCenterSolver.h"
#include "KalmanFilter.h"
#include "Preprocessing.h"
#include "Vedio.h"
#include "LightBarProcessor.h"
#include "MathTool.h"
#include "MultipleKalman.h"
#include "ArmorMsg.h"
// #include "armor_camera_capture.hpp"
#include "galaxy_camera.hpp"


class ArmorTracker : public rclcpp::Node
{
private:
    // 图像处理类
    PreProcess pp;

    // 视频信息类
    VedioProcessor vp;

    // 旋转中心处理类
    RotationCenterSolver centerSolver{1000}; // 记录最近1000帧 随便记
    // 灯条处理类
    LightBarProcessor lbp;

    // 工具类
    MathTool mtl;

    // 多目标卡尔曼类
    MultipleKalman mltkalman;

    // armor_camera::ArmorCameraCapture camera_capture_;

    bool camera_ready_ = false;
    bool has_last_frame_time_ = false;
    std::chrono::steady_clock::time_point last_frame_time_{};

    //存储LightBar的vector
    std::vector<LightBar> LightBarVector;

    // 绘图类
    // DrawKalmanFunction drawer{1000, 3500};

    cv::VideoCapture cap;

    armor_interfaces::msg::ArmorInfo armorTracker_; // 装甲信息结构体

    rclcpp::Publisher<armor_interfaces::msg::ArmorInfo>::SharedPtr armor_info_publisher; // 装甲信息发布者
    rclcpp::Publisher<geometry_msgs::msg::PointStamped>::SharedPtr center_publisher;     // 中心点发布者
    std::unique_ptr<tf2_ros::TransformBroadcaster> tf_broadcaster;     
    rclcpp::Publisher<armor_interfaces::msg::JointState>::SharedPtr tf_camera_to_world_publisher_;                  // TF广播器

    cv::Matx33d cv_to_ros; // OpenCV到ROS的转换矩阵
    double delta_t = 0.02; // 时间间隔

    int setting_;

    // CameraHandle hCamera;
    galaxy_camera::GalaxyCamera galaxy_camera_; // 使用GalaxyCamera类，索引从1开始，默认不自动启动采集线程

    rclcpp::Publisher<armor_interfaces::msg::SerialDriver>::SharedPtr serial_driver_publisher_;

    rclcpp::Subscription<armor_interfaces::msg::SerialReceiveData>::SharedPtr camera_to_world_sub_;

    armor_interfaces::msg::JointState joint_state_msg_; // 关节状态消息

    std::shared_ptr<tf2_ros::TransformListener> transform_listener_; // TF监听器
    
    std::unique_ptr<tf2_ros::Buffer> tf_buffer_; // TF缓冲区

    rclcpp::Publisher<geometry_msgs::msg::PointStamped>::SharedPtr target_point_pub_; // 目标点发布者

    std::string target_frame_; // 目标坐标系名称

    message_filters::Subscriber<geometry_msgs::msg::PointStamped> point_sub_; // 目标点订阅者

    std::shared_ptr<tf2_ros::MessageFilter<geometry_msgs::msg::PointStamped>> tf2_filter_; // 消息过滤器

    geometry_msgs::msg::PointStamped point_out; // 转换后的目标点

public:
    ArmorTracker();
    void run();
    

private:
    void publish_armor_msg();

    bool containLight(const LightBar &light_1, const LightBar &light_2);

    void ColorDetect(std::vector<cv::Point> contour,LightBar light,cv::Mat img);

    void publish_to_serial_driver(double yaw, double pitch,std::vector<cv::Point2f> armorPoints);

    void receiveDataCallback(armor_interfaces::msg::SerialReceiveData::SharedPtr msg);

    void msgCallback(const geometry_msgs::msg::PointStamped::SharedPtr point_ptr);  
};

#endif