#include "armor_camera_capture.hpp"
#include "ArmorTracker.hpp"

/*
// #if defined(ARMOR_TRACKER_USE_MVSDK)
// #include "CameraApi.h"
// #endif

// namespace
// {
//     class ArmorCameraCapture
//     {
//     public:
//         ~ArmorCameraCapture()
//         {
//             release();
//         }

//         bool open()
//         {
// #if defined(ARMOR_TRACKER_USE_MVSDK)
//             if (CameraSdkInit(1) != CAMERA_STATUS_SUCCESS)
//             {
//                 return false;
//             }

//             std::vector<tSdkCameraDevInfo> camera_list(4);
//             INT camera_count = static_cast<INT>(camera_list.size());
//             if (CameraEnumerateDevice(camera_list.data(), &camera_count) != CAMERA_STATUS_SUCCESS || camera_count <= 0)
//             {
//                 return false;
//             }

//             if (CameraInit(&camera_list[0], -1, -1, &camera_handle_) != CAMERA_STATUS_SUCCESS)
//             {
//                 return false;
//             }
//             initialized_ = true;

//             if (CameraGetCapability(camera_handle_, &capability_) != CAMERA_STATUS_SUCCESS)
//             {
//                 release();
//                 return false;
//             }

//             const bool is_mono_sensor = capability_.sIspCapacity.bMonoSensor;
//             const UINT output_format = is_mono_sensor ? CAMERA_MEDIA_TYPE_MONO8 : CAMERA_MEDIA_TYPE_BGR8;
//             if (CameraSetIspOutFormat(camera_handle_, output_format) != CAMERA_STATUS_SUCCESS)
//             {
//                 release();
//                 return false;
//             }

//             const std::size_t max_width = static_cast<std::size_t>(capability_.sResolutionRange.iWidthMax);
//             const std::size_t max_height = static_cast<std::size_t>(capability_.sResolutionRange.iHeightMax);
//             const std::size_t output_channels = is_mono_sensor ? 1U : 3U;
//             rgb_buffer_.resize(max_width * max_height * output_channels);
//             if (rgb_buffer_.empty())
//             {
//                 release();
//                 return false;
//             }

//             if (CameraPlay(camera_handle_) != CAMERA_STATUS_SUCCESS)
//             {
//                 release();
//                 return false;
//             }

//             // ====== 新增：手动曝光设置 ======
//             // 关闭自动曝光（FALSE 表示手动模式，若未定义 FALSE 可直接用 0）
//             CameraSetAeState(camera_handle_, FALSE);
//             // 设置曝光时间为 2000 微秒（2ms）
//             CameraSetExposureTime(camera_handle_, 2000.0);
//             // 可选：检查返回值，若失败可打印警告但不影响相机使用
//             // ==============================
//             return true;
// #else
//             if (!capture_.open(0))
//             {
//                 return false;
//             }
//             initialized_ = true;
//             return true;
// #endif
//         }
//         bool read(cv::Mat &frame)
//         {
// #if defined(ARMOR_TRACKER_USE_MVSDK)
//             tSdkFrameHead frame_info{};
//             BYTE *raw_buffer = nullptr;
//             const CameraSdkStatus buffer_status = CameraGetImageBuffer(camera_handle_, &frame_info, &raw_buffer, 1000);
//             if (buffer_status != CAMERA_STATUS_SUCCESS)
//             {
//                 return false;
//             }

//             const CameraSdkStatus process_status = CameraImageProcess(camera_handle_, raw_buffer, rgb_buffer_.data(), &frame_info);
//             CameraReleaseImageBuffer(camera_handle_, raw_buffer);
//             if (process_status != CAMERA_STATUS_SUCCESS)
//             {
//                 return false;
//             }

//             const int image_type = frame_info.uiMediaType == CAMERA_MEDIA_TYPE_MONO8 ? CV_8UC1 : CV_8UC3;
//             frame = cv::Mat(frame_info.iHeight, frame_info.iWidth, image_type, rgb_buffer_.data()).clone();
//             if (frame.empty())
//             {
//                 return false;
//             }
//             if (image_type == CV_8UC1)
//             {
//                 cv::cvtColor(frame, frame, cv::COLOR_GRAY2BGR);
//             }
//             return true;
// #else
//             capture_.read(frame);
//             return !frame.empty();
// #endif
//         }

//         void release()
//         {
// #if defined(ARMOR_TRACKER_USE_MVSDK)
//             if (initialized_)
//             {
//                 CameraUnInit(camera_handle_);
//                 initialized_ = false;
//             }
//             rgb_buffer_.clear();
// #else
//             capture_.release();
//             initialized_ = false;
// #endif
//         }

//     private:
// #if defined(ARMOR_TRACKER_USE_MVSDK)
//         CameraHandle camera_handle_{};
//         tSdkCameraCapbility capability_{};
//         std::vector<unsigned char> rgb_buffer_;
// #else
//         cv::VideoCapture capture_;
// #endif
//         bool initialized_ = false;
//     };
// } // namespace

// class ArmorTracker : public rclcpp::Node
// {
// private:
//     // 图像处理类
//     PreProcess pp;

//     // 视频信息类
//     VedioProcessor vp;

//     // 旋转中心处理类
//     RotationCenterSolver centerSolver{1000}; // 记录最近1000帧 随便记

//     // 灯条处理类
//     LightBarProcessor lbp;

//     // 工具类
//     MathTool mtl;

//     // 多目标卡尔曼类
//     MultipleKalman mltkalman;

//     ArmorCameraCapture camera_capture_;
//     bool camera_ready_ = false;
//     bool has_last_frame_time_ = false;
//     std::chrono::steady_clock::time_point last_frame_time_{};

//     // 绘图类
//     // DrawKalmanFunction drawer{1000, 3500};

//     cv::VideoCapture cap;

//     armor_interfaces::msg::ArmorInfo armorTracker_; // 装甲信息结构体

//     rclcpp::Publisher<armor_interfaces::msg::ArmorInfo>::SharedPtr armor_info_publisher; // 装甲信息发布者
//     rclcpp::Publisher<geometry_msgs::msg::PointStamped>::SharedPtr center_publisher;     // 中心点发布者
//     std::unique_ptr<tf2_ros::TransformBroadcaster> tf_broadcaster;                       // TF广播器

//     cv::Matx33d cv_to_ros; // OpenCV到ROS的转换矩阵
//     double delta_t = 0.02; // 时间间隔

// public:
//     ArmorTracker() : Node("armor_tracker")
//     {
//         // 初始化发布者和TF广播器
//         armor_info_publisher = this->create_publisher<armor_interfaces::msg::ArmorInfo>("armor_info", 10);
//         center_publisher = this->create_publisher<geometry_msgs::msg::PointStamped>("center_point", 10);
//         tf_broadcaster = std::make_unique<tf2_ros::TransformBroadcaster>(*this);

//         // std::string path = "/home/eee/VSCCC/ACE3.2/2.mp4";
//         // cap.open(path);

//         camera_ready_ = camera_capture_.open();
//         if (!camera_ready_)
//         {
//             RCLCPP_ERROR(this->get_logger(), "相机初始化失败");
//             return;
//         }

//         RCLCPP_INFO(this->get_logger(), "相机初始化成功");
//     }

//     void run()
//     {
//         const float ARMOR_WIDTH = 0.135f;                    // 灯条中心间距 135mm
//         const float ARMOR_HEIGHT = 0.055f;                   // 灯条高度 55mm
//         const float HALF_ARMOR_WIDTH = ARMOR_WIDTH / 2.0f;   // 0.0675m
//         const float HALF_ARMOR_HEIGHT = ARMOR_HEIGHT / 2.0f; // 0.0275m

//         cv::Mat camera_matrix = (cv::Mat_<double>(3, 3) << 2374.54248, 0.0, 698.85288,
//                                  0.0, 2377.53648, 520.8649,
//                                  0.0, 0.0, 1.0);

//         // 畸变系数
//         cv::Mat dist_coeffs = (cv::Mat_<double>(1, 5) << -0.059743, 0.355479, -0.000625, 0.001595, 0.000000);

//         // 图像相关参数
//         cv::Mat img, imgClose;
//         int my_thre_value = 25; // 二值化查找蓝色装甲板光条参数

//         // cv::Mat imgFunc = drawer.DrawMyFunctionLine();

//         int frame_width, frame_height;
//         double fps, time;

//         double yaw;

//         if (!camera_ready_)
//         {
//             return;
//         }

//         while (camera_ready_ && rclcpp::ok())
//         {
//             // vp.getVideoinformation(cap, frame_width, frame_height, fps, time);
//             // 前置变量，一个记录每帧间隔时间，一个记录这一帧记录的yaw
//             double delta_time = 1.0 / fps;
//             std::vector<double> current_yaws;

//             if (!camera_capture_.read(img))
//                 break;

//             if (img.empty())
//             {
//                 std::cout << "我去哥们你这个怎么是空图像啊" << std::endl;
//                 break; // 图像空了应该退出循环
//             }

//             cv::Mat imgClose = pp.PreprocessImg(img, my_thre_value);

//             std::vector<std::vector<cv::Point>> contours;
//             cv::findContours(imgClose, contours, cv::RETR_CCOMP, cv::CHAIN_APPROX_SIMPLE);

//             std::vector<LightBar> target;

//             for (int i = 0; i < contours.size(); i++)
//             {
//                 if (contours[i].size() < 4)
//                 {
//                     continue;
//                 }

//                 cv::RotatedRect rect = cv::minAreaRect(contours[i]);
//                 // 灯条的外接矩形

//                 double w = rect.size.width;
//                 double h = rect.size.height;
//                 double angle = rect.angle;
//                 // width height

//                 if (w < h)
//                 {
//                     std::swap(w, h);
//                     angle += 90.0;
//                 }

//                 if (w * h < 200)
//                     continue;
//                 if (w / h < 1.5)
//                     continue;

//                 LightBar light;
//                 light.rect = rect;
//                 light.angle = angle;
//                 light.center = rect.center;

//                 target.push_back(light);
//             }

//             for (int i = 0; i < target.size(); i++)
//             {
//                 for (int j = i + 1; j < target.size(); j++)
//                 {
//                     LightBar &l1 = target[i];
//                     LightBar &l2 = target[j];

//                     // 1.角度差
//                     double angle_cha = abs(l1.angle - l2.angle);
//                     if (angle_cha > 10)
//                         continue;

//                     // 2.高度差（这里用长边作为“高度”）
//                     double w1 = std::max(l1.rect.size.height, l1.rect.size.width);
//                     double w2 = std::max(l2.rect.size.height, l2.rect.size.width);
//                     double ave_width = (w1 + w2) / 2;

//                     double h_cha = abs(w1 - w2);
//                     if (h_cha / ave_width > 0.2)
//                         continue;

//                     // 3.水平间隔
//                     double dist = mtl.getMyDistance(l1.center, l2.center);
//                     if (dist / ave_width < 0.5 || dist / ave_width > 8)
//                         continue;

//                     // 4.垂直间隔
//                     double y_cha = abs(l1.center.y - l2.center.y);
//                     if (y_cha / ave_width > 0.3)
//                         continue;

//                     LightBar leftBar, rightBar;
//                     if (l1.center.x < l2.center.x)
//                     {
//                         leftBar = l1;
//                         rightBar = l2;
//                     }
//                     else
//                     {
//                         leftBar = l2;
//                         rightBar = l1;
//                     }

//                     cv::Point2f tl, bl, tr, br;
//                     lbp.getLightBarEndpoints(leftBar.rect, tl, bl);
//                     lbp.getLightBarEndpoints(rightBar.rect, tr, br);
//                     // top bottom left right

//                     std::vector<cv::Point2f> armorPoints = {tl, tr, br, bl};

//                     for (int k = 0; k < 4; k++)
//                     {
//                         cv::line(img, armorPoints[k], armorPoints[(k + 1) % 4], cv::Scalar(0, 255, 0), 2);
//                     }

//                     for (int k = 0; k < 4; k++)
//                     {
//                         std::string coord = "(" + std::to_string((int)armorPoints[k].x) + "," + std::to_string((int)armorPoints[k].y) + ")";
//                         cv::putText(img, coord, armorPoints[k], cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 255), 1);
//                         cv::circle(img, armorPoints[k], 2, cv::Scalar(0, 0, 255), -1);
//                     }

//                     // PnP解算
//                     std::vector<cv::Point3f> object_points = {
//                         cv::Point3f(-HALF_ARMOR_WIDTH, -HALF_ARMOR_HEIGHT, 0.0f), // 左上
//                         cv::Point3f(HALF_ARMOR_WIDTH, -HALF_ARMOR_HEIGHT, 0.0f),  // 右上
//                         cv::Point3f(HALF_ARMOR_WIDTH, HALF_ARMOR_HEIGHT, 0.0f),   // 右下
//                         cv::Point3f(-HALF_ARMOR_WIDTH, HALF_ARMOR_HEIGHT, 0.0f)   // 左下
//                     };

//                     cv::Mat rvec, tvec;
//                     bool pnp_success = cv::solvePnP(object_points, armorPoints, camera_matrix, dist_coeffs, rvec, tvec);

//                     if (pnp_success)
//                     {
//                         // 向处理旋转中心的对象添加观测量
//                         centerSolver.addObservation(tvec, rvec);

//                         // 尝试解算
//                         cv::Point3f calculated_center;
//                         bool fit_success = centerSolver.solve(calculated_center);

//                         if (fit_success)
//                         {
//                             // 将计算出的 3D 中心投影回 2D 画面进行绘制
//                             std::vector<cv::Point3f> fit_center_3d = {calculated_center};
//                             std::vector<cv::Point2f> fit_center_2d;

//                             // calculated_center 已经是相机坐标系下的绝对坐标了！！！！！！
//                             // 所以投影时，rvec 和 tvec 都要给 0 ！！！！！不需要不需要不需要！！！！(因为坐标变换已经包含在点里了)
//                             cv::projectPoints(fit_center_3d, cv::Mat::zeros(3, 1, CV_64F), cv::Mat::zeros(3, 1, CV_64F),
//                                               camera_matrix, dist_coeffs, fit_center_2d);

//                             // 绘制拟合出来的中心（画个绿色的点）
//                             cv::circle(img, fit_center_2d[0], 8, cv::Scalar(0, 255, 0), -1);
//                             cv::putText(img, "Fit Center", fit_center_2d[0] + cv::Point2f(10, -10),
//                                         cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 255, 0), 2);

//                             // 输出拟合半径看看准不准
//                             float radius = centerSolver.getLastRadius(calculated_center);
//                             std::string r_text = "R: " + std::to_string(radius).substr(0, 4) + "m";
//                             cv::putText(img, r_text, fit_center_2d[0] + cv::Point2f(10, 10),
//                                         cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 255, 0), 2);
//                         }
//                         else
//                         {
//                             // 拟合没准备好时，显示“Collecting Data...”
//                             cv::putText(img, "Collecting Data...", cv::Point2f(50, 50),
//                                         cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 0, 255), 2);
//                         }
//                     }

//                     // 距离：相机到装甲板中心的欧氏距离（米）
//                     double distance = cv::norm(tvec);

//                     // yaw 水平偏角
//                     // 之所以用了.at是因为AI告诉我，使用.at是更加安全的访问方式
//                     yaw = std::atan2(tvec.at<double>(0), tvec.at<double>(2)) * 180 / M_PI;
//                     current_yaws.push_back(yaw);
//                     // horiz 这是水平距离
//                     double horiz = std::sqrt(tvec.at<double>(0) * tvec.at<double>(0) + tvec.at<double>(2) * tvec.at<double>(2));
//                     // pitch：垂直偏角
//                     double pitch = std::atan2(-tvec.at<double>(1), horiz) * 180 / M_PI;

//                     // 找装甲板中心
//                     // 四个点加起来除以四就是我的结果
//                     cv::Point2f armor_center(0, 0);
//                     for (const auto &p : armorPoints)
//                         armor_center += p;
//                     armor_center /= 4.0f;

//                     // 显示信息（黄色文字）
//                     cv::Point2f text_pos = armor_center + cv::Point2f(-150, -40);
//                     std::string info_text = cv::format("Yaw: %.1f°  Pitch: %.1f°  Dist: %.2f", yaw, pitch, distance);
//                     cv::putText(img, info_text, text_pos, cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 255, 255), 2);
//                 }
//             }

//             mltkalman.update(current_yaws, delta_time);
//             armorTracker_.time = time;
//             if (mltkalman.is_active1())
//             {
//                 armorTracker_.yaw = mltkalman.get_yaw1();
//                 armor_info_publisher->publish(armorTracker_);
//             }

//             if (mltkalman.is_active2())
//             {
//                 armorTracker_.yaw = mltkalman.get_yaw2();
//                 armor_info_publisher->publish(armorTracker_);
//             }

//             // drawer.DrawMyFxxkingPoint(imgFunc, time, yaw, 1.5);

//             // cv::imshow("imgclose", imgClose);
//             cv::imshow("myimg", img);
//             // cv::imshow("func", imgFunc);

//             char key = cv::waitKey(30);
//             if (key == 27)
//             {
//                 break;
//             }
//             // writer.write(img);
//         }

//         camera_capture_.release();
//         // cap.release();
//         // writer.release();
//         cv::destroyAllWindows();
//     }

// private:
//     void publish_armor_msg()
//     {
//         armor_interfaces::msg::ArmorInfo armor_info_msg; // 创建装甲信息消息 frame
//         armor_info_msg.yaw = armorTracker_.yaw;          // 设置yaw

//         armor_info_publisher->publish(armor_info_msg); // 发布装甲信息
//     }
// };
*/

int main(int argc, char *argv[])
{
    rclcpp::init(argc, argv);
    auto tracker = std::make_shared<ArmorTracker>();
    tracker->run();
    rclcpp::spin(tracker);
    rclcpp::shutdown();
    return 0;
}