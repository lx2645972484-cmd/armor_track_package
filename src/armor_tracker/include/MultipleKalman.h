#ifndef MULTIPLEKALMAN_H
#define MULTIPLEKALMAN_H

#include "KalmanFilter.h"
#include <vector>

class MultipleKalman
{
private:
    Kalman k1{3, 1};       
    Kalman k2{3, 1};
    bool active1 = false;
    bool active2 = false;
    double pre_yaw1 = 0.0;  // 上一次成功更新的 yaw
    double pre_yaw2 = 0.0;

public:
    void update(const std::vector<double>& measurements, double delta_time);

    double min_angle_diff(double a, double b);

    double get_yaw1() const { return active1 ? k1.X(0) : std::numeric_limits<double>::quiet_NaN(); }
    double get_yaw2() const { return active2 ? k2.X(0) : std::numeric_limits<double>::quiet_NaN(); }
    bool is_active1() const { return active1; }
    bool is_active2() const { return active2; }
};

#endif