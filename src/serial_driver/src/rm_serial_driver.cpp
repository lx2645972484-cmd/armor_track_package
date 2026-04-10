/*
  安装依赖
  sudo apt install ros-humble-serial-driver
  以下仅供参考
*/

// ROS
#include <rclcpp/logging.hpp>
#include <rclcpp/qos.hpp>
#include <rclcpp/utilities.hpp>
#include <serial_driver/serial_driver.hpp>
// #include <armor_interfaces/msg/serial_receive_data.hpp>

// C++ system
#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "armor_interfaces/msg/serial_driver.hpp"
#include "rm_serial_driver/crc.hpp"
#include "rm_serial_driver/packet.hpp"
#include "rm_serial_driver/rm_serial_driver.hpp"

// flow
// reopenPort

namespace rm_serial_driver
{
#pragma pack(push, 1)
    typedef struct
    {
        uint8_t Frame_Header;   // 0xAA
        uint8_t Mode;           // 0: 哨兵模式, 1: 自瞄模式
        float Delta_Yaw_10;   // 单位 0.1°，范围 -3276.8° ~ 3276.7°
        float Delta_Pitch_10; // 单位 0.1°
        uint8_t Frame_Tail;     // 0x55
    } Struct_Camera_USB_Frame;
#pragma pack(pop)

    typedef union
    {
        Struct_Camera_USB_Frame Data;
        uint8_t Raw[sizeof(Struct_Camera_USB_Frame)];
    } Union_Camera_USB_Frame;

    void float_to_5digit_array(float num, uint8_t out[5], int symbol);

    RMSerialDriver::RMSerialDriver(const rclcpp::NodeOptions &options)
        : Node("serial_driver", options),
          owned_ctx_{new IoContext(2)},
          serial_driver_{new drivers::serial_driver::SerialDriver(*owned_ctx_)}
    {
        RCLCPP_INFO(get_logger(), "Start SerialDriver!");

        getParams();

        // Create Subscription
        //  参考
        target_sub_ = this->create_subscription<armor_interfaces::msg::SerialDriver>(
            "/tracker/target", rclcpp::SensorDataQoS(),
            std::bind(&RMSerialDriver::sendData, this, std::placeholders::_1));

        receive_data_publisher_ = this->create_publisher<armor_interfaces::msg::SerialReceiveData>("/tracker/receive_data", rclcpp::SensorDataQoS());

        try
        {
            serial_driver_->init_port(device_name_, *device_config_);
            if (!serial_driver_->port()->is_open())
            {
                serial_driver_->port()->open();
                int fd = ::open(device_name_.c_str(), O_RDWR | O_NOCTTY);
                if (fd < 0)
                {
                    // 打开失败，处理错误
                    perror("open serial port");
                }

                // 现在你可以使用 fd 进行读写、配置或清空缓冲区
                tcflush(fd, TCIOFLUSH); // 清空输入输出缓冲区

                // 使用完后记得关闭
                close(fd);
                receive_thread_ = std::thread(&RMSerialDriver::receiveData, this);
            }
        }
        catch (const std::exception &ex)
        {
            RCLCPP_ERROR(
                get_logger(), "Error creating serial port: %s - %s", device_name_.c_str(), ex.what());
            throw ex;
        }
    }

    RMSerialDriver::~RMSerialDriver()
    {
        if (receive_thread_.joinable())
        {
            receive_thread_.join();
        }

        if (serial_driver_->port()->is_open())
        {
            serial_driver_->port()->close();
        }

        if (owned_ctx_)
        {
            owned_ctx_->waitForExit();
        }
    }

