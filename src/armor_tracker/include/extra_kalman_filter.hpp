// #ifndef EXTRA_KALMAN_FILTER_HPP_
// #define EXTRA_KALMAN_FILTER_HPP_
// #include <Eigen/Dense>
// #include <opencv2/opencv.hpp>
// #include <opencv2/core/eigen.hpp>
// #include "assert.hpp"
// #include "geometry_msgs/msg/point_stamped.hpp"
// #include "rclcpp/rclcpp.hpp"
// #include "tf2_ros/transform_listener.h"
// #include "tf2_ros/buffer.h"

// class ExtraKalman
// {
// public:
//     Eigen::MatrixXd F; // 状态转移矩阵
//     Eigen::MatrixXd H; // 观测矩阵
//     Eigen::MatrixXd Q; // 过程噪声协方差
//     Eigen::MatrixXd R; // 观测噪声协方差
//     Eigen::MatrixXd P; // 估计误差协方差
//     Eigen::VectorXd X; // 状态向量
//     Eigen::MatrixXd Ft;
//     Eigen::MatrixXd I;

//     Eigen::Vector3d pos;
//     Eigen::Vector3d norm;
//     Eigen::Vector3d rotated_center;

//     ExtraKalman(int state, int guance);

//     // Eigen::VectorXd predict(double dt);

//     // Eigen::VectorXd update(geometry_msgs::msg::PointStamped point);

//     Eigen::Vector3d init(geometry_msgs::msg::PointStamped point, double delta_time, cv::Mat rvec, cv::Mat tvec);


// };

// #endif

#ifndef EXTRA_KALMAN_FILTER_HPP
#define EXTRA_KALMAN_FILTER_HPP

#include <Eigen/Dense>
#include <iostream>
#include <cmath>

class ExtraKalman
{
public:
    // 状态向量: [x, y, z, vx, vy, vz, r, theta, omega]^T
    Eigen::VectorXd X;      // 状态向量 (9x1)
    Eigen::MatrixXd P;      // 状态协方差矩阵 (9x9)
    Eigen::MatrixXd F;      // 状态转移矩阵 (9x9)
    Eigen::MatrixXd Q;      // 过程噪声协方差 (9x9)
    Eigen::MatrixXd R_mat;  // 测量噪声协方差 (4x4)
    Eigen::MatrixXd I;      // 单位阵 (9x9)

    bool is_initialized = false;

    ExtraKalman(int state_dim, int obs_dim);

    // 初始化滤波器状态
    void init(const Eigen::VectorXd& initial_X);

    // 预测步骤
    void predict(double dt);

    // 更新步骤
    // 观测向量 Z = [x_obs, y_obs, z_obs, theta_obs]^T
    void update(const Eigen::VectorXd& Z);

private:
    // 角度归一化到 [-pi, pi]
    double normalize_angle(double angle);
};

#endif