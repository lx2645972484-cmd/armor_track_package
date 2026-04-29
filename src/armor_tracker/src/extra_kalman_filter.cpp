// #include "extra_kalman_filter.hpp"

// ExtraKalman::ExtraKalman(int state, int guance)
// {
//     F = Eigen::MatrixXd::Identity(state, state);

//     H = Eigen::MatrixXd::Zero(guance, state);
//     assert(state == 9 && guance == 4);

//     // H = Eigen::MatrixXd::Zero(4, 9);
//     // H(0,0) = 1.0;
//     // H(2,2) = 1.0;
//     // H(8,8) = 1.0;
//     // H(8,9) =

//     Q = Eigen::MatrixXd::Identity(state, state) * 10;
//     R = Eigen::MatrixXd::Identity(guance, guance) * 10;
//     P = Eigen::MatrixXd::Identity(state, state);
//     X = Eigen::VectorXd::Zero(state);
//     I = Eigen::MatrixXd::Identity(state, state);
// }

// Eigen::Vector3d ExtraKalman::init(geometry_msgs::msg::PointStamped point, double delta_time, cv::Mat rvec, cv::Mat tvec)
// {
//     F(0, 4) = delta_time;
//     F(1, 5) = delta_time;
//     F(2, 6) = delta_time;
//     F(8, 9) = delta_time;

//     X << rotated_center[0], rotated_center[1], rotated_center[2], 0, 0, 0, 2, theta, 0;

//     P = Eigen::MatrixXd::Identity(9, 9);
//     P.block<3, 3>(3, 3) = Eigen::MatrixXd::Identity(3, 3) * 100.0;
//     P.block<3, 3>(6, 6) = Eigen::MatrixXd::Identity(3, 3) * 150.0;

//     H = Eigen::MatrixXd::Zero(4, 9);
//     H(0, 0) = 1.0;
//     H(2, 2) = 1.0;
//     H(8, 8) = 1.0;
//     H(8, 9) =
// }

#include "extra_kalman_filter.hpp"

ExtraKalman::ExtraKalman(int state_dim, int obs_dim)
{
    X = Eigen::VectorXd::Zero(state_dim);
    P = Eigen::MatrixXd::Identity(state_dim, state_dim);
    F = Eigen::MatrixXd::Identity(state_dim, state_dim);
    I = Eigen::MatrixXd::Identity(state_dim, state_dim);

    // 调整 Q 阵 (过程噪声)：信任模型还是信任观测
    // 匀速运动模型，速度和角速度的噪声可以设大一点
    Q = Eigen::MatrixXd::Identity(state_dim, state_dim);
    // Q(0,0) = Q(1,1) = Q(2,2) = 0.1;   // x, y, z
    // Q(3,3) = Q(4,4) = Q(5,5) = 1.0;   // vx, vy, vz
    // Q(6,6) = 0.01;                    // 半径 r 几乎不变
    // Q(7,7) = 0.1;                     // theta
    // Q(8,8) = 1.0;                     // omega 角速度可能突变

    // 调整 R 阵 (观测噪声)：取决于你的相机和PnP解算精度
    R_mat = Eigen::MatrixXd::Identity(obs_dim, obs_dim);
    // R_mat(0,0) = 0.05; // x 观测噪声
    // R_mat(1,1) = 0.05; // y 观测噪声
    // R_mat(2,2) = 0.05; // z 观测噪声
    // R_mat(3,3) = 0.1;  // theta 观测噪声
}

void ExtraKalman::init(const Eigen::VectorXd& initial_X)
{
    X = initial_X;
    
    // 初始化协方差矩阵 P
    P = Eigen::MatrixXd::Identity(9, 9);
    P.block<3, 3>(3, 3) = Eigen::MatrixXd::Identity(3, 3) * 100.0; // 初始速度极不确定
    P(6, 6) = 10.0; // r 初始不确定度
    P(8, 8) = 100.0; // omega 初始不确定度
    
    is_initialized = true;
}

void ExtraKalman::predict(double dt)
{
    // 动态更新状态转移矩阵 F 中的 dt
    F(0, 3) = dt;
    F(1, 4) = dt;
    F(2, 5) = dt;
    F(7, 8) = dt;

    // 预测状态 X = F * X
    X = F * X;
    // 规范化预测的角度
    X(7) = normalize_angle(X(7));

    // 预测协方差 P = F * P * F^T + Q
    P = F * P * F.transpose() + Q;
}

void ExtraKalman::update(const Eigen::VectorXd& Z)
{

    double r_pred = X(6);
    double theta_pred = X(7);

  
    Eigen::VectorXd Z_pred(4);
    Z_pred(0) = X(0) + r_pred * std::cos(theta_pred);
    Z_pred(1) = X(1) + r_pred * std::sin(theta_pred);
    Z_pred(2) = X(2);
    Z_pred(3) = theta_pred;

   
    Eigen::MatrixXd H = Eigen::MatrixXd::Zero(4, 9);
    H(0, 0) = 1.0;
    H(0, 6) = std::cos(theta_pred);
    H(0, 7) = -r_pred * std::sin(theta_pred);

    H(1, 1) = 1.0;
    H(1, 6) = std::sin(theta_pred);
    H(1, 7) = r_pred * std::cos(theta_pred);

    H(2, 2) = 1.0;
    H(3, 7) = 1.0;


    Eigen::VectorXd Y = Z - Z_pred;
    
    // 由于目标有4个装甲板，观测到的 theta 可能突然跳变 90度 (PI/2)
    // 这里我们将残差限制在 [-pi/4, pi/4] 或者进行就近匹配，防止滤波发散
    Y(3) = normalize_angle(Y(3)); 
    // 进阶做法：如果 Y(3) 接近 pi/2，说明切换了装甲板，你可以手动对 X(7) 加上 pi/2 来对齐

  
    Eigen::MatrixXd S = H * P * H.transpose() + R_mat;
    Eigen::MatrixXd K = P * H.transpose() * S.inverse();

    // 5. 更新状态 X = X + K * Y
    X = X + K * Y;
    X(7) = normalize_angle(X(7)); // 确保角度始终在合理区间

   
    P = (I - K * H) * P;
}

double ExtraKalman::normalize_angle(double angle)
{
    while (angle > M_PI) angle -= 2.0 * M_PI;
    while (angle < -M_PI) angle += 2.0 * M_PI;
    return angle;
}
