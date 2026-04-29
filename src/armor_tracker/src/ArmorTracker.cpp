#include "ArmorTracker.hpp"

#include <algorithm>
#include <cmath>
#include <chrono>

ArmorTracker::ArmorTracker() : Node("armor_tracker")
{
    std::string path = "/home/eee/ros2/src/armor_detect_ros2-main/vedio/2.mp4";
    cap.open(path);
    
    int num_cores = cv::getNumberOfCPUs();
    cv::setNumThreads(num_cores);

    net = cv::dnn::readNetFromONNX("/home/eee/ros2/src/armor_detect_ros2-main/src/Number-Classifier-for-RoboMaster-main/output/Zenet-sim.onnx");

    // // 初始化调试图像发布者
    // debug_image_pub_ = image_transport::create_publisher(this, "/tracker/debug_image");

    // 初始化发布者和TF广播器
    armor_info_publisher = this->create_publisher<armor_interfaces::msg::ArmorInfo>("armor_info", 10);
    center_publisher = this->create_publisher<geometry_msgs::msg::PointStamped>("center_point", 10);

    // 发布相机到世界坐标系的初始姿态
    tf_camera_to_world_publisher_ = this->create_publisher<sensor_msgs::msg::JointState>("/joint_states", 10);
    joint_state_msg_.name = {"yaw_joint", "pitch_joint"};
    joint_state_msg_.position = {0.0, 0.0};
    tf_camera_to_world_publisher_->publish(joint_state_msg_);

    // 动态广播器，用于发布旋转中心到相机的位置
    tf_broadcaster_ = std::make_unique<tf2_ros::TransformBroadcaster>(this);

    // 获取图像时间戳发布方
    time_stamp_publisher_ = this->create_publisher<std_msgs::msg::Float32>("time_stamp_from_get_image", 10);

    // TF相关参数
    target_frame_ = "base_link"; // 赋值给成员变量，不是声明局部变量
    typedef std::chrono::duration<int> seconds_type;
    seconds_type buffer_timeout(1);

    // 初始化TF监听器和缓冲区
    tf_buffer_ = std::make_unique<tf2_ros::Buffer>(this->get_clock());
    auto timer_interface = std::make_shared<tf2_ros::CreateTimerROS>(
        this->get_node_base_interface(),
        this->get_node_timers_interface());
    tf_buffer_->setCreateTimerInterface(timer_interface);
    transform_listener_ = std::make_shared<tf2_ros::TransformListener>(*tf_buffer_);

    // 初始化串口数据发布者和订阅者
    serial_driver_publisher_ = this->create_publisher<armor_interfaces::msg::SerialDriver>("/tracker/target", rclcpp::SensorDataQoS());

    // 订阅串口数据
    camera_to_world_sub_ = this->create_subscription<armor_interfaces::msg::SerialReceiveData>(
        "/tracker/receive_data", rclcpp::SensorDataQoS(),
        std::bind(&ArmorTracker::receiveDataCallback, this, std::placeholders::_1));

    this->declare_parameter<std::string>("camera_info_path", "");
    std::string yaml_path = this->get_parameter("camera_info_path").as_string();

    if (yaml_path.empty())
    {
        RCLCPP_ERROR(this->get_logger(), "必须提供 camera_info_path 参数");
        return;
    }

    // 使用抽离出来的解析器
    if (!rm_vision::CameraIntrinsicsParser::parseYaml(yaml_path, intrinsics))
    {
        RCLCPP_ERROR(this->get_logger(), "加载内参失败，请检查文件格式和路径");
        return;
    }

    RCLCPP_INFO(this->get_logger(), "成功加载相机 [%s] 的内参", intrinsics.camera_name.c_str());

    if (intrinsics.distortion_model != "plumb_bob")
    {
        RCLCPP_WARN(this->get_logger(),
                    "畸变模型为 '%s'，当前示例只针对 plumb_bob。",
                    intrinsics.distortion_model.c_str());
    }

    // 生成去畸变映射表
    cv::Mat map1, map2;
    cv::initUndistortRectifyMap(
        intrinsics.K, intrinsics.D, cv::Mat::eye(3, 3, CV_64F), intrinsics.K,
        intrinsics.image_size, CV_16SC2, map1, map2);

    last_time_ = this->now();

    RCLCPP_INFO(this->get_logger(), "相机初始化成功");

    this->declare_parameter<int>("setting", 3);
    setting_ = this->get_parameter("setting").as_int();

    this->declare_parameter<int>("blue_thre_value", 140);
    blue_thre_value_ = this->get_parameter("blue_thre_value").as_int();

    this->declare_parameter<int>("red_thre_value", 150);
    red_thre_value_ = this->get_parameter("red_thre_value").as_int();

    this->declare_parameter<int>("exposure_time", 3000);
    exposure_time_ = this->get_parameter("exposure_time").as_int();

    this->declare_parameter<double>("gain", 8.0);
    gain_ = this->get_parameter("gain").as_double();

    this->declare_parameter<double>("gamma", 1.5); // 1.5 是个不错的起步值，暗部会明显提亮
    double gamma_ = this->get_parameter("gamma").as_double();

    RCLCPP_INFO(this->get_logger(), "对方阵营参数: %d (0=蓝色, 2=红色)", setting_);

    param_subscriber_ = this->add_on_set_parameters_callback(
        std::bind(&ArmorTracker::parametersCallback, this, std::placeholders::_1));

    //初始化相机
    if (!galaxy_camera_.init())
    {
        // 初始化失败
        RCLCPP_INFO(this->get_logger(), "相机初始化失败");
        return;
    }

    galaxy_camera_.setExposureTime(exposure_time_);
    galaxy_camera_.setGain(gain_);
    galaxy_camera_.setGamma(gamma_);
    galaxy_camera_.startCapture();

    last_time_ = this->now();
    RCLCPP_INFO(this->get_logger(), "相机初始化成功, 对方阵营: %d (0=蓝, 2=红)", setting_);
}

