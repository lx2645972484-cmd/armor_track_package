#include "Vedio.h"
#include <cstdio>     // 用于 printf（调试用）

// =============================================
// 获取相机视频信息（分辨率、帧率、当前时间）
// =============================================
void VedioProcessor::getVideoinformation(CameraHandle hCamera,
                                         int &frame_width,
                                         int &frame_height,
                                         double &fps,
                                         double &time_ms)
{
    // 1. 获取当前分辨率
    tSdkImageResolution sCurVideoSize = {0};
    CameraGetImageResolution(hCamera, &sCurVideoSize);

    frame_width  = sCurVideoSize.iWidth;
    frame_height = sCurVideoSize.iHeight;

    // 2. 获取当前帧率（Hz）
    int RateHZ = 0;
    CameraGetFrameRate(hCamera, &RateHZ);
    fps = (RateHZ > 0) ? RateHZ : 0.0;

    // 3. 获取当前帧的时间戳（转换为毫秒）
    UINT tsLow = 0, tsHigh = 0;
    CameraGetFrameTimeStamp(hCamera, &tsLow, &tsHigh);
    uint64_t currentUs = ((uint64_t)tsHigh << 32) | tsLow;
    time_ms = currentUs / 1000.0;
}

// =============================================
// 处理每一帧（计算从第一帧开始的运行时间）
// =============================================
void VedioProcessor::ProcessFrame(CameraHandle hCamera)
{
    UINT tsLow = 0, tsHigh = 0;
    CameraGetFrameTimeStamp(hCamera, &tsLow, &tsHigh);

    uint64_t currentUs = ((uint64_t)tsHigh << 32) | tsLow;

    // 第一次调用时记录基准时间戳
    if (!g_hasBaseTimestamp)
    {
        g_baseTimestampUs = currentUs;
        g_hasBaseTimestamp = true;
        printf("【基准时间戳已记录】%llu us\n", g_baseTimestampUs);
        return;                     // 第一帧运行时间为 0
    }

    // 计算从第一帧到现在经过的时间
    uint64_t runtimeUs = currentUs - g_baseTimestampUs;
    double   runtimeSec = runtimeUs / 1000000.0;

    printf("当前帧时间戳: %llu us | 运行时间: %.3f 秒\n", currentUs, runtimeSec);
}