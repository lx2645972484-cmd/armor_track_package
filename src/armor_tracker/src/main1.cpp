// #include "armor_camera_capture.hpp"
// #include "ArmorTracker.hpp"

// int main(int argc, char *argv[])
// {
//     rclcpp::init(argc, argv);
//     auto tracker = std::make_shared<ArmorTracker>();
//     std::thread run_thread([&tracker]() {
//         tracker->run();
//     });
//     run_thread.detach(); // 分离线程，让它在后台跑
//     rclcpp::spin(tracker);
//     rclcpp::shutdown();
//     return 0;
// }

#include "armor_camera_capture.hpp"
#include "ArmorTracker.hpp"
#include <thread>

int main(int argc, char *argv[])
{
    rclcpp::init(argc, argv);
    auto tracker = std::make_shared<ArmorTracker>();
    
    // 1. 让 ROS 2 的回调函数（如串口接收）在后台线程运行
    std::thread spin_thread([&tracker]() {
        rclcpp::spin(tracker);
    });
    
    // 2. 图像处理和 OpenCV 绘图必须在主线程运行！
    tracker->run();
    
    // 3. 退出时的清理逻辑
    if (spin_thread.joinable()) {
        spin_thread.join();
    }
    rclcpp::shutdown();
    return 0;
}