void ArmorTracker::run()
{
    isFindArmor = false;

    // cv::Mat camera_matrix = (cv::Mat_<double>(3, 3) << 2374.54248, 0.0, 698.85288,
    //                          0.0, 2377.53648, 520.8649,
    //                          0.0, 0.0, 1.0);

    // // 畸变系数
    // cv::Mat dist_coeffs = (cv::Mat_<double>(1, 5) << -0.059743, 0.355479, -0.000625, 0.001595, 0.000000);

    // 图像相关参数
    cv::Mat img, imgClose, imgWarp;
    galaxy_camera::ImageData img_data;

    double yaw;
    double pitch;

    std::vector<cv::Point2f> armorPoints;
    armorPoints.reserve(4);

    if (!galaxy_camera_.isReady())
    {
        std::cout << "我草，哥们，相机炸了" << std::endl;
        return;
    }

    cv::Mat rvec_zero = cv::Mat::zeros(3, 1, CV_64F);
    cv::Mat tvec_zero = cv::Mat::zeros(3, 1, CV_64F);
    std::vector<double> current_yaws;
    current_yaws.reserve(32);
    std::vector<std::vector<cv::Point>> contours;

    while (galaxy_camera_.isReady() && rclcpp::ok())
    {
        auto time_stamp_msg = std_msgs::msg::Float32();
        rclcpp::Time time = this->now();
        time_stamp_msg.data = time.seconds();

        armorPoints.clear();
        LightBarVector.clear();
        current_yaws.clear(); 
        contours.clear();

        rclcpp::Time current_time = this->now(); // 获取当前时间

        rclcpp::Duration delta_t = current_time - last_time_; // 计算与上次处理消息的时间间隔

        last_time_ = current_time; // 更新上次处理消息的时间

        double delta_time = delta_t.seconds(); // 获取时间间隔，单位为秒

        if (!galaxy_camera_.getImage(img_data))
        {
            // 给相机一点时间出图，避免 CPU 100% 空转
            std::this_thread::sleep_for(std::chrono::milliseconds(2));
            cv::waitKey(1); // 【关键修复】：确保即使没图，窗口依然能响应操作系统事件！
            continue;       // 不要 break，继续等待下一帧
        }

        img = cv::Mat(img_data.height, img_data.width, CV_8UC3, img_data.data.data());

        cv::cvtColor(img, img, cv::COLOR_RGB2BGR);

        if (img.empty())
        {
            std::cout << "我去哥们你这个怎么是空图像啊" << std::endl;
            break; // 图像空了应该退出循环
        }

        imgClose = pp.PreprocessImg(img, blue_thre_value_, red_thre_value_, setting_);

        cv::findContours(imgClose, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

        for (int i = 0; i < contours.size(); i++)
        {
            if (contours[i].size() < 4)
            {
                continue;
            }

            cv::RotatedRect rect = cv::minAreaRect(contours[i]);
            cv::Rect light_rect = cv::boundingRect(contours[i]);

            double w = rect.size.width;
            double h = rect.size.height;

            cv::Vec4f line_vector;
            // cv::DIST_L2 表示使用标准的最小二乘法
            cv::fitLine(contours[i], line_vector, cv::DIST_L2, 0, 0.01, 0.01);

            // line_vector 的前两个元素是直线的方向向量 (vx, vy)
            // 后两个元素是直线上的一点 (x0, y0)，通常是质心，这里我们算角度只需要向量
            float vx = line_vector[0];
            float vy = line_vector[1];

            // 使用 std::atan2 计算角度，得到的是弧度 [-pi, pi]
            // 并且可以通过乘以 180 / PI 转换为角度
            double angle_rad = std::atan2(vy, vx);
            double angle_deg = angle_rad * 180.0 / CV_PI;

            // 统一映射到你想要的范围（比如 0~180 度）
            if (angle_deg < 0)
            {
                angle_deg += 180.0;
            }

            if (w < h)
            {
                std::swap(w, h);
            }

            if (w * h < 100)
                continue;
            if (w / h < 1.2)
                continue;

            LightBar light;
            light.rect = rect;
            light.angle = angle_deg;
            light.center = rect.center;
            light.light_rect = light_rect;

            ColorDetect(contours[i], light, img);
        }

        for (int i = 0; i < this->LightBarVector.size(); i++)
        {
            for (int j = i + 1; j < this->LightBarVector.size(); j++)
            {
                LightBar &l1 = LightBarVector[i];
                LightBar &l2 = LightBarVector[j];

                // 优化：利用 std::abs 避免隐式转整型导致的精度丢失问题并加速计算
                double angle_cha = std::abs(l1.angle - l2.angle);

                if (angle_cha > 180)
                {
                    angle_cha = 360 - angle_cha;
                }
                // if (angle_cha > 10)
                //     continue;

                double w1 = std::max(l1.rect.size.height, l1.rect.size.width);
                double w2 = std::max(l2.rect.size.height, l2.rect.size.width);
                double ave_height = (w1 + w2) / 2;

                double h_cha = std::abs(w1 - w2);
                if (h_cha / ave_height > 0.5)
                    continue;

                double dist = mtl.getMyDistance(l1.center, l2.center);
                // if (dist / ave_height < 1.3 || dist / ave_height > 4.8)
                //     continue;

                if (l1.color != l2.color)
                {
                    continue;
                }

                if (std::abs(l1.center.y - l2.center.y) / ave_height > 2.0)
                {
                    continue;
                }

                if (this->containLight(l1, l2))
                {
                    continue;
                }

                LightBar leftBar, rightBar;
                if (l1.center.x < l2.center.x)
                {
                    leftBar = l1;
                    rightBar = l2;
                }
                else
                {
                    leftBar = l2;
                    rightBar = l1;
                }

                cv::Point2f lp[4];
                l1.rect.points(lp);

                cv::Point2f rp[4];
                l2.rect.points(rp);

                cv::Point2f tl, bl, tr, br;
                cv::Point2f n_tl, n_bl, n_tr, n_br;
                cv::Point2f top_lift, top_right, bottom_left, bottom_right;

                lbp.getLightBarEndpoints(leftBar.rect, tl, bl);
                lbp.getLightBarEndpoints(rightBar.rect, tr, br);

                lbp.getLightBarInnerPoints(leftBar.rect, n_tl, n_bl, 0);
                lbp.getLightBarInnerPoints(rightBar.rect, n_tr, n_br, 1);

                armorPoints = {tl, tr, br, bl};

                // if (tl.x < 0 || tl.y < 0 || tr.x < 0 || tr.y < 0 || bl.x < 0 || bl.y < 0 || br.x < 0 || br.y < 0)
                // {
                //     RCLCPP_INFO(get_logger(), "计算初始时顶点阶段出错");
                //     continue;
                // }

                // 计算从上到下的方向向量
                auto result = mtl.armor_vector_extend(n_tl, n_tr, n_bl, n_br);
                cv::Point2f src_points[4];
                src_points[0] = result[0];
                src_points[1] = result[1];
                src_points[2] = result[2];
                src_points[3] = result[3];

                cv::Point2f dst_points[4] = {{0.0f, 0.0f}, {28.0f, 0.0f}, {28.0f, 28.0f}, {0.0f, 28.0f}};
                cv::Mat warp_matrix = cv::getPerspectiveTransform(src_points, dst_points);
                cv::warpPerspective(img, imgWarp, warp_matrix, cv::Size(28, 28));
                cvtColor(imgWarp, imgWarp, cv::COLOR_BGR2GRAY);

                // cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE(2.0, cv::Size(4, 4));
                // clahe->apply(imgWarp, imgWarp); // 克拉赫增强

                cv::threshold(imgWarp, imgWarp, 5, 255, cv::THRESH_BINARY);

                cv::Mat blob = cv::dnn::blobFromImage(imgWarp, 1.0 / 255.0, cv::Size(28.0f, 28.0f), cv::Scalar(0), false, false);

                net.setInput(blob);
                cv::Mat output = net.forward();

                cv::Mat class_scores = output.colRange(1, 11);

                cv::Point classIdPoint;
                double confidence;
                minMaxLoc(class_scores, nullptr, &confidence, nullptr, &classIdPoint);
                int predicted_id = classIdPoint.x;

                cv::putText(img, "Number of Observations: " + std::to_string(predicted_id), cv::Point2f(100, 100),
                            cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 0, 255), 2);

                double armor_width = tr.x - tl.x;
                double armor_height = bl.y - tl.y;

                if (armor_width <= 0 || armor_height <= 0)
                {
                    continue;
                }
                cv::Rect rect(tl.x, tl.y, armor_width, armor_height);

                if (rect.x < 0)
                    rect.x = 0;
                if (rect.y < 0)
                    rect.y = 0;
                if (rect.x + rect.width > img.cols)
                    rect.width = img.cols - rect.x;
                if (rect.y + rect.height > img.rows)
                    rect.height = img.rows - rect.y;

                if (rect.width <= 0 || rect.height <= 0)
                {
                    continue;
                }

                for (int k = 0; k < 4; k++)
                {
                    int next_k = (k + 1) % 4;
                    cv::line(img, lp[k], lp[next_k], cv::Scalar(0, 255, 0), 2);
                    cv::line(img, rp[k], rp[next_k], cv::Scalar(0, 255, 0), 2);
                    cv::line(img, armorPoints[k], armorPoints[next_k], cv::Scalar(0, 255, 0), 2);

                    std::string coord = cv::format("(%d,%d)", (int)armorPoints[k].x, (int)armorPoints[k].y);
                    cv::putText(img, coord, armorPoints[k], cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 255), 1);
                    cv::circle(img, armorPoints[k], 2, cv::Scalar(0, 0, 255), -1);

                    isFindArmor = true;
                }

                cv::Mat rvec, tvec;
                bool pnp_success = cv::solvePnP(object_points, armorPoints, intrinsics.K, intrinsics.D, rvec, tvec);

                /*
                if (pnp_success)
                {
                     centerSolver.addObservation(tvec, rvec);
                    cv::Point3f calculated_center;
                    bool fit_success = centerSolver.solve(calculated_center);

                     if (fit_success)
                     {
                        std::vector<cv::Point3f> fit_center_3d = {calculated_center};
                        std::vector<cv::Point2f> fit_center_2d;

                         // 优化：复用初始化的 zeros 矩阵，杜绝每帧动态生成 Mat 的操作
                       cv::projectPoints(fit_center_3d, rvec_zero, tvec_zero,
                                           intrinsics.K, intrinsics.D, fit_center_2d);

                        cv::circle(img, fit_center_2d[0], 8, cv::Scalar(0, 255, 0), -1);
                        cv::putText(img, "Fit Center", fit_center_2d[0] + cv::Point2f(10, -10),
                                    cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 255, 0), 2);

                        float radius = centerSolver.getLastRadius(calculated_center);
                         // 优化：避免 substring 操作造成的越界崩溃与字符串复制开销
                        std::string r_text = cv::format("R: %.2fm", radius);
                        cv::putText(img, r_text, fit_center_2d[0] + cv::Point2f(10, 10),
                                    cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 255, 0), 2);
                 }
                     else
                     {
                         cv::putText(img, "Collecting Data...", cv::Point2f(50, 50),
                                  cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 0, 255), 2);
                     }
                 }
                */

                double distance = cv::norm(tvec);

                // 优化：用底层内存指针获取数据远比调用 .at<double>() 快
                const double *tvec_data = tvec.ptr<double>();

                cv::Mat R;
                Eigen::Vector3d pos;
                Eigen::Vector3d norm;
                Eigen::Vector3d rotated_center;
                cv::Rodrigues(rvec, R); // 旋转向量 -> 旋转矩阵

                cv::cv2eigen(tvec, pos); // huoqu pos

                cv::cv2eigen(R.col(2), norm); // huoqu norm

                if (ekf.is_initialized == false)
                {
                    norm[0] *= 0.25;
                    norm[1] *= 0.25;
                    norm[2] *= 0.25;
                }
                else if (ekf.is_initialized == true)
                {
                    double norm_horiz = std::sqrt(norm[2] * norm[2] + norm[0] * norm[0]);
                    double norm_zong = std::sqrt(norm[1] * norm[1] + norm[2] * norm[2] + norm[0] * norm[0]);
                    double n = norm_zong / norm_horiz;
                    norm[0] *= ekf.X(6) * n;
                    norm[1] *= ekf.X(6) * n;
                    norm[2] *= ekf.X(6) * n;
                }
                rotated_center[0] = pos[0] + norm[0];
                rotated_center[1] = pos[1] + norm[1];
                rotated_center[2] = pos[2] + norm[2];

                geometry_msgs::msg::PointStamped rotated_center_point_in_camera;
                rotated_center_point_in_camera.header.frame_id = "camera_link";
                rotated_center_point_in_camera.header.stamp = rclcpp::Time(0);
                rotated_center_point_in_camera.point.x = rotated_center(2);  // ROS的前 = OpenCV的前
                rotated_center_point_in_camera.point.y = -rotated_center(0); // ROS的左 = OpenCV右的反向
                rotated_center_point_in_camera.point.z = -rotated_center(1); // ROS的上 = OpenCV下的反向

                geometry_msgs::msg::PointStamped rotated_center_point_out_local;

                geometry_msgs::msg::PointStamped target_point_in_camera;
                target_point_in_camera.header.frame_id = "camera_link";
                target_point_in_camera.header.stamp = rclcpp::Time(0);
                target_point_in_camera.point.x = tvec.at<double>(2);  // ROS的前 = OpenCV的前
                target_point_in_camera.point.y = -tvec.at<double>(0); // ROS的左 = OpenCV右的反向
                target_point_in_camera.point.z = -tvec.at<double>(1); // ROS的上 = OpenCV下的反向

                geometry_msgs::msg::PointStamped point_out_local;
                try
                {
                    tf_buffer_->transform(rotated_center_point_in_camera, rotated_center_point_out_local, "base_link", tf2::durationFromSec(0.0));
                    tf_buffer_->transform(target_point_in_camera, point_out_local, target_frame_, tf2::durationFromSec(0.0));
                    this->target_point_out = point_out_local;
                    // auto camera_transformStamped = tf_buffer_->lookupTransform("camera_link", "base_link", tf2::TimePointZero);
                }
                catch (const tf2::TransformException &ex)
                {
                    // // 如果查不到，说明底盘位姿没跟上，丢弃这帧的数据，直接 continue 抓取下一张图
                    // RCLCPP_WARN_THROTTLE(this->get_logger(), *this->get_clock(), 1000,
                    //                      "坐标变换未就绪，丢弃本帧: %s", ex.what());
                    // isFindArmor = false;
                    // continue;
                }

                double dx = point_out_local.point.x - rotated_center_point_out_local.point.x;
                double dy = point_out_local.point.y - rotated_center_point_out_local.point.y;

                double true_theta = std::atan2(dy, dx);

                Eigen::VectorXd X(9);
                Eigen::VectorXd Z(4);

                if (ekf.is_initialized == false && isFindArmor == true)
                {
                    X << rotated_center_point_out_local.point.x, rotated_center_point_out_local.point.y,
                        rotated_center_point_out_local.point.z, 0, 0, 0, 0.25, true_theta, 0;
                    ekf.init(X);
                }
                else if (ekf.is_initialized == true && isFindArmor == true)
                {
                    Z << point_out_local.point.x, point_out_local.point.y, point_out_local.point.z, true_theta;
                    ekf.predict(delta_time);
                    ekf.update(Z);
                }

                // 至此，我们获得了相机到底座的变换关系，以及装甲板中心点在底座坐标系下的坐标！！！

                yaw = std::atan2(target_point_out.point.y, target_point_out.point.x) * 180 / M_PI;
                current_yaws.push_back(yaw);

                double horiz = std::sqrt(target_point_out.point.x * target_point_out.point.x + target_point_out.point.y * target_point_out.point.y);
                pitch = std::atan2(target_point_out.point.z, horiz) * 180 / M_PI;

                // std::cout << "装甲板中心点坐标 (观测量): "
                //           << " | Yaw: " << yaw
                //           << " Pitch: " << pitch
                //           << " horiz: " << horiz
                //           << std::endl;

                cv::Point2f armor_center(0, 0);
                for (const auto &p : armorPoints)
                    armor_center += p;
                armor_center /= 4.0f;

                cv::Point2f text_pos = armor_center + cv::Point2f(-150, -40);
                std::string info_text = cv::format("Yaw: %.1f°  Pitch: %.1f°  Dist: %.2f", yaw, pitch, distance);
                cv::putText(img, info_text, text_pos, cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 255, 255), 2);
            }
        }

        geometry_msgs::msg::PointStamped safe_point_out;

        // 新学的绝招，瞬间加锁，把数据掏出来，立刻解锁
        {
            std::lock_guard<std::mutex> lock(target_point_out_mutex_);
            safe_point_out = this->target_point_out;
        }

        if (KalmanInit == false && isFindArmor == true)
        {
            kalman.init(safe_point_out, delta_time);
            KalmanInit = true;
        }

        if (KalmanInit == true)
        {
            kalman.predict(delta_time);
            if (isFindArmor == true)
            {
                kalman.update(safe_point_out);
            }
        }

        geometry_msgs::msg::PointStamped kalman_point;
        kalman_point.header.frame_id = "base_link";
        kalman_point.header.stamp = rclcpp::Time(0);
        kalman_point.point.x = kalman.X(0);
        kalman_point.point.y = kalman.X(1);
        kalman_point.point.z = kalman.X(2);

        geometry_msgs::msg::PointStamped rotated_center_extra_kalman_point;
        rotated_center_extra_kalman_point.header.frame_id = "base_link";
        rotated_center_extra_kalman_point.header.stamp = rclcpp::Time(0);
        rotated_center_extra_kalman_point.point.x = ekf.X(0);
        rotated_center_extra_kalman_point.point.y = ekf.X(1);
        rotated_center_extra_kalman_point.point.z = ekf.X(2);

        if (!armorPoints.empty())
        {
            yaw_test = std::atan2(kalman_point.point.y, kalman_point.point.x) * 180.0 / M_PI;
            double horiz_test = std::sqrt(kalman_point.point.x * kalman_point.point.x + kalman_point.point.y * kalman_point.point.y);
            pitch_test = std::atan2(kalman_point.point.z, horiz_test) * 180.0 / M_PI;
        }
        geometry_msgs::msg::PointStamped test;
        geometry_msgs::msg::PointStamped ekf_test;
        try
        {
            test = tf_buffer_->transform(kalman_point, "camera_link");
            ekf_test = tf_buffer_->transform(rotated_center_extra_kalman_point, "camera_link");
        }
        catch (const tf2::TransformException &ex)
        {
            // RCLCPP_WARN_THROTTLE(this->get_logger(), *this->get_clock(), 1000,
            //                      "卡尔曼预测点 TF 变换未就绪: %s", ex.what());
            // continue; // 直接跳过这帧的后续绘制和发送，抓取下一张图
        }

        double project_x, project_y, project_z;
        double rotated_project_x, rotated_project_y, rotated_project_z;
        mtl.axis_turn_ros_to_opencv(project_x, project_y, project_z, test);
        mtl.axis_turn_ros_to_opencv(rotated_project_x, rotated_project_y, rotated_project_z, ekf_test);

        std::vector<cv::Point3f> target_out_center_3d;
        target_out_center_3d.emplace_back(project_x, project_y, project_z);
        std::vector<cv::Point2f> target_out_center_2d;

        std::vector<cv::Point3f> rotated_center_out_center_3d;
        rotated_center_out_center_3d.emplace_back(rotated_project_x, rotated_project_y, rotated_project_z);
        std::vector<cv::Point2f> rotated_center_out_center_2d;

        // 优化：复用初始化的 zeros 矩阵，杜绝每帧动态生成 Mat 的操作
        cv::projectPoints(target_out_center_3d, rvec_zero, tvec_zero,
                          intrinsics.K, intrinsics.D, target_out_center_2d);

        cv::projectPoints(rotated_center_out_center_3d, rvec_zero, tvec_zero,
                          intrinsics.K, intrinsics.D, rotated_center_out_center_2d);

        cv::circle(img, target_out_center_2d[0], 8, cv::Scalar(0, 0, 255), -1);
        cv::circle(img, rotated_center_out_center_2d[0], 8, cv::Scalar(0, 255, 0), -1);

        cv::imshow("Debug Image", img);
        if (!imgWarp.empty())
        {
            cv::imshow("Warped Armor", imgWarp);
        }
        cv::imshow("Preprocessed", imgClose);
        cv::waitKey(1);
        publish_to_serial_driver(yaw_test, pitch_test, armorPoints);
        time_stamp_publisher_->publish(time_stamp_msg);
    }

    galaxy_camera_.stop();
    cv::destroyAllWindows();
}

