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

    void training (const std::vector<Features> &features, const cv::Mat &labels, const std::string &model,
            const int k_fold, const int max_iter, const double epsilon) {

        const cv::Mat dataFeatures = toMat(features);

        std::cout << "FT size " << dataFeatures.rows << "*" << dataFeatures.cols << std::endl;

        std::cout << "SVM Initialization..." << std::endl;

        cv::Ptr<cv::ml::SVM> svm = cv::ml::SVM::create();

        if (phd::io::exists(model)) {
            try {
                svm = svm->load(model);
            } catch (cv::Exception &ex) {
                std::cerr << ex.what() << std::endl;
            }

        } else {
            std::cerr << std::endl << "No saved model has been found... Training will start from scratch." << std::endl;

            svm->setType(cv::ml::SVM::C_SVC);
            svm->setKernel(cv::ml::SVM::RBF);
//            svm->setC(phd::devices::accelerometer::std_coefficients.C);
//            svm->setGamma(0.1);
            svm->setTermCriteria(cv::TermCriteria(cv::TermCriteria::MAX_ITER, max_iter, epsilon));
        }

        std::cout << "SVM Training..." << std::endl;

        auto train_data = cv::ml::TrainData::create(dataFeatures, cv::ml::ROW_SAMPLE, labels);

        svm->trainAuto(train_data,
                       k_fold,
                       cv::ml::SVM::getDefaultGrid(cv::ml::SVM::C),
                       cv::ml::SVM::getDefaultGrid(cv::ml::SVM::GAMMA), //cv::ml::ParamGrid(0.00001, 1.0, 0.00001),
                       cv::ml::SVM::getDefaultGrid(cv::ml::SVM::P),
                       cv::ml::SVM::getDefaultGrid(cv::ml::SVM::NU),
                       cv::ml::SVM::getDefaultGrid(cv::ml::SVM::COEF),
                       cv::ml::SVM::getDefaultGrid(cv::ml::SVM::DEGREE),
                       true);

//        svm->train(train_data);

        std::cout << "Finished." << std::endl;

        svm->save(model);

        std::cout << "Model Saved." << std::endl;
    }

    cv::Mat classify(const std::vector<Features> &features, const std::string &model) {

        std::cout << "Loading SVM from " << model << std::endl;

        cv::Mat labels = cv::Mat(0, 0, CV_32SC1);

        cv::Ptr<cv::ml::SVM> svm = cv::ml::SVM::load(model);

        const cv::Mat data = toMat(features);

        if (svm->isTrained()) {
            std::cout << "Classifying... ";
            svm->predict(data, labels);
            std::cout << "Finished." << std::endl;
            transpose(labels, labels);
        } else {
            std::cerr << "SVM Classifier is not trained";
            exit(-1);
        }

        svm->clear();

        return labels;
    }
}