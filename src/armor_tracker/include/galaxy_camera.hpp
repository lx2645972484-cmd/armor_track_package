#ifndef GALAXY_CAMERA_HPP
#define GALAXY_CAMERA_HPP

#include "GxIAPI.h"
#include "DxImageProc.h"
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <string>

namespace galaxy_camera {

// 图像结构体，存储 RGB 图像数据
struct ImageData {
    uint32_t width;
    uint32_t height;
    std::vector<uint8_t> data;  // RGB8 格式，大小为 width * height * 3
    uint64_t timestamp_ms;      // 毫秒时间戳（可使用 std::chrono 获取）
};

class GalaxyCamera {
public:
    /**
     * @brief 构造函数
     * @param device_index 相机索引，从1开始
     * @param auto_start 是否自动开始采集线程
     */
    GalaxyCamera(uint32_t device_index = 1, bool auto_start = false);
    
    ~GalaxyCamera();

    /**
     * @brief 初始化相机（打开设备、获取参数、启动采集）
     * @return true 成功，false 失败
     */
    bool init();

    /**
     * @brief 开始采集线程（内部循环抓图）
     */
    void startCapture();

    /**
     * @brief 停止采集线程并关闭相机
     */
    void stop();

    /**
     * @brief 获取最新一帧图像（线程安全）
     * @param out_image 输出图像结构体
     * @return true 有有效图像，false 无新图像
     */
    bool getImage(ImageData& out_image);

    /**
     * @brief 设置曝光时间（微秒）
     * @param exposure_us 曝光时间，单位微秒
     * @return true 成功，false 失败
     */
    bool setExposureTime(double exposure_us);

    /**
     * @brief 获取当前曝光时间（微秒）
     * @return 曝光时间值，失败返回 -1
     */
    double getExposureTime();

    /**
     * @brief 设置增益
     * @param gain 增益值（范围取决于相机）
     * @return true 成功，false 失败
     */
    bool setGain(double gain);

    /**
     * @brief 获取当前增益
     * @return 增益值，失败返回 -1
     */
    double getGain();

    bool isReady() const;

    /**
     * @brief 获取相机传感器宽度（最大）
     */
    int64_t getWidthMax() const { return width_max_; }
    int64_t getHeightMax() const { return height_max_; }

private:
    // 内部采集线程函数
    void captureLoop();

    // 将 Bayer 帧转换为 RGB24 并存储
    bool processFrame(GX_FRAME_DATA& bayer_frame, ImageData& out_image);

    uint32_t device_index_;         // 相机索引
    GX_DEV_HANDLE camera_handle_;   // 相机句柄

    // 图像尺寸信息
    int64_t width_max_;
    int64_t height_max_;

    // 采集线程控制
    std::atomic<bool> running_;
    std::thread capture_thread_;

    // 最新图像存储（互斥保护）
    std::mutex image_mutex_;
    ImageData latest_image_;
    bool has_new_image_;            // 是否有未读取的新图像（可选）

    // 失败计数（用于检测相机异常）
    int fail_count_;
};

} // namespace galaxy_camera

#endif // GALAXY_CAMERA_HPP