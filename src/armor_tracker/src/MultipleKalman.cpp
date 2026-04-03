#include "MultipleKalman.h"
#include <iostream>
#include <algorithm>
#include <cmath>

double MultipleKalman::min_angle_diff(double a, double b)
{
    double diff = std::fabs(a - b);
    return diff;
}

void MultipleKalman::update(const std::vector<double> &measurements, double delta_time)
{

    // 最大允许的角度匹配误差
    const double MAX_MATCH_DIFF = 4;

    // 前置处理
    if (active2 && !active1)
    {
        std::swap(k1, k2);
        std::swap(pre_yaw1, pre_yaw2);
        active1 = true;
        active2 = false;
    }

    if (measurements.empty())
    {
        active1 = false;
        active2 = false;
        return;
    }

    // 预测阶段
    double pred_yaw1 = 0.0;
    double pred_yaw2 = 0.0;

    if (active1)
    {
        pred_yaw1 = k1.X(0);
        k1.predict();
    }
    if (active2)
    {
        pred_yaw2 = k2.X(0);
        k2.predict();
    }

    // 数据关联与更新
    if (measurements.size() == 1)
    {
        double meas = measurements[0];

        // 两个都未激活,初始化 k1
        if (!active1 && !active2)
        {
            k1.init(meas, delta_time, meas);

            active1 = true;
            return;
        }

        // 只有一个活跃,更新或重新初始化
        if (active1 && !active2)
        {
            double diff = min_angle_diff(meas, pred_yaw1);
            if (diff > MAX_MATCH_DIFF)
            {
                // 跳变太大，重新初始化
                k1.init(meas, delta_time, meas);
            }
            else
            {
                k1.update(meas, delta_time, pre_yaw1);
                pre_yaw1 = k1.X(0);
            }
            return;
        }

        // 两个都活跃,竞争和门限防御
        if (active1 && active2)
        {
            double diff1 = min_angle_diff(meas, pred_yaw1);
            double diff2 = min_angle_diff(meas, pred_yaw2);

            if (diff1 > MAX_MATCH_DIFF && diff2 > MAX_MATCH_DIFF)
            {
                // 两个预测都太远就全部丢弃，重新初始化 k1
                active1 = false;
                active2 = false;
                k1.init(meas, delta_time, meas);

                active1 = true;
            }
            else if (diff1 <= MAX_MATCH_DIFF && diff2 > MAX_MATCH_DIFF)
            {
                // 只有 k1 合理
                k1.update(meas, delta_time, pre_yaw1);
                pre_yaw1 = k1.X(0);
                active2 = false;
            }
            else if (diff2 <= MAX_MATCH_DIFF && diff1 > MAX_MATCH_DIFF)
            {
                // 只有 k2 合理就更新 k2 并交换为主
                k2.update(meas, delta_time, pre_yaw2);
                pre_yaw2 = k2.X(0);
                active1 = false;
                std::swap(k1, k2);
                std::swap(pre_yaw1, pre_yaw2);
                active1 = true;
            }
            else // 两个都合理就选更近的
            {
                if (diff1 <= diff2)
                {
                    k1.update(meas, delta_time, pre_yaw1);
                    pre_yaw1 = k1.X(0);
                    active2 = false;
                }
                else
                {
                    k2.update(meas, delta_time, pre_yaw2);
                    pre_yaw2 = k2.X(0);
                    active1 = false;
                    std::swap(k1, k2);
                    std::swap(pre_yaw1, pre_yaw2);
                    active1 = true;
                }
            }
        }
    }
    else if (measurements.size() == 2)
    {
        double m1 = measurements[0];
        double m2 = measurements[1];

        // 都没激活 初始化两个
        if (!active1 && !active2)
        {
            k1.init(m1, delta_time, m1);

            active1 = true;

            k2.F = k1.F;
            k2.H = k1.H;
            k2.Q = k1.Q;
            k2.R = k1.R;
            k2.P = k1.P;
            k2.I = k1.I;
            k2.init(m2, delta_time, m2);

            active2 = true;
            return;
        }

        // 只有一个活跃 更新旧的 + 初始化新的
        if (active1 && !active2)
        {
            double d1 = min_angle_diff(m1, pred_yaw1);
            double d2 = min_angle_diff(m2, pred_yaw1);
            bool m1_is_closer = (d1 <= d2);
            double old_diff = m1_is_closer ? d1 : d2;

            if (old_diff > MAX_MATCH_DIFF)
            {
                // 旧目标已消失，用两个测量重新初始化
                active1 = false;
                k1.init(m1, delta_time, m1);

                active1 = true;

                k2.F = k1.F;
                k2.H = k1.H;
                k2.Q = k1.Q;
                k2.R = k1.R;
                k2.P = k1.P;
                k2.I = k1.I;
                k2.init(m2, delta_time, m2);

                active2 = true;
            }
            else
            {
                double old_meas = m1_is_closer ? m1 : m2;
                double new_meas = m1_is_closer ? m2 : m1;

                k1.update(old_meas, delta_time, pre_yaw1);
                pre_yaw1 = k1.X(0);

                k2.F = k1.F;
                k2.H = k1.H;
                k2.Q = k1.Q;
                k2.R = k1.R;
                k2.P = k1.P;
                k2.I = k1.I;
                k2.init(new_meas, delta_time, new_meas);

                active2 = true;
            }
            return;
        }

        // 两个都活跃，匈牙利匹配，门限
        if (active1 && active2)
        {
            double d11 = min_angle_diff(m1, pred_yaw1);
            double d12 = min_angle_diff(m1, pred_yaw2);
            double d21 = min_angle_diff(m2, pred_yaw1);
            double d22 = min_angle_diff(m2, pred_yaw2);

            bool chose_better = (d11 + d22 < d12 + d21);
            double chosen_d_k1 = chose_better ? d11 : d21;
            double chosen_d_k2 = chose_better ? d22 : d12;
            double chosen_m_k1 = chose_better ? m1 : m2;
            double chosen_m_k2 = chose_better ? m2 : m1;

            if (chosen_d_k1 > MAX_MATCH_DIFF && chosen_d_k2 > MAX_MATCH_DIFF)
            {
                // 两个都失败，重新初始化
                active1 = false;
                active2 = false;
                k1.init(m1, delta_time, m1);

                active1 = true;
                k2.F = k1.F;
                k2.H = k1.H;
                k2.Q = k1.Q;
                k2.R = k1.R;
                k2.P = k1.P;
                k2.I = k1.I;
                k2.init(m2, delta_time, m2);

                active2 = true;
            }
            else if (chosen_d_k1 > MAX_MATCH_DIFF)
            {
                // k1 失败，只保留 k2 并交换k1为主
                k2.update(chosen_m_k2, delta_time, pre_yaw2);
                pre_yaw2 = k2.X(0);
                active1 = false;
                std::swap(k1, k2);
                std::swap(pre_yaw1, pre_yaw2);
                active1 = true;
            }
            else if (chosen_d_k2 > MAX_MATCH_DIFF)
            {
                // k2 失败，只更新 k1
                k1.update(chosen_m_k1, delta_time, pre_yaw1);
                pre_yaw1 = k1.X(0);
                active2 = false;
            }
            else
            {
                // 两个都成功，按最优方案更新
                k1.update(chosen_m_k1, delta_time, pre_yaw1);
                pre_yaw1 = k1.X(0);
                k2.update(chosen_m_k2, delta_time, pre_yaw2);
                pre_yaw2 = k2.X(0);
            }
        }
    }
}