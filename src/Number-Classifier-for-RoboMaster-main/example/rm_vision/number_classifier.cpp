// Copyright 2022 Chen Jun
// Licensed under the MIT License.

// OpenCV
#include <opencv2/core.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/dnn.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>

// STL
#include <algorithm>
#include <cstddef>
#include <fstream>
#include <map>
#include <string>
#include <vector>

#include "armor_detector/armor.hpp"
#include "armor_detector/number_classifier.hpp"

namespace rm_auto_aim
{
NumberClassifier::NumberClassifier(
  const std::string & model_path, const std::string & label_path, const double thre,
  const std::vector<std::string> & ignore_classes)
: threshold(thre), ignore_classes_(ignore_classes)
{
  net_ = cv::dnn::readNetFromONNX(model_path);

  std::ifstream label_file(label_path);
  std::string line;
  while (std::getline(label_file, line)) {
    class_names_.push_back(line);
  }
}

void NumberClassifier::extractNumbers(const cv::Mat & src, std::vector<Armor> & armors)
{
  const std::vector<cv::Point2f> points_modified{
      {0.0, 7.0}, {0.0, 21.0}, {28.0, 21.0}, {28.0, 7.0}};
  const cv::Size roi_size{28, 28};
  for (auto &armor : armors) {
    std::vector<cv::Point2f> points{
        armor.left_light.top, armor.left_light.bottom, armor.right_light.bottom,
        armor.right_light.top};
    auto transform_matrix =
        cv::getPerspectiveTransform(points, points_modified);
    cv::Mat roi;
    cv::warpPerspective(rgb_img, roi, transform_matrix, roi_size);
    cv::cvtColor(roi, roi, cv::COLOR_BGR2GRAY);
    armor.number_img = roi;
  }
}

float NumberClassifier::sigmoid(float x) {
  return 1 / (1 + exp(-x));
}

cv::Mat NumberClassifier::softmax(cv::Mat input) {
  float max_prob = *std::max_element(input.begin<float>(), input.end<float>());
  cv::Mat softmax_prob;
  cv::exp(input - max_prob, softmax_prob);
  float sum = static_cast<float>(cv::sum(softmax_prob)[0]);
  return softmax_prob / sum;
}

void NumberClassifier::classify(std::vector<Armor> & armors)
{
  for (auto & armor : armors) {
    // Get input
    cv::Mat tmp;
    roi.convertTo(tmp, CV_32FC1);
    tmp = tmp / 255.0;
    // Set input
    cv::Mat blob = cv::dnn::blobFromImage(tmp, 1., cv::Size(28, 28));
    net_.setInput(blob);
    // Get output
    cv::Mat outputs = net_.forward();
    // Process output
    float obj_pb = sigmoid(*outputs.begin<float>());
    if (obj_pb > conf_thresh_) {
      cv::Mat cls_score = cv::Mat(10, 1, CV_32FC1);
      std::copy(outputs.begin<float>() + 1, outputs.end<float>(),
                cls_score.begin<float>());
      cls_score = softmax(cls_score);
      // Get highest confidence and corresponding id
      float confidence = 0;
      int id = 0;
      int idx = 0;
      for (auto iter = cls_score.begin<float>();
           iter != cls_score.end<float>(); ++iter) {
        if (*iter > confidence) {
          confidence = *iter;
          id = idx;
        }
        ++idx;
      }
      // Id start from 1 to 9 while label_id start from 0
      int label_id = id - 1;
      // Sentry contains two ids, 7 of old version, 9 of new version
      if (id == 6 || id == 9) {
        label_id = 6;
      }
      // Outpost has id 7
      if (id == 7) {
        label_id = 5;
      }  
      armor.confidence = confidence;
      armor.number = class_names_[id];
      td::stringstream result_ss;
      result_ss << armor.number << ": " << std::fixed << std::setprecision(1)
                << armor.confidence * 100.0 << "%";
      armor.classfication_result = result_ss.str();
    } else {
      armor.number = class_names_[8];
      armor.confidence = 1.0;
    }
  }

  armors.erase(
    std::remove_if(
      armors.begin(), armors.end(),
      [this](const Armor & armor) {
        if (armor.confidence < threshold) {
          return true;
        }

        for (const auto & ignore_class : ignore_classes_) {
          if (armor.number == ignore_class) {
            return true;
          }
        }

        bool mismatch_armor_type = false;
        if (armor.armor_type == LARGE) {
          mismatch_armor_type =
            armor.number == "Outpost" || armor.number == "2" || armor.number == "Guard";
        } else if (armor.armor_type == SMALL) {
          mismatch_armor_type = armor.number == "1" || armor.number == "Base";
        }
        return mismatch_armor_type;
      }),
    armors.end());
}


}  // namespace rm_auto_aim
