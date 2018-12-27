//
// Created by Matteo Gabellini on 27/10/2018.
//
#include <executionmodes.h>

#include <chrono>
#include <vector>
#include <string>
#include <thread>
#include <time.h>
#include <assert.h>
#include <fstream>

#include <phdetection/ontologies.hpp>
#include <phdetection/core.hpp>
#include <phdetection/svm.hpp>
#include <phdetection/bayes.hpp>

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/istreamwrapper.h>

#include <serialport/SerialPort.h>
#include <camera.h>
#include <networking.h>
#include <accelerometer/features.h>
#include <accelerometer/utils.h>
#include <accelerometer/accelerometer.h>

#include <execution/utils.h>
#include <execution/observables/accelerometer.h>
#include <execution/observables/gps.h>

#include <fingerprint.h>

using namespace rapidjson;
using namespace std;

cv::Mat extractFeaturesAndClassify(const string &method, const string &bayes_model, const string &svm_model, cv::Mat &image,
                               const phd::io::Configuration &phdConfig) {

    //cout << endl << "---------------" << image << endl;

    auto features = phd::getFeatures(image, phdConfig);

    cv::Mat labels;

    try {
        labels = phd::classify(method, svm_model, bayes_model, features);
    } catch(phd::UndefinedMethod &ex)  {
        cerr << "ERROR: " << ex.what() << endl;
        exit(-1);
    }

    cout << "LABELS: " << labels << endl;

    return labels;
}

void runObservationMode(bool poison_pill,
        phd::devices::gps::GPSDataStore* gpsDataStore,
        phd::io::Configuration phdConfig,
        phd::configurations::CVArgs cvConfig,
        phd::configurations::ServerConfig serverConfig){

    std::cout << "Capture Device ID: " << cv::VideoCaptureAPIs::CAP_ANY << std::endl;

    while(!poison_pill) {

        std::string position = toJSON(gpsDataStore->fetch(), std::string());

        cv::Mat image = phd::devices::camera::fetch(cv::VideoCaptureAPIs::CAP_ANY);

        if (cvConfig.rotate) {
            cv::rotate(image, image, cv::ROTATE_180);
        }

//        cv::imshow("Capture", image);
//        waitKey(0);

        cv::Mat labels = extractFeaturesAndClassify(cvConfig.method, cvConfig.bayes, cvConfig.svm, image, phdConfig);
        if(labels.rows != 0) {
            labels = labels.row(0);

            vector<int> l(labels.ptr<int>(0), labels.ptr<int>(0) + labels.cols);

            if (std::find(l.begin(), l.end(), 1) != l.end() ||
                std::find(l.begin(), l.end(), 2) != l.end()) {

                sendDataToServer(position, serverConfig);
            }
        }
        std::this_thread::sleep_for(chrono::milliseconds(500));
    }
}


void testFingerPrintCalculation(){

    std::string uid = fingerprint::getUID();

    std::cout << "Fp: " << uid << std::endl;

    std::cout << "Validation: " << fingerprint::validateUID(uid) << std::endl;
}

void trainAccelerometerMlAlgorithm(const phd::configurations::MLOptions<phd::configurations::SVMParams> &args,
                                   const bool cross_validate) {

    std::vector<phd::devices::accelerometer::data::Features> features;
    std::vector<int> labels;

    auto sliding_function = [](int window) { return window - 1; };

    if (phd::io::is_dir(args.train_set.data())) {
        vector<cv::String> globs;
        cv::glob(args.train_set + "/*.json", globs);

        for (const string ds : globs) {
            const auto rawData = phd::devices::accelerometer::utils::readJSONDataset(ds);
            phd::devices::accelerometer::utils::toFeatures(rawData, "z", sliding_function, features, labels);
        }

    } else if (phd::io::is_file(args.train_set.data())) {

        const auto rawData = phd::devices::accelerometer::utils::readJSONDataset(args.train_set);
        phd::devices::accelerometer::utils::toFeatures(rawData, "z", sliding_function, features, labels);

    } else {
        cerr << "Undefined directory or file " << args.train_set << endl;
        exit(-3);
    }

    const cv::Mat train_data = toMat(features);
    const cv::Mat normalized_train_data =
            phd::devices::accelerometer::data::normalize(
                    train_data,
                    args.norm_range.first,
                    args.norm_range.second,
                    args.norm_method
                );

    if (cross_validate) {
        phd::devices::accelerometer::data::cross_train(
                normalized_train_data,
                cv::Mat(cv::Size(1, static_cast<int>(labels.size())), CV_32SC1, labels.data()),
                args.model,
                args.params.second
        );

    } else {
        phd::devices::accelerometer::data::train(
                normalized_train_data,
                cv::Mat(cv::Size(1, static_cast<int>(labels.size())), CV_32SC1, labels.data()),
                args.model,
                args.params.second
        );
    }

    features.clear();
    labels.clear();
}

void testAccelerometerMlAlgorithm(const phd::configurations::MLOptions<phd::configurations::SVMParams> &args) {

    cout << "Testing Classifier against Test Set..." << endl;

    std::vector<phd::devices::accelerometer::data::Features> features;
    std::vector<int> labels;

    auto sliding_function = [](int window) { return window - 1; };

    if (phd::io::is_dir(args.test_set.data())) {
        vector<cv::String> globs;
        cv::glob(args.test_set + "/*.json", globs);

        for (const string ds : globs) {
            const auto rawData = phd::devices::accelerometer::utils::readJSONDataset(ds);
            phd::devices::accelerometer::utils::toFeatures(rawData, "z", sliding_function, features, labels);
        }

    } else if (phd::io::is_file(args.test_set.data())) {

        const auto rawData = phd::devices::accelerometer::utils::readJSONDataset(args.test_set);
        phd::devices::accelerometer::utils::toFeatures(rawData, "z", sliding_function, features, labels);

    } else {
        cerr << "Undefined directory or file " << args.test_set << endl;
        exit(-3);
    }

    const cv::Mat test_data = phd::devices::accelerometer::data::toMat(features);

    const cv::Mat normalized_test_data =
            phd::devices::accelerometer::data::normalize(
                    test_data,
                    args.norm_range.first,
                    args.norm_range.second,
                    args.norm_method
            );

    auto test_labels = phd::devices::accelerometer::data::classify(normalized_test_data, args.model);
    float tp = 0, fp = 0, fn = 0, tn = 0;

    for (int i = 0; i < labels.size(); ++i) {
        if (test_labels.at<float>(0, i) == 1 && labels[i] == 1) {
            tp++;
        } else if (test_labels.at<float>(0, i) == 0 && labels[i] == 0) {
            tn++;
        } else if (test_labels.at<float>(0, i) == 1 && labels[i] == 0) {
            fp++;
        } else {
            fn++;
        }
    }

    cout << "TP: " << tp << " | TN: " << tn << " | FP: " << fp << " | FN: " << fn << endl;

    cout << "Precision: " << (tp/(tp+fp)) << endl;
    cout << "Recall/Sensitivity: " << (tp/(tp+fn)) << endl;
    cout << "F1: " << (2*tp/(2*tp+fp+fn)) << endl;
}