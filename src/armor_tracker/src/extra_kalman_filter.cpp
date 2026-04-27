#include "extra_kalman_filter.hpp"

ExtraKalman::ExtraKalman(int state, int guance)
{
    F = Eigen::MatrixXd::Identity(state, state);
    
    
    H = Eigen::MatrixXd::Zero(guance, state);
    assert(state == 9 && guance == 4);
    
    // H = Eigen::MatrixXd::Zero(4, 9);
    // H(0,0) = 1.0;
    // H(2,2) = 1.0;
    // H(8,8) = 1.0;
    // H(8,9) = 
    
    Q = Eigen::MatrixXd::Identity(state, state) * 10;
    R = Eigen::MatrixXd::Identity(guance, guance) * 10;
    P = Eigen::MatrixXd::Identity(state, state);
    X = Eigen::VectorXd::Zero(state);
    I = Eigen::MatrixXd::Identity(state, state);
}

Eigen::Vector3d ExtraKalman::init(geometry_msgs::msg::PointStamped point,double delta_time,cv::Mat rvec, cv::Mat tvec)
{
    cv::Mat R;
    cv::Rodrigues(rvec, R); // 旋转向量 -> 旋转矩阵

    cv::cv2eigen(tvec, pos); // huoqu pos

    cv::cv2eigen(R.col(2), norm); // huoqu norm

    norm[0] *= 2;
    norm[1] *= 2;
    norm[2] *= 2;

    rotated_center[0] = pos[0] + norm[0];
    rotated_center[1] = pos[1] + norm[1];
    rotated_center[2] = pos[2] + norm[2];

    double theta = std::atan2(rotated_center[0] - pos[0], rotated_center[2] - pos[2]);

    F(0,4) = delta_time;
    F(1,5) =delta_time;
    F(2,6) = delta_time;
    F(8,9) = delta_time;
    
    X << rotated_center[0], rotated_center[1], rotated_center[2], 0, 0, 0, 2, theta, 0;

    P = Eigen::MatrixXd::Identity(9, 9);
    P.block<3,3>(3,3) = Eigen::MatrixXd::Identity(3,3) * 100.0;
    P.block<3,3>(6,6) = Eigen::MatrixXd::Identity(3,3) * 150.0;
    
    H = Eigen::MatrixXd::Zero(4, 9);
    H(0,0) = 1.0;
    H(2,2) = 1.0;
    H(8,8) = 1.0;
    H(8,9) = 
}
