// ArmorTracker.cpp
#include "ArmorTracker.hpp"

ArmorTracker::ArmorTracker() : Node("armor_tracker")
{
    // 初始化发布者和TF广播器
    armor_info_publisher = this->create_publisher<armor_interfaces::msg::ArmorInfo>("armor_info", 10);
    center_publisher = this->create_publisher<geometry_msgs::msg::PointStamped>("center_point", 10);
    tf_broadcaster = std::make_unique<tf2_ros::TransformBroadcaster>(*this);
    serial_driver_publisher_= this->create_publisher<armor_interfaces::msg::SerialDriver>("/tracker/target", 10);

    // std::string path = "/home/eee/VSCCC/ACE3.2/2.mp4";
    // cap.open(path);

    if (!galaxy_camera_.init()) 
    {
        // 初始化失败
        RCLCPP_INFO(this->get_logger(), "相机初始化失败");
        return;
    }
    galaxy_camera_.startCapture();  // 手动开始采集线程
    galaxy_camera_.setExposureTime(2000); // 设置曝光时间为2000微秒
    
    // if (!camera_ready_)
    // {
    //     RCLCPP_ERROR(this->get_logger(), "相机初始化失败");
    //     return;
    // }

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
    int blue_thre_value = 190; // 二值化查找蓝色装甲板光条参数
    int red_thre_value = 100;

    // cv::Mat imgFunc = drawer.DrawMyFunctionLine();

    int frame_width = 0, frame_height = 0;
    double fps = 0.0, current_time_ms = 0.0;

    double yaw;
    double pitch;

    std::vector<cv::Point2f> armorPoints;

    if (!galaxy_camera_.isReady())
    {
        std::cout << "我草，哥们，相机炸了" << std::endl;
        return;
    }

    // vp.getVideoinformation(hCamera, frame_width, frame_height, fps, current_time_ms);
    // std::cout << "[Info] 相机分辨率: " << frame_width << " x " << frame_height
    //           << " | FPS: " << fps << std::endl;

    // 开始相机采集（确保只调用一次）
    // CameraPlay(hCamera);

    while (galaxy_camera_.isReady() && rclcpp::ok())
    {
        armorPoints.clear();
        LightBarVector.clear();
        double delta_time = 1.0 / fps;
        std::vector<double> current_yaws;

        if (!galaxy_camera_.getImage(img_data))
            break;

        img = cv::Mat(img_data.height, img_data.width, CV_8UC3, img_data.data.data());

        if (img.empty())
        {
            std::cout << "我去哥们你这个怎么是空图像啊" << std::endl;
            break; // 图像空了应该退出循环
        }

        imgClose = pp.PreprocessImg(img, blue_thre_value, red_thre_value, setting_);

        std::vector<std::vector<cv::Point>> contours;
        cv::findContours(imgClose, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

        // std::vector<LightBar> target;

        for (int i = 0; i < contours.size(); i++)
        {
            if (contours[i].size() < 4)
            {
                continue;
            }

            cv::RotatedRect rect = cv::minAreaRect(contours[i]);
            cv::Rect light_rect = cv::boundingRect(contours[i]);
            // 灯条的外接矩形(light_rect用于roi分离红蓝灯条)

            double w = rect.size.width;
            double h = rect.size.height;
            double angle = rect.angle;
            // width height

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

                // 1.角度差
                double angle_cha = abs(l1.angle - l2.angle);
                if (angle_cha > 10)
                    continue;

                // 2.高度差（这里用长边作为“高度”）
                double w1 = std::max(l1.rect.size.height, l1.rect.size.width);
                double w2 = std::max(l2.rect.size.height, l2.rect.size.width);
                double ave_width = (w1 + w2) / 2;

                double h_cha = abs(w1 - w2);
                if (h_cha / ave_width > 0.5)
                    continue;

                // 3.水平间隔
                double dist = mtl.getMyDistance(l1.center, l2.center);
                if (dist / ave_width < 1.5 || dist / ave_width > 6)
                    continue;

                // 4.垂直间隔
                // double y_cha = abs(l1.center.y - l2.center.y);
                // if (y_cha / ave_width > 0.3)
                //     continue;

                // 5.颜色检测
                if (l1.color != l2.color)
                {
                    continue;
                }

                // 6.灯条包含检测
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
                // top bottom left right

                // 装甲板四个角点组成的vector
                armorPoints = {tl, tr, br, bl};

                double armor_width = tr.x - tl.x;
                double armor_height = bl.y - tl.y;

                if (armor_width <= 0 || armor_height <= 0)
                {
                    continue; // 无效矩形，跳过
                }
                cv::Rect rect(tl.x, tl.y, armor_width, armor_height);

                // 裁剪到图像内部
                if (rect.x < 0)
                    rect.x = 0;
                if (rect.y < 0)
                    rect.y = 0;
                if (rect.x + rect.width > img.cols)
                    rect.width = img.cols - rect.x;
                if (rect.y + rect.height > img.rows)
                    rect.height = img.rows - rect.y;

                // 检查矩形是否有效
                if (rect.width <= 0 || rect.height <= 0)
                {
                    // 跳过该装甲板，不进行后续处理
                    continue;
                }

                for (int k = 0; k < 4; k++)
                {
                    cv::line(img, lp[k], lp[(k + 1) % 4], cv::Scalar(0, 255, 0), 2);
                }

                for (int k = 0; k < 4; k++)
                {
                    cv::line(img, rp[k], rp[(k + 1) % 4], cv::Scalar(0, 255, 0), 2);
                }

                // 取ROI
                cv::Mat roi = img(rect);

                for (int k = 0; k < 4; k++)
                {
                    cv::line(img, armorPoints[k], armorPoints[(k + 1) % 4], cv::Scalar(0, 255, 0), 2);
                }

                for (int k = 0; k < 4; k++)
                {
                    std::string coord = "(" + std::to_string((int)armorPoints[k].x) + "," + std::to_string((int)armorPoints[k].y) + ")";
                    cv::putText(img, coord, armorPoints[k], cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 255), 1);
                    cv::circle(img, armorPoints[k], 2, cv::Scalar(0, 0, 255), -1);
                }

                // PnP解算
                std::vector<cv::Point3f> object_points = {
                    cv::Point3f(-HALF_ARMOR_WIDTH, -HALF_ARMOR_HEIGHT, 0.0f), // 左上
                    cv::Point3f(HALF_ARMOR_WIDTH, -HALF_ARMOR_HEIGHT, 0.0f),  // 右上
                    cv::Point3f(HALF_ARMOR_WIDTH, HALF_ARMOR_HEIGHT, 0.0f),   // 右下
                    cv::Point3f(-HALF_ARMOR_WIDTH, HALF_ARMOR_HEIGHT, 0.0f)   // 左下
                };

                cv::Mat rvec, tvec;
                bool pnp_success = cv::solvePnP(object_points, armorPoints, camera_matrix, dist_coeffs, rvec, tvec);

                if (pnp_success)
                {
                    // 向处理旋转中心的对象添加观测量
                    centerSolver.addObservation(tvec, rvec);

                    // 尝试解算
                    cv::Point3f calculated_center;
                    bool fit_success = centerSolver.solve(calculated_center);

                    if (fit_success)
                    {
                        // 将计算出的 3D 中心投影回 2D 画面进行绘制
                        std::vector<cv::Point3f> fit_center_3d = {calculated_center};
                        std::vector<cv::Point2f> fit_center_2d;

                        // calculated_center 已经是相机坐标系下的绝对坐标了！！！！！！
                        // 所以投影时，rvec 和 tvec 都要给 0 ！！！！！不需要不需要不需要！！！！(因为坐标变换已经包含在点里了)
                        cv::projectPoints(fit_center_3d, cv::Mat::zeros(3, 1, CV_64F), cv::Mat::zeros(3, 1, CV_64F),
                                          camera_matrix, dist_coeffs, fit_center_2d);

                        // 绘制拟合出来的中心（画个绿色的点）
                        cv::circle(img, fit_center_2d[0], 8, cv::Scalar(0, 255, 0), -1);
                        cv::putText(img, "Fit Center", fit_center_2d[0] + cv::Point2f(10, -10),
                                    cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 255, 0), 2);

                        // 输出拟合半径看看准不准
                        float radius = centerSolver.getLastRadius(calculated_center);
                        std::string r_text = "R: " + std::to_string(radius).substr(0, 4) + "m";
                        cv::putText(img, r_text, fit_center_2d[0] + cv::Point2f(10, 10),
                                    cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 255, 0), 2);
                    }
                    else
                    {
                        // 拟合没准备好时，显示“Collecting Data...”
                        cv::putText(img, "Collecting Data...", cv::Point2f(50, 50),
                                    cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 0, 255), 2);
                    }
                }

                // 距离：相机到装甲板中心的欧氏距离（米）
                double distance = cv::norm(tvec);

                // yaw 水平偏角
                // 之所以用了.at是因为AI告诉我，使用.at是更加安全的访问方式
                yaw = std::atan2(tvec.at<double>(0), tvec.at<double>(2)) * 180 / M_PI;
                current_yaws.push_back(yaw);
                // horiz 这是水平距离
                double horiz = std::sqrt(tvec.at<double>(0) * tvec.at<double>(0) + tvec.at<double>(2) * tvec.at<double>(2));
                // pitch：垂直偏角
                pitch = std::atan2(-tvec.at<double>(1), horiz) * 180 / M_PI;

                // 找装甲板中心
                // 四个点加起来除以四就是我的结果
                cv::Point2f armor_center(0, 0);
                for (const auto &p : armorPoints)
                    armor_center += p;
                armor_center /= 4.0f;

                // 显示信息（黄色文字）
                cv::Point2f text_pos = armor_center + cv::Point2f(-150, -40);
                std::string info_text = cv::format("Yaw: %.1f°  Pitch: %.1f°  Dist: %.2f", yaw, pitch, distance);
                cv::putText(img, info_text, text_pos, cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 255, 255), 2);
            }
        }

        if (!current_yaws.empty())
        {
            mltkalman.update(current_yaws, delta_time);
        }

        armorTracker_.time = current_time_ms/1000.0;

        if (mltkalman.is_active1())
        {
            armorTracker_.yaw = mltkalman.get_yaw1();
            armor_info_publisher->publish(armorTracker_);
        }

        if (mltkalman.is_active2())
        {
            armorTracker_.yaw = mltkalman.get_yaw2();
            armor_info_publisher->publish(armorTracker_);
        }

        publish_to_serial_driver(yaw, pitch, armorPoints);

        // drawer.DrawMyFxxkingPoint(imgFunc, time, yaw, 1.5);

        cv::imshow("imgclose", imgClose);
        cv::imshow("myimg", img);
        // cv::imshow("func", imgFunc);

        char key = cv::waitKey(30);
        if (key == 27)
        {
            break;
        }
        // writer.write(img);
    }

    galaxy_camera_.stop();
    // cap.release();
    // writer.release();
    cv::destroyAllWindows();
}

