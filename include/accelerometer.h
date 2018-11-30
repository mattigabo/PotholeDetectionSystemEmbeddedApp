//
// Created by Xander on 01/11/2018.
//

#ifndef POTHOLEDETECTIONSYSTEMEMBEDDEDAPP_ACCELEROMETER_H
#define POTHOLEDETECTIONSYSTEMEMBEDDEDAPP_ACCELEROMETER_H

#include <vector>
#include <string>
#include <opencv2/core.hpp>
#include "configurationutils.h"

namespace phd::devices::accelerometer {

    const float g = 9.8196f; // m/s^2
    const int  n_features = 12;

    typedef struct Features {
        float mean;
        float mean_confidence;
        float std_dev;
        float std_dev_confidence;
        float variance; // Has no confidence score
        float relative_std_dev;
        float relative_std_dev_confidence;
        float max_min_diff;
        float max_min_diff_confidence;
        float confidences_sum;
        float confidences_sum_confidence;
        int thresholds_overpass_count;
    } Features;

    typedef struct Thresholds {
        float mean_threshold;
        float std_dev_threshold;
        float relative_std_dev_threshold;
        float max_min_diff_threshold;
        float sum_threshold;
    } Thresholds;

    const Thresholds std_thresholds = {
        .mean_threshold = g * 0.3f,
        .std_dev_threshold = g * 0.15f,
        .relative_std_dev_threshold = g * 0.015f,
        .max_min_diff_threshold = g * 0.2f,
        .sum_threshold = 3
    };

    typedef struct Coefficients {
        float high_confidence_score;
        float low_confidence_score;
        int windows_size;
        int C;
    } Coefficients;

    const Coefficients std_coefficients = {
        .high_confidence_score = 0.8,
        .low_confidence_score = 0.2,
        .windows_size = 30,
        .C = 10
    };

    /**
     *
     * @param stream All or part of the values recorded by the Accelerometer on a certain axis
     * @param window The number of values to get, that is the size of the returning vector
     * @param slider The index of the first element of the window
     * @return The values found inside the sliding window
     */
    std::vector<float> getWindow(const std::vector<float> &stream, const int window, const int slider);

    /**
     *
     *
     * @param stream All or part of the values recorded by the Accelerometer on a certain axis
     * @param window The number of values to get from the back of the stream, that is the size of the returning vector
     * @return The values found inside the sliding window
     */
    std::vector<float> getWindow(const std::vector<float> &stream, const int window);

    /**
     *
     * @param window The array of accelerometer values from which extract the features
     * @return The compound structure encapsulating the features
     */
    Features getFeatures(const std::vector<float> &window);

    /**
     *
     * @param features
     * @param labels
     * @param model
     */
    void cross_training(
            const cv::Mat &features,
            const cv::Mat &labels,
            const std::string &model,
            const phd::configurations::SVMParams params);

    /**
     *
     * @param features
     * @param labels
     * @param model
     */
    void training(
            const cv::Mat &features,
            const cv::Mat &labels,
            const std::string &model,
            const phd::configurations::SVMParams params);

    /**
     *
     * @param features
     * @param model
     * @return
     */
    cv::Mat classify(const cv::Mat &features, const std::string &model);

    /**
     *
     * @param features
     * @return
     */
    cv::Mat toMat(const std::vector<Features> &features);


    cv::Mat normalize(const cv::Mat &features, const double minValue, const double maxValue, const int type);
}

#endif //POTHOLEDETECTIONSYSTEMEMBEDDEDAPP_ACCELEROMETER_H
