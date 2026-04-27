#include "MathTool.h"

float MathTool::getMyDistance(cv::Point2f p1, cv::Point2f p2)
{
    return sqrt(pow(p1.x - p2.x, 2) + pow(p1.y - p2.y, 2));
}

void MathTool::axis_turn_ros_to_opencv(double &x, double &y, double &z, geometry_msgs::msg::PointStamped ros2_test)
{
    x = -ros2_test.point.y; // OpenCV的右 = ROS左的反向
    y = -ros2_test.point.z; // OpenCV的下 = ROS上的反向
    z = ros2_test.point.x;  // OpenCV的前 = ROS的前
}

void MathTool::axis_turn_opencv_to_ros(double &ros_x, double &ros_y, double &ros_z, double cv_x, double cv_y, double cv_z)
{
    ros_x = cv_z;  // ROS的前 = OpenCV的前
    ros_y = -cv_x; // ROS的左 = OpenCV右的反向
    ros_z = -cv_y; // ROS的上 = OpenCV下的反向
}

void MathTool::axis_turn_opencv_to_ros(double &ros_x, double &ros_y, double &ros_z, geometry_msgs::msg::PointStamped opencv_test)
{
    ros_x = opencv_test.point.z;  // ROS的前 = OpenCV的前
    ros_y = -opencv_test.point.x; // ROS的左 = OpenCV右的反向 -x
    ros_z = -opencv_test.point.y; // ROS的上 = OpenCV下的反向 -y
}

void MathTool::axis_turn_ros_to_opencv(double &cv_x, double &cv_y, double &cv_z, double ros_x, double ros_y, double ros_z)
{
    cv_x = -ros_y; // OpenCV的右 = ROS左的反向
    cv_y = -ros_z; // OpenCV的下 = ROS上的反向
    cv_z = ros_x;  // OpenCV的前 = ROS的前
}

std::unique_ptr<cv::Point2f[]> MathTool::armor_vector_extend(cv::Point2f &tl, cv::Point2f &tr, cv::Point2f &bl, cv::Point2f &br)
{
    cv::Point2f left_vector = bl - tl;
    cv::Point2f right_vector = br - tr;
    cv::Point2f top_left_ext = tl - 0.55f * left_vector;
    cv::Point2f top_right_ext = tr - 0.55f * right_vector;
    cv::Point2f bottom_left_ext = bl + 0.55f * left_vector;
    cv::Point2f bottom_right_ext = br + 0.55f * right_vector;

    cv::Point2f top_horizontal_vec = top_right_ext - top_left_ext;
    cv::Point2f bottom_horizontal_vec = bottom_right_ext - bottom_left_ext;

    float shrink_ratio = 0.12f;

    cv::Point2f final_top_left = top_left_ext + shrink_ratio * top_horizontal_vec;
    cv::Point2f final_top_right = top_right_ext - shrink_ratio * top_horizontal_vec;
    cv::Point2f final_bottom_left = bottom_left_ext + shrink_ratio * bottom_horizontal_vec;
    cv::Point2f final_bottom_right = bottom_right_ext - shrink_ratio * bottom_horizontal_vec;

    cv::Point2f src_points[4] = {final_top_left, final_top_right, final_bottom_right, final_bottom_left};

    auto result = std::make_unique<cv::Point2f[]>(4);
    result[0] = final_top_left;
    result[1] = final_top_right;
    result[2] = final_bottom_right;
    result[3] = final_bottom_left;

    return result;
}