#ifndef MATHTOOL
#define MATHTOOL

#include <cmath>
#include <opencv2/opencv.hpp>
#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/point_stamped.hpp"

class MathTool
{
public:
    float getMyDistance(cv::Point2f p1, cv::Point2f p2);

    // cv::Point2f& tl,cv::Point2f& tr,cv::Point2f& bl,cv::Point2f& br

    // 讲ROS坐标系转化为OpenCV坐标系，改变xyz的朝向
    void axis_turn_ros_to_opencv(double &x, double &y, double &z, geometry_msgs::msg::PointStamped ros2_test);
    

    void axis_turn_opencv_to_ros(double &ros_x, double &ros_y, double &ros_z, double cv_x, double cv_y, double cv_z);
   

    void axis_turn_opencv_to_ros(double &ros_x, double &ros_y, double &ros_z, geometry_msgs::msg::PointStamped opencv_test);
    

    void axis_turn_ros_to_opencv(double &cv_x, double &cv_y, double &cv_z, double ros_x, double ros_y, double ros_z);
    

    std::unique_ptr<cv::Point2f[]> armor_vector_extend(cv::Point2f &tl, cv::Point2f &tr, cv::Point2f &bl, cv::Point2f &br);
    
};

#endif