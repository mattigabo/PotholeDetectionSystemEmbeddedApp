//
// Created by Xander on 01/11/2018.
//

#ifndef POTHOLEDETECTIONSYSTEMEMBEDDEDAPP_ACCELEROMETER_H
#define POTHOLEDETECTIONSYSTEMEMBEDDEDAPP_ACCELEROMETER_H

#include <vector>

namespace phd::devices::accelerometer {

    const double g = 9.8196; // m/s^2

    typedef struct Features {
        double mean;
        double mean_confidence;
        double std_dev;
        double std_dev_confidence;
        double variance; // Has no confidence score
        double relative_std_dev;
        double relative_std_dev_confidence;
        double max_min_diff;
        double max_min_diff_confidence;
        double confidences_sum;
        double confidences_sum_confidence;
        int thresholds_overpass_count;
    } Features;

    typedef struct Thresholds {
        double mean_threshold;
        double std_dev_threshold;
        double relative_std_dev_threshold;
        double max_min_diff_threshold;
        double sum_threshold;
    } Thresholds;

    const Thresholds std_thresholds = {
        .mean_threshold = g * 0.3,
        .std_dev_threshold = g * 0.15,
        .relative_std_dev_threshold = g * 0.015,
        .max_min_diff_threshold = g * 0.2,
        .sum_threshold = 3
    };

    typedef struct Coefficients {
        double high_confidence_score = 0.8;
        double low_confidence_score = 0.2;
        int windows_size = 30;
        int C = 10;
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
    std::vector<double> getWindow(const std::vector<double> stream, const int window, const int slider);

    /**
     *
     *
     * @param stream All or part of the values recorded by the Accelerometer on a certain axis
     * @param window The number of values to get from the back of the stream, that is the size of the returning vector
     * @return The values found inside the sliding window
     */
    std::vector<double> getWindow(const std::vector<double> stream, const int window);

    /**
     *
     * @param window The array of accelerometer values from which extract the features
     * @return The compound structure encapsulating the features
     */
    Features getFeatures(const std::vector<double> window);

}

#endif //POTHOLEDETECTIONSYSTEMEMBEDDEDAPP_ACCELEROMETER_H