void ArmorTracker::publish_armor_msg()
{
    armor_interfaces::msg::ArmorInfo armor_info_msg;
    armor_info_msg.yaw = armorTracker_.yaw;

    armor_info_publisher->publish(armor_info_msg);
}

bool ArmorTracker::containLight(const LightBar &light_1, const LightBar &light_2)
{
    if (light_1.center.x < 0 || light_1.center.y < 0 ||
        light_2.center.x < 0 || light_2.center.y < 0)
    {
        RCLCPP_WARN(this->get_logger(), "灯条中心点坐标无效");
        return false;
    }

    float expand_factor = 100.0f;
    float min_x = std::min(light_1.center.x, light_2.center.x) - expand_factor;
    float max_x = std::max(light_1.center.x, light_2.center.x) + expand_factor;
    float min_y = std::min(light_1.center.y, light_2.center.y) - expand_factor;
    float max_y = std::max(light_1.center.y, light_2.center.y) + expand_factor;

    for (const auto &test_light : LightBarVector)
    {
        if (test_light.center == light_1.center || test_light.center == light_2.center)
        {
            continue;
        }
        if (test_light.center.x >= min_x && test_light.center.x <= max_x &&
            test_light.center.y >= min_y && test_light.center.y <= max_y)
        {
            return true;
        }
    }

    return false;
}

