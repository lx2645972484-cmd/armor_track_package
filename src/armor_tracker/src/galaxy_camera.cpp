#include "galaxy_camera.hpp"
#include <chrono>
#include <iostream>
#include <cstring>

// 辅助宏，判断 GX API 是否成功
#define GX_SUCCESS(X) (X == GX_STATUS_SUCCESS)

namespace galaxy_camera
{

    GalaxyCamera::GalaxyCamera(uint32_t device_index, bool auto_start)
        : device_index_(device_index), camera_handle_(nullptr), width_max_(0), height_max_(0), running_(false), has_new_image_(false), fail_count_(0)
    {
        if (auto_start)
        {
            init();
            startCapture();
        }
    }

    GalaxyCamera::~GalaxyCamera()
    {
        stop();
    }

    bool GalaxyCamera::init()
    {
        GX_STATUS status;

        GXSetEnum(camera_handle_, GX_ENUM_TRIGGER_MODE, GX_TRIGGER_MODE_OFF);

        // 初始化 SDK
        status = GXInitLib();
        if (!GX_SUCCESS(status))
        {
            std::cerr << "[GalaxyCamera] GXInitLib failed, code = " << std::hex << status << std::dec << std::endl;
            return false;
        }

        // 尝试打开相机（持续重试）
        while (true)
        {
            uint32_t device_count = 0;
            status = GXUpdateDeviceList(&device_count, 100);
            if (device_count < 1)
            {
                std::cerr << "[GalaxyCamera] No camera found, device_count = " << device_count << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(1));
                continue;
            }
            status = GXOpenDeviceByIndex(device_index_, &camera_handle_);
            if (!GX_SUCCESS(status))
            {
                std::cerr << "[GalaxyCamera] Can not open camera, status = " << status << std::endl;
            }
            else
            {
                break;
            }
        }

        // 获取图像尺寸信息
        GXGetInt(camera_handle_, GX_INT_WIDTH_MAX, &width_max_);
        GXGetInt(camera_handle_, GX_INT_HEIGHT_MAX, &height_max_);

        // 设置默认参数（曝光时间、增益可通过后续接口修改）
        // 注意：这里可以调用一次 setExposureTime / setGain 来确保初始值合理，但为了简洁先不设置

        // 开始采集（发送命令）
        // status = GXSendCommand(camera_handle_, GX_COMMAND_ACQUISITION_START);
        // if (!GX_SUCCESS(status))
        // {
        //     std::cerr << "[GalaxyCamera] Failed to start acquisition" << std::endl;
        //     return false;
        // }

        ///////////////////////////////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////////////////////////////

        //ATTENTION!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!这里改变了相机开流指令！！！！！！！！！！！！

        //可以按需要修改！！！！

