#include "DrawMyFunction.h"

#include <Eigen/Dense>
#include <opencv2/opencv.hpp>
#include <vector>
#include <cmath>
#include <deque>


DrawKalmanFunction::DrawKalmanFunction(double kuan, double chang)
{
        this->kuan = kuan;
        this->chang = chang;
}

cv::Mat DrawKalmanFunction::DrawMyFunctionLine() // 画出坐标轴
{
        cv::Mat image(kuan, chang, CV_8UC3, cv::Scalar(255, 255, 255));

        std::string strV = "V";
        std::string strT = "V";
        // 坐标轴
        cv::line(image, cv::Point(25, kuan - 25), cv::Point(chang, kuan - 25), cv::Scalar(200, 200, 200), 2);
        cv::line(image, cv::Point(25, kuan - 25), cv::Point(25, 0), cv::Scalar(200, 200, 200), 2);

        cv::putText(image, "Green is observation", cv::Point(20, 40), cv::FONT_HERSHEY_COMPLEX, 0.75, cv::Scalar(0, 255, 0), 2);
        cv::putText(image, "RED is Kalman", cv::Point(20, 20), cv::FONT_HERSHEY_COMPLEX, 0.75, cv::Scalar(0, 0, 255), 2);

        return image;
}

cv::Point DrawKalmanFunction::DrawMyFxxkingPoint(cv::Mat &image, double time, double w) // 画Kalman预测结果
{
        int x = static_cast<int>(25 + 0.1*time);
        int y = static_cast<int>(kuan - (500 + w*20));

        cv::Point center(x, y);

        cv::circle(image, center, 3, cv::Scalar(0, 0, 255), -1);

        return center;
}

cv::Point DrawKalmanFunction::DrawMyFxxkingPoint(cv::Mat &image, double time, double w, double) // 画观测量结果
{

        int x = static_cast<int>(25 + 0.1*time);
        int y = static_cast<int>(kuan - (500 + w*20));

        cv::Point center(x, y);

        cv::circle(image, center, 3, cv::Scalar(0, 255, 0), -1);

        return center;
}
