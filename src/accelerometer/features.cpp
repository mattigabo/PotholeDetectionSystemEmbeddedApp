//
// Created by Xander on 01/11/2018.
//

#include "accelerometer/features.h"
#include <math.h>
#include <algorithm>
#include <iostream>
#include <opencv2/ml.hpp>
#include <phdetection/io.hpp>
#include <accelerometer/features.h>


namespace phd {
    namespace devices {
        namespace accelerometer {
            namespace data {

                std::vector<float> getWindow(const std::vector<float> &stream, const int window, const int slider) {

                    if (slider > stream.size()) {
                        return std::vector<float>(0);
                    } else if (stream.size() < slider + window) {
                        return std::vector<float>(stream.begin() + slider, stream.end());
                    } else {
                        return std::vector<float>(stream.begin() + slider, stream.begin() + slider + window);
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

                    return sum / (array.size() - 1);
                }

                float std_dev(const float variance) {
                    return sqrt(variance);
                }

                float coefficient_of_variation(const float std_dev, const float mean) {
                    return std_dev / mean;
                }

                float max_min_difference(const std::vector<float> &array) {
                    float max = *std::max_element(array.begin(), array.end());
                    float min = *std::min_element(array.begin(), array.end());

                    return max - min;
                }

                bool assign_confidence(const float value, const float threshold, float &assigned_confidence) {
                    if (value > threshold) {
                        assigned_confidence = std_coefficients.high_confidence_score;
                        return true;
                    } else {
                        assigned_confidence = std_coefficients.low_confidence_score;
                        return false;
                    }
                }

                Features getFeatures(const std::vector<float> &window) {

                    Features ft = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0};

                    ft.mean = mean(window);
                    ft.variance = variance(window, ft.mean);
                    ft.std_dev = std_dev(ft.variance);
                    ft.coefficient_of_variation = coefficient_of_variation(ft.std_dev, ft.mean);
                    ft.max_min_diff = max_min_difference(window);

                    if (assign_confidence(ft.mean, std_thresholds.mean_threshold, ft.mean_confidence))
                        ft.thresholds_overpass_count++;

                    if (assign_confidence(ft.std_dev, std_thresholds.std_dev_threshold, ft.std_dev_confidence))
                        ft.thresholds_overpass_count++;

                    if (assign_confidence(ft.coefficient_of_variation, std_thresholds.relative_std_dev_threshold,
                            ft.coefficient_of_variation_confidence))
                        ft.thresholds_overpass_count++;

                    if (assign_confidence(ft.max_min_diff, std_thresholds.max_min_diff_threshold,
                            ft.max_min_diff_confidence))
                        ft.thresholds_overpass_count++;

                    ft.confidences_sum = ft.mean_confidence + ft.std_dev_confidence + ft.coefficient_of_variation_confidence +
                            ft.max_min_diff_confidence;

                    if (assign_confidence(ft.confidences_sum, std_thresholds.sum_threshold,
                            ft.confidences_sum_confidence))
                        ft.thresholds_overpass_count++;

                    return ft;
                }

                cv::Mat toMat(const Features &features) {

                    auto data = cv::Mat(1, n_features, CV_32FC1);

                    data.at<float>(0, 0) = features.mean;
                    data.at<float>(0, 1) = features.mean_confidence;
                    data.at<float>(0, 2) = features.std_dev;
                    data.at<float>(0, 3) = features.std_dev_confidence;
                    data.at<float>(0, 4) = features.variance;
                    data.at<float>(0, 5) = features.coefficient_of_variation;
                    data.at<float>(0, 6) = features.coefficient_of_variation_confidence;
                    data.at<float>(0, 7) = features.max_min_diff;
                    data.at<float>(0, 8) = features.max_min_diff_confidence;
                    data.at<float>(0, 9) = features.confidences_sum;
                    data.at<float>(0, 10) = features.confidences_sum_confidence;
                    data.at<float>(0, 11) = static_cast<float>(features.thresholds_overpass_count);

                    return data;
                }

                cv::Mat toMat(const std::vector<Features> &features) {
                    cv::Mat data(0, n_features, CV_32FC1);

                    for (int i = 0; i < features.size(); i++) {

                        data.push_back(toMat(features[i]));

                    }

//                    std::cout << data.rows << "|" << data.cols << std::endl;

                    return data;
                }