void ArmorTracker::ColorDetect(std::vector<cv::Point> contour, LightBar light, cv::Mat img)
{
    if (img.empty() ||
        light.light_rect.x < 0 || light.light_rect.y < 0 ||
        light.light_rect.width <= 0 || light.light_rect.height <= 0 ||
        light.light_rect.x + light.light_rect.width > img.cols ||
        light.light_rect.y + light.light_rect.height > img.rows)
    {
        RCLCPP_WARN(rclcpp::get_logger("ArmorTracker"), "Invalid ROI or image for color detection");
        return;
    }

    cv::Mat roi = img(light.light_rect);
    cv::Mat mask = cv::Mat::zeros(roi.size(), CV_8UC1);

    // 优化：由于直接操作了原函数的 cv::pointPolygonTest(多边形逐像素检测），这是一个复杂度极高的操作。
    // 改为创建小型 Mask，并通过 cv::mean (C++底层高强度向量化操作) 进行直接取均值对比，速度飙升。
    std::vector<cv::Point> offset_contour(contour.size());
    for (size_t i = 0; i < contour.size(); i++)
    {
        offset_contour[i] = cv::Point(contour[i].x - light.light_rect.x, contour[i].y - light.light_rect.y);
    }

    std::vector<std::vector<cv::Point>> contours_to_draw = {offset_contour};
    cv::drawContours(mask, contours_to_draw, 0, cv::Scalar(255), cv::FILLED);

    // 计算掩膜区域内BGR通道的均值
    cv::Scalar mean_color = cv::mean(roi, mask);

    // 由于是BGR格式，mean_color[2]对应红色Red，mean_color[0]对应蓝色Blue
    light.color = mean_color[2] > mean_color[0] ? 2 : 0;
    LightBarVector.push_back(light);
}