    void RMSerialDriver::receiveData()
    {
        Union_Camera_USB_Frame Frame;
        
        std::vector<uint8_t> data(sizeof(Struct_Camera_USB_Frame));
        // std::vector<uint8_t> data;
        // data.reserve(sizeof(ReceivePacket));

        while (rclcpp::ok())
        {
            try
            {
                serial_driver_->port()->receive(data);

                // RCLCPP_INFO(get_logger(), "Received header: 0x%02X", header[0]);

                // Invalid header: 0x00
                //  if (data[0] == 0xAA)
                //  {
                //     RCLCPP_INFO(get_logger(), "Received header: 0x%02X", data[0]);
                //  }
                //  else
                //  {
                //      RCLCPP_WARN_THROTTLE(get_logger(), *get_clock(), 20, "Invalid header: 0x%02X", data[0]);
                //      continue; // 跳过本次循环，等待下一个数据包
                //  }
                
                
                // serial_driver_->port()->receive(data);

                // if(data.back() != 0x55)
                // {
                //     RCLCPP_WARN_THROTTLE(get_logger(), *get_clock(), 20, "Invalid tail: 0x%02X", data.back());
                //     continue; // 跳过本次循环，等待下一个数据包
                // }

                // data.insert(data.begin(), header[0]);
                for(int i = 0; i < data.size(); i++)
                {
                    Frame.Raw[i] = data[i];
                }

                // for(int i = 0; i < sizeof(Frame.Raw); i++)
                // {
                //     RCLCPP_INFO(get_logger(), "Received data[%d]: 0x%02X", i, Frame.Raw[i]);
                // }

                double yaw = Frame.Data.Delta_Yaw_10 ;
                double pitch = Frame.Data.Delta_Pitch_10 ;

                // RCLCPP_INFO(get_logger(), "Received yaw: %f, pitch: %f", yaw, pitch);

                armor_interfaces::msg::SerialReceiveData::SharedPtr msg = std::make_shared<armor_interfaces::msg::SerialReceiveData>();
                msg->yaw = yaw;
                msg->pitch = pitch;

                receive_data_publisher_->publish(*msg);

                // if (packet.checksum == 0xFE)
                // {

                // }
                // else
                // {
                //   RCLCPP_ERROR(get_logger(), "0xFE error!");
                // }

                // 调试选项
                //  RCLCPP_INFO(get_logger(), "接收数据: ");
                //  for(uint8_t byte : data)
                //  {
                //      RCLCPP_INFO(get_logger(), "接收数据0x%02X", byte);
                //  }

                // // 处理数据
                // int first_int = data[3] * 1000 + data[4] * 100 + data[5] * 10 + data[6];
                // double first = first_int / 10.0;

                // // 后5个数字组成第二个数：10514 -> 10514 / 100 = 105.14
                // int second_int = data[8] * 1000 + data[9] * 100 + data[10] * 10 + data[11] * 1;
                // double second = second_int / 10.0;

                // printf("第一个数: %.2f\n", first);
                // printf("第二个数: %.2f\n", second);
                // }
                // else
                // {
                // RCLCPP_WARN_THROTTLE(get_logger(), *get_clock(), 20, "Invalid header: %02X", header[0]);
                // }
            }
            catch (const std::exception &ex)
            {
                {
                    int fd = ::open(device_name_.c_str(), O_RDWR | O_NOCTTY);
                    if (fd < 0)
                    {
                        // 打开失败，处理错误
                        perror("open serial port");
                    }

                    // 现在你可以使用 fd 进行读写、配置或清空缓冲区
                    tcflush(fd, TCIOFLUSH); // 清空输入输出缓冲区

                    // 使用完后记得关闭
                    close(fd);
                }

                RCLCPP_ERROR_THROTTLE(
                    get_logger(), *get_clock(), 20, "Error while receiving data: %s", ex.what());
                reopenPort();
            }
        }
    }

