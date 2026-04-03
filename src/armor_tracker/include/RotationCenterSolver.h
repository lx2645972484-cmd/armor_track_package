#ifndef ROTATIONCENTERSOLVER_H
#define ROTATIONCENTERSOLVER_H

#include <opencv2/opencv.hpp>
#include <vector>
#include <Eigen/Dense>

class RotationCenterSolver
{
private:
    struct Observation
    {
        Eigen::Vector3d armor_pos; // 装甲板中心 (tvec)
        Eigen::Vector3d normal;    // 装甲板法向量 (Z轴朝向)
    };

    std::vector<Observation> history;
    int max_history_size;

public:
    RotationCenterSolver(int history_size = 50);

    // 添加新的观测数据
    void addObservation(const cv::Mat &tvec, const cv::Mat &rvec);
    
    // 计算旋转中心 (最小二乘法)
    bool solve(cv::Point3f &result_center);

    // 获取拟合出的半径 (调试用)
    float getLastRadius(const cv::Point3f &center);
    
};

#endif