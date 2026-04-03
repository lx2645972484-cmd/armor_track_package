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
        status = GXSendCommand(camera_handle_, GX_COMMAND_ACQUISITION_START);
        if (!GX_SUCCESS(status))
        {
            std::cerr << "[GalaxyCamera] Failed to start acquisition" << std::endl;
            return false;
        }

        std::cout << "[GalaxyCamera] Initialized successfully, max resolution = " << width_max_ << "x" << height_max_ << std::endl;
        return true;
    }

    void GalaxyCamera::startCapture()
    {
        if (running_)
            return;
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

    void GalaxyCamera::captureLoop()
    {
        GX_FRAME_DATA bayer_frame = {0};
        GX_STATUS status;
        std::vector<char> bayer_buffer_holder;

        // 获取负载大小，分配缓冲区
        int64_t payloadSize = 0;
        GXGetInt(camera_handle_, GX_INT_PAYLOAD_SIZE, &payloadSize);
        bayer_buffer_holder.reserve(payloadSize);
        bayer_frame.pImgBuf = bayer_buffer_holder.data();

        std::cout << "[GalaxyCamera] Capture thread started." << std::endl;

        while (running_)
        {
            // 获取一帧图像（超时 500ms）
            status = GXGetImage(camera_handle_, &bayer_frame, 500);
            if (GX_SUCCESS(status))
            {
                // 处理并转换为 RGB
                ImageData rgb_image;
                if (processFrame(bayer_frame, rgb_image))
                {
                    std::lock_guard<std::mutex> lock(image_mutex_);
                    latest_image_ = std::move(rgb_image);
                    has_new_image_ = true;
                }
                fail_count_ = 0;
            }
            else
            {
                // 获取失败，尝试停止并重新开始采集
                std::cerr << "[GalaxyCamera] Get image failed, status = " << status << std::endl;
                GXSendCommand(camera_handle_, GX_COMMAND_ACQUISITION_STOP);
                GXSendCommand(camera_handle_, GX_COMMAND_ACQUISITION_START);
                fail_count_++;
            }

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