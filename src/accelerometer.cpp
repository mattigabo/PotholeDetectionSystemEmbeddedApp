//
// Created by Xander on 01/11/2018.
//

#include "accelerometer.h"
#include <math.h>
#include <algorithm>
#include <iostream>
#include <opencv2/ml.hpp>
#include <phdetection/io.hpp>

namespace phd::devices::accelerometer {

    std::vector<float> getWindow(const std::vector<float> &stream, const int window, const int slider) {

        if (slider > stream.size()) {
            return std::vector<float>(0);
        } else if (stream.size() < slider + window) {
            return std::vector<float> (stream.begin() + slider, stream.end());
        } else {
            return std::vector<float> (stream.begin() + slider, stream.begin() + slider + window);
        }
    }

    std::vector<float> getWindow(const std::vector<float> &stream, const int window) {

        if (stream.size() <= window) {
            return stream;
        } else {
            return std::vector<float>(stream.begin() + stream.size() - window, stream.end());
        }
    }

    float mean(const std::vector<float> array) {
        float sum = 0;
        for (float d : array) {
            sum += d;
        }

        return sum / array.size();
    }

    float variance(const std::vector<float> &array, const float mean) {
        float sum = 0;
        for (float d : array) {
            sum = sum + (d - mean) * (d - mean);
        }

        return sum / array.size();
    }

    float std_dev(const float variance) {
        return sqrt(variance);
    }

    float relative_std_dev(const float std_dev, const float mean) {
        return std_dev / mean;
    }

    float max_min_difference(const std::vector<float> &array) {
        float max = *std::max_element(array.begin(), array.end());
        float min = *std::min_element(array.begin(), array.end());

        return max - min;
    }

    float assign_confidence(const float value, const float threshold, bool& check) {
        float d = std_coefficients.low_confidence_score;
        check = false;
        if (value > threshold) {
            d = std_coefficients.high_confidence_score;
            check = true;
        }

        return d;
    }

    Features getFeatures(const std::vector<float> &window) {

        Features ft = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0};

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

    cv::Mat toMat(const std::vector<Features> &features) {
        cv::Mat data(static_cast<int>(features.size()), n_features, CV_32FC1);

        for (int i = 0; i < features.size(); i++) {
            data.at<float>(i, 0) = features[i].mean;
            data.at<float>(i, 1) = features[i].mean_confidence;
            data.at<float>(i, 2) = features[i].std_dev;
            data.at<float>(i, 3) = features[i].std_dev_confidence;
            data.at<float>(i, 4) = features[i].variance;
            data.at<float>(i, 5) = features[i].relative_std_dev;
            data.at<float>(i, 6) = features[i].relative_std_dev_confidence;
            data.at<float>(i, 7) = features[i].max_min_diff;
            data.at<float>(i, 8) = features[i].max_min_diff_confidence;
            data.at<float>(i, 9) = features[i].confidences_sum;
            data.at<float>(i, 10) = features[i].confidences_sum_confidence;
            data.at<float>(i, 11) = features[i].thresholds_overpass_count;
        }
        return data;
    }

    cv::Mat normalize(const cv::Mat &features, const double minValue, const double maxValue, const int type) {

        cv::Mat normalized_features = cv::Mat(features.rows, features.cols, features.type());

        for (int i = 0; i < features.cols; ++i) {
            cv::normalize(features.col(i), normalized_features.col(i), minValue, maxValue, type);
        }

        return normalized_features;
    }

    void cross_training(const cv::Mat &features, const cv::Mat &labels, const std::string &model,
                        const phd::configurations::SVMParams params) {

        std::cout << "FT size " << features.rows << "*" << features.cols << std::endl;

        std::cout << "SVM Initialization..." << std::endl;

        cv::Ptr<cv::ml::SVM> svm = cv::ml::SVM::create();

        std::cerr << "Training will start from scratch." << std::endl;

        svm->setType(params.type);
        svm->setKernel(params.kernel);
        svm->setTermCriteria(cv::TermCriteria(cv::TermCriteria::MAX_ITER, params.max_iter, params.epsilon));

        std::cout << "SVM Cross-Training..." << std::endl;

        auto train_data = cv::ml::TrainData::create(features, cv::ml::ROW_SAMPLE, labels);

        svm->trainAuto(train_data,
                       params.kfold,
                       cv::ml::SVM::getDefaultGrid(cv::ml::SVM::C),
                       cv::ml::SVM::getDefaultGrid(cv::ml::SVM::GAMMA),
                       cv::ml::SVM::getDefaultGrid(cv::ml::SVM::P),
                       cv::ml::SVM::getDefaultGrid(cv::ml::SVM::NU),
                       cv::ml::SVM::getDefaultGrid(cv::ml::SVM::COEF),
                       cv::ml::SVM::getDefaultGrid(cv::ml::SVM::DEGREE),
                       true);

        std::cout << "Finished." << std::endl;

        svm->save(model);

        std::cout << "Model saved @ " << model <<std::endl;
    }

    void training(const cv::Mat &features, const cv::Mat &labels, const std::string &model,
                  const phd::configurations::SVMParams params) {

        std::cout << "FT size " << features.rows << "*" << features.cols << std::endl;

        std::cout << "SVM Initialization..." << std::endl;

        cv::Ptr<cv::ml::SVM> svm = cv::ml::SVM::create();

        std::cerr << "Training will start from scratch." << std::endl;

        svm->setType(params.type);
        svm->setKernel(params.kernel);
        svm->setC(params.C);
        svm->setGamma(params.gamma);
        svm->setTermCriteria(cv::TermCriteria(cv::TermCriteria::MAX_ITER, params.max_iter, params.epsilon));

        std::cout << "SVM Training..." << std::endl;

        auto train_data = cv::ml::TrainData::create(features, cv::ml::ROW_SAMPLE, labels);

        svm->train(train_data);

        std::cout << "Finished." << std::endl;

        svm->save(model);

        std::cout << "Model saved @ " << model <<std::endl;
    }

    cv::Mat classify(const cv::Mat &features, const std::string &model) {

        cv::Mat labels = cv::Mat(0, 0, CV_32SC1);

        cv::Ptr<cv::ml::SVM> svm = cv::ml::SVM::load(model);

        std::cout << "SVM loaded from model " << model << std::endl;

        if (svm->isTrained()) {
            std::cout << "Classifying... ";
            svm->predict(features, labels);
            std::cout << "Finished." << std::endl;
            transpose(labels, labels);
        } else {
            std::cerr << "SVM Classifier is not trained" << std::endl;
            exit(-1);
        }

        svm->clear();

        return labels;
    }
}