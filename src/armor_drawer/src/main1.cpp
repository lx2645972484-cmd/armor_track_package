#include "DrawMyFunction.h"

#include <Eigen/Dense>
#include <opencv2/opencv.hpp>
#include <vector>
#include <cmath>
#include <deque>

#include "rclcpp/rclcpp.hpp"
#include "armor_interfaces/msg/armor_info.hpp"

using std::placeholders::_1;

class FunctionDrawer : public rclcpp::Node
{
public:
    FunctionDrawer() : Node("function_drawer")
    {
        fuc_subcription = this->create_subscription<armor_interfaces::msg::ArmorInfo>("armor_info", 10, std::bind(&FunctionDrawer::FunctionDraw,this,_1));
    }

    void FunctionDraw(armor_interfaces::msg::ArmorInfo armor_info)
    {
        imgFunc = drawer.DrawMyFunctionLine();
        drawer.DrawMyFxxkingPoint(imgFunc, armor_info.time, armor_info.yaw, 1.5);
        // cv::imshow("func",imgFunc);
        cv::waitKey(1);
    }

private:
    rclcpp::Subscription<armor_interfaces::msg::ArmorInfo>::SharedPtr fuc_subcription;
    cv::Mat imgFunc;
    DrawKalmanFunction drawer{1000, 3500};
};

int main(int argc,char * argv[])
{
    rclcpp::init(argc,argv);
    auto subcription = std::make_shared<FunctionDrawer>();
    rclcpp::spin(subcription);
    rclcpp::shutdown();
    return 0;
}