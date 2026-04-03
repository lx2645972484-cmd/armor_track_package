#include "RotationCenterSolver.h"
#include <opencv2/core/eigen.hpp>

RotationCenterSolver::RotationCenterSolver(int history_size) : max_history_size(history_size) {}

void RotationCenterSolver::addObservation(const cv::Mat &tvec, const cv::Mat &rvec)
{
    cv::Mat R;
    cv::Rodrigues(rvec, R); // 旋转向量 -> 旋转矩阵

    // 获取装甲板位置 (tvec)
    Eigen::Vector3d pos;
    cv::cv2eigen(tvec, pos);

    // 获取法向量 (旋转矩阵的第三列，即Z轴)
    Eigen::Vector3d norm;
    cv::cv2eigen(R.col(2), norm);

    // 存入历史记录
    history.push_back({pos, norm});
    if (history.size() > max_history_size)
    {
        history.erase(history.begin()); // 移除最老的数据
    }
}

bool RotationCenterSolver::solve(cv::Point3f &result_center)
{
    if (history.size() < 10)
        return false; // 数据太少不算

    // 构建线性方程组 Ax = b
    // 这里对应公式中的: (Sum(I - nnT)) * C = Sum((I - nnT) * A)
    Eigen::Matrix3d Mat_A = Eigen::Matrix3d::Zero();
    Eigen::Vector3d Vec_b = Eigen::Vector3d::Zero();

    Eigen::Matrix3d I = Eigen::Matrix3d::Identity();

    for (const auto &obs : history)
    {
        // 将 Point3f 转为 Mat 方便计算
        // cv::Mat n = (cv::Mat_<double>(3, 1) << obs.normal.x, obs.normal.y, obs.normal.z);
        const Eigen::Vector3d &n = obs.normal;
        // cv::Mat p = (cv::Mat_<double>(3, 1) << obs.armor_pos.x, obs.armor_pos.y, obs.armor_pos.z);
        const Eigen::Vector3d &p = obs.armor_pos;

        // term = I - n * n^T
        Eigen::Matrix3d term = I - n * n.transpose();

        // 累加
        Mat_A += term;
        Vec_b += term * p;
    }

    // 求解 C
    Eigen::Vector3d C;
    // 使用 SVD 分解求逆，更稳定
    Eigen::JacobiSVD<Eigen::Matrix3d> svd(Mat_A, Eigen::ComputeFullU | Eigen::ComputeFullV);
C = svd.solve(Vec_b);

    result_center.x = (float)C(0);
    result_center.y = (float)C(1);
    result_center.z = (float)C(2);

    // 简单校验：计算出的中心不应该离装甲板太远 (例如超过1米)
    // 这里取最新一帧做对比
    double dist = (history.back().armor_pos - C).norm();
    if (dist > 1.0 || dist < 0.05)
        return false; // 异常值过滤

    return true;
}

float RotationCenterSolver::getLastRadius(const cv::Point3f &center)
{
    if (history.empty())
    {
        return 0.0f;
    }
    Eigen::Vector3d center_eig(center.x, center.y, center.z);
    // 取出最近一次记录的装甲板位置
    Eigen::Vector3d last_armor_pos = history.back().armor_pos;
    
    // 计算最近装甲板位置到旋转中心的直线距离
    Eigen::Vector3d difference = last_armor_pos - center_eig;
    double distance = difference.norm();  // 计算向量的长度（即距离）
    
    // 将双精度距离转为单精度浮点数返回
    return (float)distance;
}