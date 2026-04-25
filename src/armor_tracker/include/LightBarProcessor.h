#ifndef LIGHTBARPROCESSOR_H
#define LIGHTBARPROCESSOR_H

#include <opencv2/opencv.hpp>

class LightBarProcessor
{
public:
    void getLightBarEndpoints(const cv::RotatedRect &rect, cv::Point2f &top, cv::Point2f &bottom);
    
    void getLightBarInnerPoints(const cv::RotatedRect &rect, cv::Point2f &top, cv::Point2f &bottom, int sign);
};

struct LightBar
{
    cv::RotatedRect rect;
    double angle;
    cv::Point2f center;
    cv::Rect light_rect;
    int color;
};

#endif