    void RMSerialDriver::sendData(armor_interfaces::msg::SerialDriver::SharedPtr msg)
    {

        try
        {
            Union_Camera_USB_Frame frame;
            frame.Data.Frame_Header = 0xAA;
            frame.Data.Mode = msg->symbol;
            frame.Data.Delta_Yaw_10 = msg->yaw;   // 将yaw转换为0.1度单位
            frame.Data.Delta_Pitch_10 = msg->pitch; // 将pitch转换为0.1度单位
            frame.Data.Frame_Tail = 0x55;
            
            // SendPacket packet;
            // 处理发送数据
            // uint8_t yaw_data[5];
            // uint8_t pitch_data[5];

            double yaw;
            double pitch;

            // yaw = msg->yaw;
            // pitch = msg->pitch;
            // // int yaw_symbol = msg->yaw_symbol;
            // // int pitch_symbol = msg->pitch_symbol;

            // float_to_5digit_array(yaw, yaw_data, yaw_symbol);
            // float_to_5digit_array(pitch, pitch_data, pitch_symbol);

            // std::vector<uint8_t> data(13);
            // data[0] = 0xAA;
            // data[1] = 1;
            // data[2] = yaw_symbol;
            // data[7] = pitch_symbol;
            // data[12] = 0x55;

            // for (int i = 3; i <= 6; i++)
            // {
            //     data[i] = yaw_data[i - 3];
            // }

            // for (int i = 8; i <= 11; i++)
            // {
            //     data[i] = pitch_data[i - 8];
            // }

            // for (int i = 0; i < 13; i++)
            // {
            //     RCLCPP_INFO(get_logger(), "data[%d]发送数据: %d", i, data[i]);
            // }

            // std::vector<uint8_t> a;
            // a.push_back(1);

            // std::vector<uint8_t> data = toVector(packet);
            std::vector<uint8_t> data(frame.Raw, frame.Raw + sizeof(frame));
            serial_driver_->port()->send(data);

            // 调试选项
            // RCLCPP_INFO(get_logger(), "发送数据: ");
            // for(uint8_t byte : data)
            // {
            //     RCLCPP_INFO(get_logger(), "发送数据:%d", byte);
            // }
        }
        catch (const std::exception &ex)
        {
            RCLCPP_ERROR(get_logger(), "Error while sending data: %s", ex.what());
            reopenPort();
        }
    }

    void RMSerialDriver::getParams()
    {
        using FlowControl = drivers::serial_driver::FlowControl;
        using Parity = drivers::serial_driver::Parity;
        using StopBits = drivers::serial_driver::StopBits;

        uint32_t baud_rate{};
        auto fc = FlowControl::NONE;
        auto pt = Parity::NONE;
        auto sb = StopBits::ONE;

        try
        {
            device_name_ = declare_parameter<std::string>("device_name", "");
        }
        catch (rclcpp::ParameterTypeException &ex)
        {
            RCLCPP_ERROR(get_logger(), "The device name provided was invalid");
            throw ex;
        }

        try
        {
            baud_rate = declare_parameter<int>("baud_rate", 0);
        }
        catch (rclcpp::ParameterTypeException &ex)
        {
            RCLCPP_ERROR(get_logger(), "The baud_rate provided was invalid");
            throw ex;
        }

        try
        {
            const auto fc_string = declare_parameter<std::string>("flow_control", "");

            if (fc_string == "none")
            {
                fc = FlowControl::NONE;
            }
            else if (fc_string == "hardware")
            {
                fc = FlowControl::HARDWARE;
            }
            else if (fc_string == "software")
            {
                fc = FlowControl::SOFTWARE;
            }
            else
            {
                throw std::invalid_argument{
                    "The flow_control parameter must be one of: none, software, or hardware."};
            }
        }
        catch (rclcpp::ParameterTypeException &ex)
        {
            RCLCPP_ERROR(get_logger(), "The flow_control provided was invalid");
            throw ex;
        }

        try
        {
            const auto pt_string = declare_parameter<std::string>("parity", "");

            if (pt_string == "none")
            {
                pt = Parity::NONE;
            }
            else if (pt_string == "odd")
            {
                pt = Parity::ODD;
            }
            else if (pt_string == "even")
            {
                pt = Parity::EVEN;
            }
            else
            {
                throw std::invalid_argument{"The parity parameter must be one of: none, odd, or even."};
            }
        }

        catch (rclcpp::ParameterTypeException &ex)
        {
            RCLCPP_ERROR(get_logger(), "The parity provided was invalid");
            throw ex;
        }

        try
        {
            const auto sb_string = declare_parameter<std::string>("stop_bits", "");

            if (sb_string == "1" || sb_string == "1.0")
            {
                sb = StopBits::ONE;
            }
            else if (sb_string == "1.5")
            {
                sb = StopBits::ONE_POINT_FIVE;
            }
            else if (sb_string == "2" || sb_string == "2.0")
            {
                sb = StopBits::TWO;
            }
            else
            {
                throw std::invalid_argument{"The stop_bits parameter must be one of: 1, 1.5, or 2."};
            }
        }
        catch (rclcpp::ParameterTypeException &ex)
        {
            RCLCPP_ERROR(get_logger(), "The stop_bits provided was invalid");
            throw ex;
        }

        device_config_ =
            std::make_unique<drivers::serial_driver::SerialPortConfig>(baud_rate, fc, pt, sb);
    }

