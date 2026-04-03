#include "MathTool.h"

float MathTool::getMyDistance(cv::Point2f p1, cv::Point2f p2)
{
    return sqrt(pow(p1.x - p2.x, 2) + pow(p1.y - p2.y, 2));
}