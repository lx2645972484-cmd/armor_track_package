#include "LightBarProcessor.h"

bool compareByY(const cv::Point2f &a, const cv::Point2f &b)
{
    return a.y < b.y;
}

void LightBarProcessor::getLightBarEndpoints(const cv::RotatedRect &rect, cv::Point2f &top, cv::Point2f &bottom)
{
    cv::Point2f pts[4];
    rect.points(pts);

    // 以Y升序排列四个点
    std::sort(pts, pts + 4, compareByY);

    // 取上面两个点的平均值作为上端点中心
    top = (pts[0] + pts[1]) / 2.0f;
    // 取下面两个点的平均值作为下端点中心
    bottom = (pts[2] + pts[3]) / 2.0f;
}