    void RMSerialDriver::reopenPort()
    {
        RCLCPP_WARN(get_logger(), "Attempting to reopen port");
        try
        {
            if (serial_driver_->port()->is_open())
            {
                serial_driver_->port()->close();
            }
            serial_driver_->port()->open();
            RCLCPP_INFO(get_logger(), "Successfully reopened port");
        }
        catch (const std::exception &ex)
        {
            RCLCPP_ERROR(get_logger(), "Error while reopening port: %s", ex.what());

            int fd = ::open(device_name_.c_str(), O_RDWR | O_NOCTTY);
            if (fd < 0)
            {
                // 打开失败，处理错误
                perror("open serial port");
            }

            // 现在你可以使用 fd 进行读写、配置或清空缓冲区
            tcflush(fd, TCIOFLUSH); // 清空输入输出缓冲区

            // 使用完后记得关闭
            close(fd);
            if (rclcpp::ok())
            {
                rclcpp::sleep_for(std::chrono::seconds(1));
                reopenPort();
            }
        }
    }

    void float_to_5digit_array(float num, uint8_t out[5], int symbol)
    {
        // 乘以 100 并四舍五入取整，避免浮点误差（如 2.35*100 可能得到 234.9999）
        int value = (int)roundf(num * 10.0f);

        // 从高位到低位提取数字，共 5 位
        out[0] = symbol;
        out[1] = (value / 1000) % 10;
        out[2] = (value / 100) % 10;
        out[3] = (value / 10) % 10;
        out[4] = (value / 1) % 10;
    }

} // namespace rm_serial_driver

namespace rm_serial_driver
{
    // ... 类定义内部 ...

    /**
     * @brief 业务主入口：启动串口通信链路
     *
     * 业务流：
     * 1. 打开物理串口
     * 2. 启动接收线程 (监听下位机反馈)
     * 3. 此时订阅者(/tracker/target)已就绪，等待识别模块数据
     */
    void RMSerialDriver::run()
    {
        RCLCPP_INFO(get_logger(), "=================================");
        RCLCPP_INFO(get_logger(), "  RMSerialDriver: Starting...");
        RCLCPP_INFO(get_logger(), "=================================");

        // 1. 确保串口物理连接已建立
        // 虽然构造函数里尝试过，但在这里做最后的确认更稳健
        if (!serial_driver_->port()->is_open())
        {
            RCLCPP_WARN(get_logger(), "Port is closed. Attempting to open...");
            try
            {
                serial_driver_->port()->open();
                RCLCPP_INFO(get_logger(), "Serial Port Opened Successfully.");
            }
            catch (const std::exception &ex)
            {
                RCLCPP_ERROR(get_logger(), "CRITICAL: Failed to open port: %s", ex.what());
                return; // 启动失败，不再继续
            }
        }

        // 2. 启动接收线程 (处理下位机反馈)
        // 使用 std::thread 将其放入后台，避免阻塞主线程
        if (!receive_thread_.joinable())
        {
            receive_thread_ = std::thread(&RMSerialDriver::receiveData, this);
            RCLCPP_INFO(get_logger(), "Receive Thread Started (Waiting for feedback)...");
        }

        RCLCPP_INFO(get_logger(), "System Ready. Waiting for Yaw/Pitch topics...");
    }

    // ...
}

#include "rclcpp_components/register_node_macro.hpp"

// Register the component with class_loader.
// This acts as a sort of entry point, allowing the component to be discoverable when its library
// is being loaded into a running process.
RCLCPP_COMPONENTS_REGISTER_NODE(rm_serial_driver::RMSerialDriver)