#ifndef DRAWMYFUNCTION_H
#define DRAWMYFUNCTION_H

#include <Eigen/Dense>
#include <opencv2/opencv.hpp>
#include <vector>
#include <cmath>
#include <deque>


class DrawKalmanFunction
{
public:
    double kuan;
    double chang;

    DrawKalmanFunction(double kuan, double chang);

    cv::Mat DrawMyFunctionLine();

    cv::Point DrawMyFxxkingPoint(cv::Mat &image, double time, double w);

    cv::Point DrawMyFxxkingPoint(cv::Mat &image, double time, double w, double);
};

#endif