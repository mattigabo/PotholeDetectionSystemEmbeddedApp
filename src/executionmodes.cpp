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