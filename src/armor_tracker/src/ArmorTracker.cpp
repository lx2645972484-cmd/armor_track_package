#include "ArmorTracker.hpp"
#include <algorithm>
#include <cmath>
#include <chrono>

ArmorTracker::ArmorTracker() : Node("armor_tracker")
{
    int num_cores = cv::getNumberOfCPUs();
    cv::setNumThreads(num_cores);

    // 初始化调试图像发布者
    debug_image_pub_ = image_transport::create_publisher(this, "/tracker/debug_image");

    // 初始化发布者和TF广播器
    armor_info_publisher = this->create_publisher<armor_interfaces::msg::ArmorInfo>("armor_info", 10);
    center_publisher = this->create_publisher<geometry_msgs::msg::PointStamped>("center_point", 10);

    // 发布相机到世界坐标系的初始姿态
    tf_camera_to_world_publisher_ = this->create_publisher<armor_interfaces::msg::JointState>("/joint_states", 10);
    joint_state_msg_.name = {"yaw_joint", "pitch_joint"};
    joint_state_msg_.position = {0.0, 0.0};
    tf_camera_to_world_publisher_->publish(joint_state_msg_);

    // TF相关参数
    target_frame_ = "turret_base_link"; // 赋值给成员变量，不是声明局部变量
    typedef std::chrono::duration<int> seconds_type;
    seconds_type buffer_timeout(1);

    // 初始化TF监听器和缓冲区
    tf_buffer_ = std::make_unique<tf2_ros::Buffer>(this->get_clock());
    auto timer_interface = std::make_shared<tf2_ros::CreateTimerROS>(
        this->get_node_base_interface(),
        this->get_node_timers_interface());
    tf_buffer_->setCreateTimerInterface(timer_interface);
    transform_listener_ = std::make_shared<tf2_ros::TransformListener>(*tf_buffer_);

    // 订阅目标点
    // point_sub_.subscribe(this, "target_point");

    // // 初始化目标点发布者
    // target_point_pub_ = this->create_publisher<geometry_msgs::msg::PointStamped>("target_point", 10);

    // // 创建消息过滤器过滤被处理的数据；
    // tf2_filter_ = std::make_shared<tf2_ros::MessageFilter<geometry_msgs::msg::PointStamped>>(
    //     point_sub_, *tf_buffer_, target_frame_, 100, this->get_node_logging_interface(),
    //     this->get_node_clock_interface(), buffer_timeout);
    // // 为消息过滤器注册转换坐标点数据的回调函数。
    // tf2_filter_->registerCallback(&ArmorTracker::msgCallback, this);

    // 初始化串口数据发布者和订阅者
    serial_driver_publisher_ = this->create_publisher<armor_interfaces::msg::SerialDriver>("/tracker/target", rclcpp::SensorDataQoS());

    // 订阅串口数据
    camera_to_world_sub_ = this->create_subscription<armor_interfaces::msg::SerialReceiveData>(
        "/tracker/receive_data", rclcpp::SensorDataQoS(),
        std::bind(&ArmorTracker::receiveDataCallback, this, std::placeholders::_1));

    // std::string path = "/home/eee/VSCCC/ACE3.2/2.mp4";
    // cap.open(path);

    // 初始化相机
    if (!galaxy_camera_.init())
    {
        // 初始化失败
        RCLCPP_INFO(this->get_logger(), "相机初始化失败");
        return;
    }
    galaxy_camera_.startCapture();        // 手动开始采集线程
    galaxy_camera_.setExposureTime(5000); // 设置曝光时间为5000微秒

    // if (!camera_ready_)
    // {
    //     RCLCPP_ERROR(this->get_logger(), "相机初始化失败");
    //     return;
    // }

    last_time_ = this->now();

    RCLCPP_INFO(this->get_logger(), "相机初始化成功");

    this->declare_parameter<int>("setting", 3);
    setting_ = this->get_parameter("setting").as_int();
    RCLCPP_INFO(this->get_logger(), "对方阵营参数: %d (0=蓝色, 2=红色)", setting_);
}

