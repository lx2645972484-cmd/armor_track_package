#include "KalmanFilter.h"  
#include <Eigen/Dense>
#include <cmath>

// 构造函数
// 构造函数
Kalman::Kalman(int state, int observe)  
{
    // 初始化矩阵维度
    F = Eigen::MatrixXd::Identity(state, state);
    
    // 【修改这里】H 的维度应该是 observe x state，且只观测第一个位置 yaw
    H = Eigen::MatrixXd::Zero(observe, state);
    if(observe > 0 && state > 0) {
        H(0, 0) = 1.0; 
    }
    
    Q = Eigen::MatrixXd::Identity(state, state) * 1;
    R = Eigen::MatrixXd::Identity(observe, observe) * 10;
    P = Eigen::MatrixXd::Identity(state, state);
    X = Eigen::VectorXd::Zero(state);
    I = Eigen::MatrixXd::Identity(state, state);
}

// 预测步骤
double Kalman::predict()
{
    X = F * X;  // 状态预测
    P = F * P * F.transpose() + Q;  

    return X(0);
}

// 更新步骤
double Kalman::update(double yaw,double delta_time,double pre_yaw)
{
    Eigen::VectorXd Z(1);
    Z << yaw;
    
    // 计算卡尔曼增益
    Eigen::MatrixXd S = H * P * H.transpose() + R;
    Eigen::MatrixXd K = P * H.transpose() * S.inverse();
    
    // 更新状态
    X = X + K * (Z - H * X);
    
    // 更新误差协方差（使用Joseph形式提高数值稳定性）
    int state_size = X.size();
    // Eigen::MatrixXd I_state = Eigen::MatrixXd::Identity(state_size, state_size);
    // P = (I_state - K * H) * P * (I_state - K * H).transpose() + K * R * K.transpose();
    
    
    P = (I - K * H) * P;
    return X(0);
}

void Kalman::init(double yaw,double delta_time,double pre_yaw)
{
    double w = yaw - pre_yaw;
    double a_w = w/delta_time;
    
    F << 1, delta_time, 1.0/2.0 * delta_time * delta_time,
         0,1,delta_time,
         0,0,1;

    X << yaw,w,a_w;
}