void ArmorTracker::publish_armor_msg()
{
    armor_interfaces::msg::ArmorInfo armor_info_msg; // 创建装甲信息消息 frame
    armor_info_msg.yaw = armorTracker_.yaw;          // 设置yaw

    armor_info_publisher->publish(armor_info_msg); // 发布装甲信息
}

bool ArmorTracker::containLight(const LightBar &light_1, const LightBar &light_2)
{
    // 检查输入数据的有效性 - 修正：cv::Point2f 没有 empty() 方法
    // 改为检查点坐标是否在合理范围内
    if (light_1.center.x < 0 || light_1.center.y < 0 ||
        light_2.center.x < 0 || light_2.center.y < 0)
    {
        RCLCPP_WARN(this->get_logger(), "灯条中心点坐标无效");
        return false;
    }

    // 获取两个灯条的中心点
    cv::Point2f center1 = light_1.center;
    cv::Point2f center2 = light_2.center;

    // 计算包围两个中心点的最小矩形
    cv::Rect bounding_rect = cv::boundingRect(std::vector<cv::Point2f>{center1, center2});

    // 增加矩形区域的宽度和高度，以便更好地检测其他灯条
    int expand_factor = 20; // 可以根据实际情况调整这个因子
    bounding_rect.x -= expand_factor;
    bounding_rect.y -= expand_factor;
    bounding_rect.width += 2 * expand_factor;
    bounding_rect.height += 2 * expand_factor;

    // 遍历所有灯条,检查是否有其他灯条位于该矩形内
    // 注意：这里需要访问存储灯条的容器，根据上下文应该是 LightBarVector
    for (const auto &test_light : LightBarVector)
    {
        // 跳过正在配对的两个灯条本身
        if (test_light.center == center1 || test_light.center == center2)
        {
            continue;
        }

        // 检查灯条的中心点是否在矩形内
        if (bounding_rect.contains(test_light.center))
        {
            return true; // 发现有其他灯条存在
        }
    }

    return false; // 没有其他灯条
}

