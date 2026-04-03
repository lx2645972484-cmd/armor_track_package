#include "Preprocessing.h"

cv::Mat PreProcess::PreprocessImg(cv::Mat img, int blue_value, int red_value, int setting)
{
    cv::Mat subimg, mask, imgClose;
    // std::vector<cv::Mat> channels;
    // cv::split(img, channels);
    // cv::subtract(channels[0], channels[2], subimg);
    // cv::threshold(subimg, mask, value, 255, cv::THRESH_BINARY);

    // 分离BGR通道
    std::vector<cv::Mat> channels;
    cv::split(img, channels); // channels[0]=B, channels[1]=G, channels[2]=R

    // 蓝色掩码: B - R > blueThresh
    cv::Mat blueDiff, blueMask;
    cv::subtract(channels[0], channels[2], blueDiff);
    cv::threshold(blueDiff, blueMask, blue_value, 255, cv::THRESH_BINARY);

    // 红色掩码: R - B > redThresh
    cv::Mat redDiff, redMask;
    cv::subtract(channels[2], channels[0], redDiff);
    cv::threshold(redDiff, redMask, red_value, 255, cv::THRESH_BINARY);

    // 合并两个掩码（逻辑或）
    // cv::Mat mask;
    // cv::bitwise_or(blueMask, redMask, mask);

    // // 可选：闭运算去除小孔洞
    // cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
    // cv::morphologyEx(mask, mask, cv::MORPH_CLOSE, kernel);

    // return mask;
    if (setting == 0)
    {
        cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));
        cv::morphologyEx(blueMask, imgClose, cv::MORPH_OPEN, kernel);
    }

    if (setting == 2)
    {
        cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));
        cv::morphologyEx(redMask, imgClose, cv::MORPH_OPEN, kernel);
    }

    return imgClose;
}