        std::cout << "[GalaxyCamera] Initialized successfully, max resolution = " << width_max_ << "x" << height_max_ << std::endl;
        return true;
    }

    void GalaxyCamera::startCapture()
    {
        if (running_)
            return;

        //调整部分
        GX_STATUS status = GXSendCommand(camera_handle_, GX_COMMAND_ACQUISITION_START);
        if (!GX_SUCCESS(status))
        {
            std::cerr << "[GalaxyCamera] Failed to start acquisition" << std::endl;
            return;
        }

        running_ = true;
        capture_thread_ = std::thread(&GalaxyCamera::captureLoop, this);
    }

    void GalaxyCamera::stop()
    {
        if (!running_)
            return;
        running_ = false;
        if (capture_thread_.joinable())
        {
            capture_thread_.join();
        }
        if (camera_handle_)
        {
            GXSendCommand(camera_handle_, GX_COMMAND_ACQUISITION_STOP);
            GXCloseDevice(camera_handle_);
            camera_handle_ = nullptr;
        }
        GXCloseLib();
        std::cout << "[GalaxyCamera] Stopped and closed." << std::endl;
    }

    bool GalaxyCamera::getImage(ImageData &out_image)
    {
        std::lock_guard<std::mutex> lock(image_mutex_);
        if (!has_new_image_)
            return false;
        out_image = latest_image_; // 拷贝图像数据
        has_new_image_ = false;    // 可选：标记已读取
        return true;
    }

    bool GalaxyCamera::setExposureTime(double exposure_us)
    {
        if (!camera_handle_)
            return false;
        GX_STATUS status = GXSetFloat(camera_handle_, GX_FLOAT_EXPOSURE_TIME, exposure_us);
        if (!GX_SUCCESS(status))
        {
            std::cerr << "[GalaxyCamera] Failed to set exposure time, status = " << status << std::endl;
            return false;
        }
        return true;
    }

    double GalaxyCamera::getExposureTime()
    {
        if (!camera_handle_)
            return -1;
        double value;
        GX_STATUS status = GXGetFloat(camera_handle_, GX_FLOAT_EXPOSURE_TIME, &value);
        if (!GX_SUCCESS(status))
            return -1;
        return value;
    }

    bool GalaxyCamera::setGain(double gain)
    {
        if (!camera_handle_)
            return false;
        GX_STATUS status = GXSetFloat(camera_handle_, GX_FLOAT_GAIN, gain);
        if (!GX_SUCCESS(status))
        {
            std::cerr << "[GalaxyCamera] Failed to set gain, status = " << status << std::endl;
            return false;
        }
        return true;
    }

    double GalaxyCamera::getGain()
    {
        if (!camera_handle_)
            return -1;
        double value;
        GX_STATUS status = GXGetFloat(camera_handle_, GX_FLOAT_GAIN, &value);
        if (!GX_SUCCESS(status))
            return -1;
        return value;
    }

    bool GalaxyCamera::setGamma(double gamma_value)
    {
        if (!camera_handle_)
            return false;

        // 1. 必须先开启 Gamma 使能 (Gamma Enable)
        // 注意：大恒部分较老的型号或特定固件可能是 GX_ENUM_GAMMA_MODE，
        // 但目前绝大多数支持 GxIAPI 的型号都使用 GX_BOOL_GAMMA_ENABLE
        GX_STATUS status_enable = GXSetBool(camera_handle_, GX_BOOL_GAMMA_ENABLE, true);
        if (!GX_SUCCESS(status_enable))
        {
            std::cerr << "[GalaxyCamera] Failed to enable Gamma (maybe already on or not supported), status = " << status_enable << std::endl;
            // 我们不 return false，因为有些相机默认开启且只读使能位，直接尝试去设值
        }

        // 2. 写入具体的 Gamma 值
        GX_STATUS status = GXSetFloat(camera_handle_, GX_FLOAT_GAMMA, gamma_value);
        if (!GX_SUCCESS(status))
        {
            std::cerr << "[GalaxyCamera] Failed to set Gamma, status = " << status << std::endl;
            return false;
        }
        
        return true;
    }

    double GalaxyCamera::getGamma()
    {
        if (!camera_handle_)
            return -1;
            
        double value;
        GX_STATUS status = GXGetFloat(camera_handle_, GX_FLOAT_GAMMA, &value);
        if (!GX_SUCCESS(status))
            return -1;
            
        return value;
    }

    void GalaxyCamera::captureLoop()
    {
        GX_FRAME_DATA bayer_frame = {0};
        GX_STATUS status;
        std::vector<char> bayer_buffer_holder;

        // 获取负载大小，分配缓冲区
        int64_t payloadSize = 0;
        GXGetInt(camera_handle_, GX_INT_PAYLOAD_SIZE, &payloadSize);
        bayer_buffer_holder.resize(payloadSize);
        bayer_frame.pImgBuf = bayer_buffer_holder.data();

        std::cout << "[GalaxyCamera] Capture thread started." << std::endl;

        while (running_)
        {
            // 获取一帧图像（超时 500ms）
            try
            {
                status = GXGetImage(camera_handle_, &bayer_frame, 500);
                // 处理并转换为 RGB
                // ImageData rgb_image;
                // if (processFrame(bayer_frame, rgb_image))
                // {
                //     std::lock_guard<std::mutex> lock(image_mutex_);
                //     latest_image_ = std::move(rgb_image);
                //     has_new_image_ = true;
                // }
                // fail_count_ = 0;

                // if (status == GX_STATUS_SUCCESS && bayer_frame.nStatus == GX_FRAME_STATUS_SUCCESS)
                // {
                //     ImageData rgb_image;
                //     if (processFrame(bayer_frame, rgb_image))
                //     {
                //         std::lock_guard<std::mutex> lock(image_mutex_);
                //         latest_image_ = std::move(rgb_image);
                //         has_new_image_ = true;
                //     }
                //     fail_count_ = 0; // 只有在真正成功获取并处理后，才重置错误计数
                // }
                // else
                // {
                //     // 【取消注释并优化】：触发掉线恢复机制
                //     std::cerr << "[GalaxyCamera] Get image failed or frame incomplete. Status: " << status
                //               << " FrameStatus: " << bayer_frame.nStatus << std::endl;

                //     // 尝试重启采集流
                //     GXSendCommand(camera_handle_, GX_COMMAND_ACQUISITION_STOP);
                //     std::this_thread::sleep_for(std::chrono::milliseconds(10)); // 给硬件一点缓冲时间
                //     GXSendCommand(camera_handle_, GX_COMMAND_ACQUISITION_START);

                //     // fail_count_++;
                // }

                if (status == GX_STATUS_SUCCESS && bayer_frame.nStatus == GX_FRAME_STATUS_SUCCESS)
                {
                    ImageData rgb_image;
                    if (processFrame(bayer_frame, rgb_image))
                    {
                        std::lock_guard<std::mutex> lock(image_mutex_);
                        latest_image_ = std::move(rgb_image);
                        has_new_image_ = true;
                    }
                    fail_count_ = 0; // 只有在真正成功获取并处理后，才重置错误计数
                }
                else
                {
                    std::cerr << "[GalaxyCamera] Get image failed or frame incomplete. Status: " << status
                              << " FrameStatus: " << bayer_frame.nStatus << std::endl;

                    // 【核心修复】：如果是超时 (通常 -14 代表 GX_STATUS_TIMEOUT)，不要重启数据流，继续等待下一帧即可
                    if (status == -14 || status == GX_STATUS_TIMEOUT) 
                    {
                        // 仅做日志打印，直接 continue，不累加 fail_count_，不重启相机
                        continue; 
                    }

                    fail_count_++;

                    // 只有当连续多次出现严重错误时，才尝试重启数据流
                    if (fail_count_ >= 3 && fail_count_ <= 5)
                    {
                        std::cerr << "[GalaxyCamera] Attempting to restart acquisition stream..." << std::endl;
                        GXSendCommand(camera_handle_, GX_COMMAND_ACQUISITION_STOP);
                        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // 增加缓冲时间至 100ms
                        GXSendCommand(camera_handle_, GX_COMMAND_ACQUISITION_START);
                    }
                }
            }
            catch (const std::exception &e)
            {
                std::cerr << "[GalaxyCamera] Exception in captureLoop: " << e.what() << std::endl;
                running_ = false;
                break;
            }
            catch (...)
            {
                std::cerr << "[GalaxyCamera] Unknown exception in captureLoop" << std::endl;
                running_ = false;
                break;
            }
            // else
            // {
            //     // 获取失败，尝试停止并重新开始采集
            //     std::cerr << "[GalaxyCamera] Get image failed, status = " << status << std::endl;
            //     GXSendCommand(camera_handle_, GX_COMMAND_ACQUISITION_STOP);
            //     GXSendCommand(camera_handle_, GX_COMMAND_ACQUISITION_START);
            //     fail_count_++;
            // }

            if (fail_count_ > 5)
            {
                std::cerr << "[GalaxyCamera] Too many failures, stopping capture." << std::endl;
                running_ = false;
                break;
            }
        }
        std::cout << "[GalaxyCamera] Capture thread exited." << std::endl;
    }

    bool GalaxyCamera::processFrame(GX_FRAME_DATA &bayer_frame, ImageData &out_image)
    {
        // 确定 Bayer 格式类型
        DX_PIXEL_COLOR_FILTER bayer_type;
        switch (bayer_frame.nPixelFormat)
        {
        case GX_PIXEL_FORMAT_BAYER_GR8:
            bayer_type = BAYERGR;
            break;
        case GX_PIXEL_FORMAT_BAYER_RG8:
            bayer_type = BAYERRG;
            break;
        case GX_PIXEL_FORMAT_BAYER_GB8:
            bayer_type = BAYERGB;
            break;
        case GX_PIXEL_FORMAT_BAYER_BG8:
            bayer_type = BAYERBG;
            break;
        default:
            std::cerr << "[GalaxyCamera] Unsupported Bayer format: " << bayer_frame.nPixelFormat << std::endl;
            return false;
        }

        out_image.width = bayer_frame.nWidth;
        out_image.height = bayer_frame.nHeight;
        out_image.data.resize(out_image.width * out_image.height * 3);
        // 记录时间戳（毫秒）
        auto now = std::chrono::system_clock::now();
        out_image.timestamp_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();

        // Bayer -> RGB24
        GX_STATUS status = DxRaw8toRGB24(bayer_frame.pImgBuf, out_image.data.data(),
                                         bayer_frame.nWidth, bayer_frame.nHeight,
                                         RAW2RGB_NEIGHBOUR, bayer_type, false);
        if (!GX_SUCCESS(status))
        {
            std::cerr << "[GalaxyCamera] Bayer to RGB conversion failed, status = " << status << std::endl;
            return false;
        }
        return true;
    }

    bool GalaxyCamera::isReady() const
    {
        return (camera_handle_ != nullptr) && running_.load();
    }

} // namespace galaxy_camera