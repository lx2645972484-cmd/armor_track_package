#ifndef KALMANFILTER_H
#define KALMANFILTER_H
#include <Eigen/Dense>
#include <opencv2/opencv.hpp>
#include <vector>
#include <cmath>
#include <deque>
#include "geometry_msgs/msg/point_stamped.hpp"

//接口：
class Kalman
{
public:
    Eigen::MatrixXd F;  // 状态转移矩阵
    Eigen::MatrixXd H;  // 观测矩阵
    Eigen::MatrixXd Q;  // 过程噪声协方差
    Eigen::MatrixXd R;  // 观测噪声协方差
    Eigen::MatrixXd P;  // 估计误差协方差
    Eigen::VectorXd X;  // 状态向量
    Eigen::MatrixXd Ft;
    Eigen::MatrixXd I;
    
    Kalman(int state,int guance);

    Eigen::VectorXd predict(double dt);

    Eigen::VectorXd update(geometry_msgs::msg::PointStamped point);

    void init(geometry_msgs::msg::PointStamped point,double delta_time);
};

#endif