void ArmorTracker::publish_to_serial_driver(double yaw, double pitch, const std::vector<cv::Point2f> &armorPoints)
{
    armor_interfaces::msg::SerialDriver serial_msg;
    // RCLCPP_INFO(this->get_logger(), "Yaw: %.2f, Pitch: %.2f", yaw, pitch);
    serial_msg.yaw = yaw;
    if (pitch < -180 || pitch > 180)
    {
        RCLCPP_INFO(this->get_logger(), "Pitch angle out of range: %f", pitch);
        serial_msg.pitch = 0.0;
    }
    else
    {
        serial_msg.pitch = -pitch;
    }
    if (armorPoints.empty())
    {
        serial_msg.symbol = 0;
    }
    else
    {
        serial_msg.symbol = 1;
    }

    serial_driver_publisher_->publish(serial_msg);
}

void ArmorTracker::receiveDataCallback(const armor_interfaces::msg::SerialReceiveData msg)
{
    // 【必须加上这句】给关节状态打上当前的时间戳！
    joint_state_msg_.header.stamp = this->now();

    joint_state_msg_.position[0] = msg.yaw * M_PI / 180.0;
    joint_state_msg_.position[1] = msg.pitch * M_PI / 180.0;

    // std::cout << "从串口获取的yaw和pitch - Yaw: " << msg.yaw
    //           << " Pitch: " << msg.pitch << std::endl;
    tf_camera_to_world_publisher_->publish(joint_state_msg_);
}

