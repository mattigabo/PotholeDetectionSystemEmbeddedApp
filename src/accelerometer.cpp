//
// Created by Xander on 01/11/2018.
//

#include "accelerometer.h"
#include <math.h>
#include <algorithm>
#include <iostream>

namespace phd::devices::accelerometer {

    std::vector<double> getWindow(const std::vector<double> stream, const int window, const int slider) {

        if (slider > stream.size()) {
            return std::vector<double>(0);
        } else if (stream.size() < slider + window) {
            return std::vector<double> (stream.begin() + slider, stream.end());
        } else {
            return std::vector<double> (stream.begin() + slider, stream.begin() + slider + window);
        }
    }

    std::vector<double> getWindow(const std::vector<double> stream, const int window) {

        if (stream.size() <= window) {
            return stream;
        } else {
            return std::vector<double>(stream.begin() + stream.size() - window, stream.end());
        }
    }

    double mean(const std::vector<double> array) {
        double sum = 0;
        for (double d : array) {
            sum += d;
        }

        return sum / array.size();
    }

    double variance(const std::vector<double> array, const double mean) {
        double sum = 0;
        for (double d : array) {
            sum = sum + (d - mean) * (d - mean);
        }

        return sum / array.size();
    }

    double std_dev(const double variance) {
        return sqrt(variance);
    }

    double relative_std_dev(const double std_dev, const double mean) {
        return std_dev / mean;
    }

    double max_min_difference(const std::vector<double> array) {
        double max = *std::max_element(array.begin(), array.end());
        double min = *std::min_element(array.begin(), array.end());

        return max - min;
    }

    double assign_confidence(const double value, const double threshold, bool& check) {
        double d = std_coefficients.low_confidence_score;
        check = false;
        if (value > threshold) {
            d = std_coefficients.high_confidence_score;
            check = true;
        }

        return d;
    }

    Features getFeatures(const std::vector<double> window) {

        Features ft = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0};

        ft.mean = mean(window);
        ft.variance = variance(window, ft.mean);
        ft.std_dev = std_dev(ft.variance);
        ft.relative_std_dev = relative_std_dev(ft.std_dev, ft.mean);
        ft.max_min_diff = max_min_difference(window);

        bool check = false;

        ft.mean_confidence = assign_confidence(ft.mean, std_thresholds.mean_threshold, check);
        if (check) ft.thresholds_overpass_count++;
        ft.std_dev_confidence = assign_confidence(ft.std_dev, std_thresholds.std_dev_threshold, check);
        if (check) ft.thresholds_overpass_count++;
        ft.relative_std_dev_confidence = assign_confidence(ft.relative_std_dev, std_thresholds.relative_std_dev_threshold, check);
        if (check) ft.thresholds_overpass_count++;
        ft.max_min_diff_confidence = assign_confidence(ft.max_min_diff, std_thresholds.max_min_diff_threshold, check);
        if (check) ft.thresholds_overpass_count++;

        ft.confidences_sum = ft.mean_confidence + ft.std_dev_confidence + ft.relative_std_dev_confidence + ft.max_min_diff_confidence;

        ft.confidences_sum_confidence = assign_confidence(ft.confidences_sum, std_thresholds.sum_threshold, check);
        if (check) ft.thresholds_overpass_count++;

        return ft;
    }

}