void ArmorTracker::run()
{
    const float ARMOR_WIDTH = 0.135f;                    // 灯条中心间距 135mm
    const float ARMOR_HEIGHT = 0.055f;                   // 灯条高度 55mm
    const float HALF_ARMOR_WIDTH = ARMOR_WIDTH / 2.0f;   // 0.0675m
    const float HALF_ARMOR_HEIGHT = ARMOR_HEIGHT / 2.0f; // 0.0275m

    cv::Mat camera_matrix = (cv::Mat_<double>(3, 3) << 2374.54248, 0.0, 698.85288,
                             0.0, 2377.53648, 520.8649,
                             0.0, 0.0, 1.0);

    // 畸变系数
    cv::Mat dist_coeffs = (cv::Mat_<double>(1, 5) << -0.059743, 0.355479, -0.000625, 0.001595, 0.000000);

    // 图像相关参数
    cv::Mat img, imgClose;
    galaxy_camera::ImageData img_data;
    int blue_thre_value = 50; // 二值化查找蓝色装甲板光条参数
    int red_thre_value = 50;

    // int frame_width = 0, frame_height = 0;
    // double fps = 0.0, current_time_ms = 0.0;

    double yaw;
    double pitch;

    std::vector<cv::Point2f> armorPoints;
    armorPoints.reserve(4);

    if (!galaxy_camera_.isReady())
    {
        std::cout << "我草，哥们，相机炸了" << std::endl;
        return;
    }

    // --- 优化：将循环中不变的对象提前声明，避免高频的动态内存开辟销毁 ---
    const std::vector<cv::Point3f> object_points = {
        cv::Point3f(-HALF_ARMOR_WIDTH, -HALF_ARMOR_HEIGHT, 0.0f), // 左上
        cv::Point3f(HALF_ARMOR_WIDTH, -HALF_ARMOR_HEIGHT, 0.0f),  // 右上
        cv::Point3f(HALF_ARMOR_WIDTH, HALF_ARMOR_HEIGHT, 0.0f),   // 右下
        cv::Point3f(-HALF_ARMOR_WIDTH, HALF_ARMOR_HEIGHT, 0.0f)   // 左下
    };

    cv::Mat rvec_zero = cv::Mat::zeros(3, 1, CV_64F);
    cv::Mat tvec_zero = cv::Mat::zeros(3, 1, CV_64F);
    std::vector<double> current_yaws;
    current_yaws.reserve(32);
    std::vector<std::vector<cv::Point>> contours;

    while (galaxy_camera_.isReady() && rclcpp::ok())
    {
        armorPoints.clear();
        LightBarVector.clear();
        current_yaws.clear(); // 优化：不清空底层内存只重置size，比重新声明快
        contours.clear();

        rclcpp::Time current_time = this->now(); // 获取当前时间

        rclcpp::Duration delta_t = current_time - last_time_; // 计算与上次处理消息的时间间隔

        last_time_ = current_time; // 更新上次处理消息的时间

        double delta_time = delta_t.seconds(); // 获取时间间隔，单位为秒

        if (!galaxy_camera_.getImage(img_data))
        {
            // 给相机一点时间出图，避免 CPU 100% 空转
            std::this_thread::sleep_for(std::chrono::milliseconds(2));
            continue; // 不要 break，继续等待下一帧
        }

        img = cv::Mat(img_data.height, img_data.width, CV_8UC3, img_data.data.data());

        cv::cvtColor(img, img, cv::COLOR_RGB2BGR);

        if (img.empty())
        {
            std::cout << "我去哥们你这个怎么是空图像啊" << std::endl;
            break; // 图像空了应该退出循环
        }

        imgClose = pp.PreprocessImg(img, blue_thre_value, red_thre_value, setting_);

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
            double angle = rect.angle;

            if (w < h)
            {
                std::swap(w, h);
                angle += 90.0;
            }

            if (w * h < 200)
                continue;
            if (w / h < 1.5)
                continue;

            LightBar light;
            light.rect = rect;
            light.angle = angle;
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
                if (angle_cha > 10)
                    continue;

                double w1 = std::max(l1.rect.size.height, l1.rect.size.width);
                double w2 = std::max(l2.rect.size.height, l2.rect.size.width);
                double ave_width = (w1 + w2) / 2;

                double h_cha = std::abs(w1 - w2);
                if (h_cha / ave_width > 0.5)
                    continue;

                double dist = mtl.getMyDistance(l1.center, l2.center);
                if (dist / ave_width < 1.5 || dist / ave_width > 6)
                    continue;

                if (l1.color != l2.color)
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
                lbp.getLightBarEndpoints(leftBar.rect, tl, bl);
                lbp.getLightBarEndpoints(rightBar.rect, tr, br);

                armorPoints = {tl, tr, br, bl};

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

                // 优化：将四个单独的绘制循环整合为一个，减少分支预测失败和循环开销
                for (int k = 0; k < 4; k++)
                {
                    int next_k = (k + 1) % 4;
                    cv::line(img, lp[k], lp[next_k], cv::Scalar(0, 255, 0), 2);
                    cv::line(img, rp[k], rp[next_k], cv::Scalar(0, 255, 0), 2);
                    cv::line(img, armorPoints[k], armorPoints[next_k], cv::Scalar(0, 255, 0), 2);

                    std::string coord = cv::format("(%d,%d)", (int)armorPoints[k].x, (int)armorPoints[k].y);
                    cv::putText(img, coord, armorPoints[k], cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 255), 1);
                    cv::circle(img, armorPoints[k], 2, cv::Scalar(0, 0, 255), -1);
                }

                cv::Mat rvec, tvec;
                bool pnp_success = cv::solvePnP(object_points, armorPoints, camera_matrix, dist_coeffs, rvec, tvec);

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
                                          camera_matrix, dist_coeffs, fit_center_2d);

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

                double distance = cv::norm(tvec);

                // 优化：用底层内存指针获取数据远比调用 .at<double>() 快
                const double *tvec_data = tvec.ptr<double>();
                double tx = tvec_data[2];
                double ty = -tvec_data[0];
                double tz = -tvec_data[1];

                geometry_msgs::msg::PointStamped target1; // 发布装甲板中心点到TF
                // target1.header.frame_id = "camera_link";       // 注意：这里的坐标系是相机坐标系，不是世界坐标系

                // // 优化：直接使用 ROS 2 的时间接口获取当前时间，避免了每帧调用 std::chrono 的系统调用开销
                // target1.header.stamp = this->now();
                // // 优化：直接使用 ROS 2 的坐标接口发布数据，避免了构造 ROS 2 消息的开销
                // target1.point.x = tx;
                // target1.point.y = ty;
                // target1.point.z = tz;

                // // 发布目标点到相机坐标系的变换TF，供其他节点使用
                // target_point_pub_->publish(target1);

                geometry_msgs::msg::PointStamped point_in;
                point_in.header.frame_id = "camera_link";
                point_in.header.stamp = rclcpp::Time(0);
                point_in.point.x = tx;
                point_in.point.y = ty;
                point_in.point.z = tz;

                geometry_msgs::msg::PointStamped point_out_local;

                try
                {
                    tf_buffer_->transform(point_in, point_out_local, target_frame_, tf2::durationFromSec(0.0));

                    this->point_out = point_out_local;
                }
                catch (const tf2::TransformException &ex)
                {
                    // 如果查不到，说明底盘位姿没跟上，丢弃这帧的数据，直接 continue 抓取下一张图
                    RCLCPP_WARN_THROTTLE(this->get_logger(), *this->get_clock(), 1000,
                                         "坐标变换未就绪，丢弃本帧: %s", ex.what());
                    continue;
                }

                // camera -> base_link 的变换只需要查询一次
                auto camera_transformStamped = tf_buffer_->lookupTransform("camera_link", "turret_base_link", tf2::TimePointZero);

                // 至此，我们获得了相机到底座的变换关系，以及装甲板中心点在底座坐标系下的坐标！！！

                yaw = std::atan2(tx, tz) * 180 / M_PI;
                current_yaws.push_back(yaw);

                double horiz = std::sqrt(tx * tx + tz * tz);
                pitch = std::atan2(-ty, horiz) * 180 / M_PI;

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

        if (KalmanInit == false)
        {
            kalman.init(point_out, delta_time);
            KalmanInit = true;
        }

        if (KalmanInit == true)
        {
            kalman.predict(delta_time);
            kalman.update(point_out);
        }

        geometry_msgs::msg::PointStamped kalman_point;
        kalman_point.header.frame_id = "turret_base_link";
        kalman_point.header.stamp = this->now();
        kalman_point.point.x = kalman.X(0);
        kalman_point.point.y = kalman.X(1);
        kalman_point.point.z = kalman.X(2);

        double z = kalman.X(0);
        double y = -kalman.X(2);
        double x = -kalman.X(1);

        geometry_msgs::msg::PointStamped test;
        test.header.frame_id = "camera_link";
        test.header.stamp = this->now();
        test.point.x = x;
        test.point.y = y;
        test.point.z = z;

        // std::cout << "卡尔曼滤波预测点坐标 (相机坐标系): X=" << test.point.x
        //<< " Y=" << test.point.y
        // << " Z=" << test.point.z << std::endl;

        std::vector<cv::Point3f> out_center_3d;
        out_center_3d.emplace_back(x, y, z);
        std::vector<cv::Point2f> out_center_2d;

        // 优化：复用初始化的 zeros 矩阵，杜绝每帧动态生成 Mat 的操作
        cv::projectPoints(out_center_3d, rvec_zero, tvec_zero,
                          camera_matrix, dist_coeffs, out_center_2d);

        cv::circle(img, out_center_2d[0], 8, cv::Scalar(0, 0, 255), -1);

        // geometry_msgs::msg::PointStamped test;

        // tf_buffer_->transform(kalman_point, test, "camera_link");

        double yaw_test = std::atan2(kalman_point.point.y, kalman_point.point.x) * 180.0 / M_PI;
        double horiz_test = std::sqrt(kalman_point.point.x * kalman_point.point.x + kalman_point.point.y * kalman_point.point.y);
        double pitch_test = std::atan2(kalman_point.point.z, horiz_test) * 180.0 / M_PI;

        // RCLCPP_INFO(this->get_logger(), "卡尔曼滤波预测点角度 (世界坐标系): X=%.2f Y=%.2f Z=%.2f", yaw_test, pitch_test, horiz_test);


        if (debug_image_pub_.getNumSubscribers() > 0)
        {
            std_msgs::msg::Header header;
            // 获取当前时间戳
            header.stamp = this->now();
            // 填入相机的坐标系 ID，跟你 TF 发布的一致即可
            header.frame_id = "camera_link";

            // cv_bridge::CvImage 将 header、图像编码格式("bgr8")和原始 cv::Mat 包装起来
            // toImageMsg() 将其转化为 sensor_msgs::msg::Image
            auto img_msg = cv_bridge::CvImage(header, "bgr8", img).toImageMsg();

            // 发布图像！
            debug_image_pub_.publish(img_msg);
        }

        publish_to_serial_driver(yaw_test, pitch_test, armorPoints);

        // char key = cv::waitKey(30);
        // if (key == 27)
        // {
        //     break;
        // }
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

    // 优化：彻底砍掉原先为了包围盒去临时创建 std::vector 的昂贵内存分配与回收
    float expand_factor = 20.0f;
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

        // 直接进行数值范围校验，速度提升极其明显
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
    joint_state_msg_.position[0] = msg.yaw;
    joint_state_msg_.position[1] = msg.pitch;

    std::cout << "从串口获取的yaw和pitch - Yaw: " << msg.yaw
              << " Pitch: " << msg.pitch << std::endl;
    tf_camera_to_world_publisher_->publish(joint_state_msg_);
}

void ArmorTracker::msgCallback(const geometry_msgs::msg::PointStamped::SharedPtr point_ptr)
{
    RCLCPP_INFO(this->get_logger(), "msgCallback 被调用！来自帧: %s", point_ptr->header.frame_id.c_str());
    try
    {
        tf_buffer_->transform(*point_ptr, point_out, target_frame_);
        RCLCPP_INFO(
            this->get_logger(), "坐标点相对于%s的坐标:(%.2f,%.2f,%.2f)",
            target_frame_.c_str(),
            point_out.point.x,
            point_out.point.y,
            point_out.point.z);
    }
    catch (tf2::TransformException &ex)
    {
        RCLCPP_WARN(
            this->get_logger(), "变换失败: %s\n", ex.what());
    }
}