void ArmorTracker::msgCallback(const geometry_msgs::msg::PointStamped::SharedPtr point_ptr)
{
    RCLCPP_INFO(this->get_logger(), "msgCallback 被调用！来自帧: %s", point_ptr->header.frame_id.c_str());
    try
    {
        {
            std::lock_guard<std::mutex> lock(target_point_out_mutex_);
            tf_buffer_->transform(*point_ptr, target_point_out, target_frame_);
        }
        RCLCPP_INFO(
            this->get_logger(), "坐标点相对于%s的坐标:(%.2f,%.2f,%.2f)",
            target_frame_.c_str(),
            target_point_out.point.x,
            target_point_out.point.y,
            target_point_out.point.z);
    }
    catch (tf2::TransformException &ex)
    {
        RCLCPP_WARN(
            this->get_logger(), "变换失败: %s\n", ex.what());
    }
}

rcl_interfaces::msg::SetParametersResult ArmorTracker::parametersCallback(const std::vector<rclcpp::Parameter> &parameters)
{
    rcl_interfaces::msg::SetParametersResult result;
    result.successful = true;
    result.reason = "success";

    for (const auto &param : parameters)
    {
        if (param.get_name() == "blue_thre_value")
        {
            blue_thre_value_ = param.as_int();
            RCLCPP_INFO(this->get_logger(), "蓝方阈值更新为: %d", blue_thre_value_);
        }
        else if (param.get_name() == "red_thre_value")
        {
            red_thre_value_ = param.as_int();
            RCLCPP_INFO(this->get_logger(), "红方阈值更新为: %d", red_thre_value_);
        }
        else if (param.get_name() == "setting")
        {
            setting_ = param.as_int();
            RCLCPP_INFO(this->get_logger(), "攻击阵营更新为: %d (0=蓝, 2=红)", setting_);
        }
        else if (param.get_name() == "exposure_time")
        {
            exposure_time_ = param.as_int();
            if (galaxy_camera_.isReady())
            {
                galaxy_camera_.setExposureTime(exposure_time_); // 大恒 SDK 支持运行时实时改写底层寄存器
            }
            RCLCPP_INFO(this->get_logger(), "相机曝光时间更新为: %d", exposure_time_);
        }
        else if (param.get_name() == "gain")
        {
            gain_ = param.as_double();
            if (galaxy_camera_.isReady())
            {
                galaxy_camera_.setGain(gain_);
            }
            RCLCPP_INFO(this->get_logger(), "相机增益更新为: %.2f", gain_);
        }
        else if (param.get_name() == "gamma")
        {
            double new_gamma = param.as_double();
            if (galaxy_camera_.isReady())
            {
                galaxy_camera_.setGamma(new_gamma);
            }
            RCLCPP_INFO(this->get_logger(), "相机 Gamma 更新为: %.2f", new_gamma);
        }
    }
    return result;
}