                std::pair<cv::Mat, cv::Mat> findMinMaxFeatures(cv::Mat train_data){

                    auto minFeatures = cv::Mat(1, 12, CV_32FC1);
                    auto maxFeatures = cv::Mat(1, 12, CV_32FC1);

                    for (int i = 0; i < train_data.cols; i++) {
                        double tmpMin, tmpMax;
                        cv::minMaxLoc(train_data.col(i), &tmpMin, &tmpMax);

                        auto min = static_cast<float>(tmpMin);
                        auto max = static_cast<float>(tmpMax);

//                        minFeatures.at<float>(0, i) = min;
//                        maxFeatures.at<float>(0, i) = max;

                        if ( i == 0 || i == 2 || i == 4 || i == 5 || i == 7) {

                            minFeatures.at<float>(0, i) = min;
                            maxFeatures.at<float>(0, i) = max;

                        } else if (i == 1 || i == 3 || i == 6 || i == 8 || i == 10) {

                            minFeatures.at<float>(0, i) = std_coefficients.low_confidence_score;
                            maxFeatures.at<float>(0, i) = std_coefficients.high_confidence_score;

                        } else if (i == 9) {
                            minFeatures.at<float>(0, i) = 4.0f * std_coefficients.low_confidence_score;
                            maxFeatures.at<float>(0, i) = 4.0f * std_coefficients.high_confidence_score;
                        } else {
                            minFeatures.at<float>(0, i) = 0.0f;
                            maxFeatures.at<float>(0, i) = 5.0f;
                        }
                    }

                    return std::make_pair(minFeatures, maxFeatures);
                }


                cv::Mat
                normalize(const cv::Mat &features, const double minValue, const double maxValue, const int type) {

                    cv::Mat normalized_features = cv::Mat(features.rows, features.cols, features.type());

                    for (int i = 0; i < features.cols; ++i) {
                        cv::normalize(features.col(i), normalized_features.col(i), minValue, maxValue, type);
                    }

                    return normalized_features;
                }

                void cross_train(const cv::Mat &features, const cv::Mat &labels, const std::string &model,
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
                            params.k_fold,
                            cv::ml::SVM::getDefaultGrid(cv::ml::SVM::C),
                            cv::ml::SVM::getDefaultGrid(cv::ml::SVM::GAMMA),
                            cv::ml::SVM::getDefaultGrid(cv::ml::SVM::P),
                            cv::ml::SVM::getDefaultGrid(cv::ml::SVM::NU),
                            cv::ml::SVM::getDefaultGrid(cv::ml::SVM::COEF),
                            cv::ml::SVM::getDefaultGrid(cv::ml::SVM::DEGREE),
                            params.balanced_folding);

                    std::cout << "Finished." << std::endl;

                    svm->save(model);

                    std::cout << "Model saved @ " << model << std::endl;
                }

                void train(const cv::Mat &features, const cv::Mat &labels, const std::string &model,
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

                    std::cout << "Model saved @ " << model << std::endl;
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

                Features toFeatures(const std::vector<phd::devices::accelerometer::Acceleration> &window,
                        const Axis &acceleration_axis) {

                    std::vector<float> axis_values;

                    switch (acceleration_axis) {
                        case Axis::X:
                            std::for_each(window.begin(), window.end(),
                                          [&axis_values](phd::devices::accelerometer::Acceleration a) {
                                              axis_values.push_back(a.X);
                                          });
                            break;
                        case Axis::Y:
                            std::for_each(window.begin(), window.end(),
                                          [&axis_values](phd::devices::accelerometer::Acceleration a) {
                                              axis_values.push_back(a.Y);
                                          });
                            break;
                        default:
                            std::for_each(window.begin(), window.end(),
                                          [&axis_values](phd::devices::accelerometer::Acceleration a) {
                                              axis_values.push_back(a.Z);
                                          });
                    }

                    return getFeatures(axis_values);
                }

                std::vector<Features> toFeaturesVector(const cv::Mat &features) {

                    auto v = std::vector<Features>();

                    for (int i = 0; i < features.rows; ++i) {

                        auto ft = toFeatures(features.row(i));

                        v.push_back(ft);
                    }

                    return v;
                }

                Features toFeatures(const cv::Mat &features) {

                    return {
                        features.at<float>(0, 0),
                        features.at<float>(0, 1),
                        features.at<float>(0, 2),
                        features.at<float>(0, 3),
                        features.at<float>(0, 4),
                        features.at<float>(0, 5),
                        features.at<float>(0, 6),
                        features.at<float>(0, 7),
                        features.at<float>(0, 8),
                        features.at<float>(0, 9),
                        features.at<float>(0, 10),
                        static_cast<int>(features.at<float>(0, 11))
                    };
                }
            }
        }
    }
}