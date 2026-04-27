#ifndef EXTRA_KALMAN_FILTER_HPP_
#define EXTRA_KALMAN_FILTER_HPP_
#include <Eigen/Dense>
#include <opencv2/opencv.hpp>
#include <opencv2/core/eigen.hpp>
#include "assert.hpp"
#include "geometry_msgs/msg/point_stamped.hpp"

class ExtraKalman
{
public:
    Eigen::MatrixXd F; // 状态转移矩阵
    Eigen::MatrixXd H; // 观测矩阵
    Eigen::MatrixXd Q; // 过程噪声协方差
    Eigen::MatrixXd R; // 观测噪声协方差
    Eigen::MatrixXd P; // 估计误差协方差
    Eigen::VectorXd X; // 状态向量
    Eigen::MatrixXd Ft;
    Eigen::MatrixXd I;

    Eigen::Vector3d pos;
    Eigen::Vector3d norm;
    Eigen::Vector3d rotated_center;

    ExtraKalman(int state, int guance);

    // Eigen::VectorXd predict(double dt);

    // Eigen::VectorXd update(geometry_msgs::msg::PointStamped point);

    Eigen::Vector3d init(geometry_msgs::msg::PointStamped point,double delta_time,cv::Mat rvec, cv::Mat tvec);
};

#endif