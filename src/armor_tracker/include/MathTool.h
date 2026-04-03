#ifndef MATHTOOL
#define MATHTOOL

#include<cmath>
#include<opencv2/opencv.hpp>

class MathTool
{
public:
    float getMyDistance(cv::Point2f p1, cv::Point2f p2);
};

#endif