void ArmorTracker::ColorDetect(std::vector<cv::Point> contour, LightBar light, cv::Mat img)
{
    int sum_r = 0;
    int sum_b = 0;

    if (img.empty() ||
        light.light_rect.x < 0 || light.light_rect.y < 0 ||
        light.light_rect.width <= 0 || light.light_rect.height <= 0 ||
        light.light_rect.x + light.light_rect.width > img.cols ||
        light.light_rect.y + light.light_rect.height > img.rows)
    {
        // 记录错误日志，跳过本次处理或返回
        RCLCPP_WARN(rclcpp::get_logger("ArmorTracker"), "Invalid ROI or image for color detection");
        return;
    }
    cv::Mat roi = img(light.light_rect);
    for (int i = 0; i < roi.rows; i++)
    {
        for (int j = 0; j < roi.cols; j++)
        {
            if (cv::pointPolygonTest(contour, cv::Point2f(j + light.light_rect.x, i + light.light_rect.y), false) >= 0)
            {
                sum_r += roi.at<cv::Vec3b>(i, j)[2];
                sum_b += roi.at<cv::Vec3b>(i, j)[0];
            }
        }
    }

    light.color = sum_r > sum_b ? 2 : 0;
    LightBarVector.push_back(light);
}

void ArmorTracker::publish_to_serial_driver(double yaw, double pitch,std::vector<cv::Point2f> armorPoints)
{
    armor_interfaces::msg::SerialDriver serial_msg;

    serial_msg.yaw = -yaw;
    serial_msg.pitch = -pitch;
    if(armorPoints.empty())
    {
        serial_msg.symbol = 0;
    }
    else
    {
        serial_msg.symbol = 1;
    }

    serial_driver_publisher_->publish(serial_msg);
}