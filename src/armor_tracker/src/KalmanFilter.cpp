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
    assert(state == 9 && observe == 3);
    
    H = Eigen::MatrixXd::Zero(3, 9);
    H(0,0) = 1.0;
    H(1,1) = 1.0;
    H(2,2) = 1.0;
    
    Q = Eigen::MatrixXd::Identity(state, state) * 10;
    R = Eigen::MatrixXd::Identity(observe, observe) * 10;
    P = Eigen::MatrixXd::Identity(state, state);
    X = Eigen::VectorXd::Zero(state);
    I = Eigen::MatrixXd::Identity(state, state);
}

// 预测步骤
Eigen::VectorXd Kalman::predict(double dt)
{
        // 动态构造 9x9 状态转移矩阵（恒加速模型）
    F = Eigen::MatrixXd::Identity(9, 9);
    
    // 位置 += 速度 * dt + 0.5 * 加速度 * dt^2
    F(0,3) = dt;  F(0,6) = 0.5 * dt * dt;
    F(1,4) = dt;  F(1,7) = 0.5 * dt * dt;
    F(2,5) = dt;  F(2,8) = 0.5 * dt * dt;
    
    // 速度 += 加速度 * dt
    F(3,6) = dt;
    F(4,7) = dt;
    F(5,8) = dt;
    
    X = F * X;  // 状态预测
    P = F * P * F.transpose() + Q;  

    return X;
}

// 更新步骤
Eigen::VectorXd Kalman::update(geometry_msgs::msg::PointStamped point)
{
    Eigen::VectorXd Z(3); // 观测向量
    Z << point.point.x, point.point.y, point.point.z;
    
    // 计算卡尔曼增益
    Eigen::MatrixXd S = H * P * H.transpose() + R;
    Eigen::MatrixXd K = P * H.transpose() * S.inverse();
    
    Eigen::VectorXd y = Z - H * X;   // 新息
    X = X + K * y;
    
    // Joseph 形式协方差更新（确保对称正定）
    Eigen::MatrixXd I_KH = I - K * H;
    P = I_KH * P * I_KH.transpose() + K * R * K.transpose();
    return X;
}

void Kalman::init(geometry_msgs::msg::PointStamped point,double delta_time)
{
    // 初始化状态转移矩阵F，假设状态向量为 [x, y, z, v_x, v_y, v_z, a_x, a_y, a_z]，其中包含位置、速度和加速度
    
    F = Eigen::MatrixXd::Identity(9, 9);
    
    // 位置对速度的依赖
    F(0, 3) = delta_time;   // x += vx * dt
    F(1, 4) = delta_time;   // y += vy * dt
    F(2, 5) = delta_time;   // z += vz * dt
    
    // 位置对加速度的依赖
    F(0, 6) = 0.5 * delta_time * delta_time;
    F(1, 7) = 0.5 * delta_time * delta_time;
    F(2, 8) = 0.5 * delta_time * delta_time;
    
    // 速度对加速度的依赖
    F(3, 6) = delta_time;
    F(4, 7) = delta_time;
    F(5, 8) = delta_time;
         // 加入加速度项
         // 注意：这里的加速度是通过差分计算得到的，可能会比较噪声，实际使用中可能需要滤波处理
         // 如果不需要加速度项，可以将状态维度改为6，并相应调整F矩阵和X向量
         // 如果需要更复杂的模型（如非线性模型），可能需要使用扩展卡尔曼滤波或其他非线性滤波方法


    X << point.point.x, point.point.y, point.point.z, 0, 0, 0, 0, 0, 0;
    P = Eigen::MatrixXd::Identity(9, 9);
    P.block<3,3>(3,3) = Eigen::MatrixXd::Identity(3,3) * 10.0;
    P.block<3,3>(6,6) = Eigen::MatrixXd::Identity(3,3) * 50.0;
}