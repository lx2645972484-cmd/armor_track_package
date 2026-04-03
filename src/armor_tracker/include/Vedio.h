#ifndef VEDIO_H
#define VEDIO_H

#include "CameraApi.h"   // 迈德相机 SDK 头文件
#include <cstdint>     // C++11 及以后推荐使用

class VedioProcessor
{
public:
    // 获取相机当前的分辨率、帧率、以及当前帧时间（毫秒）
    void getVideoinformation(CameraHandle hCamera, 
                             int &frame_width, 
                             int &frame_height, 
                             double &fps, 
                             double &time_ms);

    // 在取图回调或取图线程中调用，处理每一帧（计算运行时间）
    void ProcessFrame(CameraHandle hCamera);

private:
    // 全局变量记录基准时间戳（第一帧的时间）
    uint64_t g_baseTimestampUs = 0;
    bool     g_hasBaseTimestamp = false;
};

#endif