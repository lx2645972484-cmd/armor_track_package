#ifndef PREPROCESSING_H
#define PREPROCESSING_H

#include <opencv2/opencv.hpp>

class PreProcess
{
public:
    cv::Mat PreprocessImg(cv::Mat img, int blue_value, int red_value, int setting);
};

#endif