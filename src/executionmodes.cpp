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
#include <accelerometer.h>
#include <accelerometerutils.h>

using namespace std;
using namespace cv;
using namespace cv::ml;

using namespace phd::io;
using namespace phd::devices::networking;
using namespace phd::devices::serialport;
using namespace phd::devices::gps;

using namespace rapidjson;

const vector<pair<string, string>> httpHeaders({
       pair<string, string>("Accept", "application/json"),
       pair<string, string>("Content-Type","application/json"),
       pair<string, string>("charset","utf-8")
});

std::string toJSON(phd::devices::gps::Coordinates coordinates) {
    rapidjson::Document document;

    document.Parse("{}");

    assert(document.IsObject());

    document.AddMember("lat", rapidjson::Value(coordinates.latitude), document.GetAllocator());
    document.AddMember("lng", rapidjson::Value(coordinates.longitude), document.GetAllocator());

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    document.Accept(writer);

    return buffer.GetString();
}

void sendDataToServer(string payload, ServerConfig serverConfig){
    CURLcode res = HTTP::POST(getURL(serverConfig), httpHeaders, payload);

    cout << "HTTP Response Code:" << res << endl;
}

Mat extractFeaturesAndClassify(const string &method, const string &bayes_model, const string &svm_model, Mat &image,
                               const Configuration &config) {

    //cout << endl << "---------------" << image << endl;

    auto features = phd::getFeatures(image, config);

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
        GPSDataStore* gpsDataStore,
        Configuration phdConfig,
        CVArgs cvConfig,
        ServerConfig serverConfig){

    std::cout << "Capture Device ID: " << cv::VideoCaptureAPIs::CAP_ANY << std::endl;

    while(!poison_pill) {

        std::string position = toJSON(gpsDataStore->fetch());

        Mat image = phd::devices::camera::fetch(cv::VideoCaptureAPIs::CAP_ANY);

        if (cvConfig.rotate) {
            cv::rotate(image, image, cv::ROTATE_180);
        }

//                cv::imshow("Capture", image);
//                waitKey(0);

        Mat labels = extractFeaturesAndClassify(cvConfig.method, cvConfig.bayes, cvConfig.svm, image, phdConfig);
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

void testGPSCommunication(GPSDataStore* storage){
    for(int i=0; i < 100; i++) {
        Coordinates coordinates = storage->fetch();
        cout << "LATITUDE: " << coordinates.latitude <<
             " LONGITUDE:" << coordinates.longitude <<
             " ALTITUDE: " << coordinates.altitude << endl;
        std::this_thread::sleep_for(1s);
    }
}

void testHTTPCommunication(ServerConfig serverConfig){
    Coordinates pointNearUniversity = {44.147618, 12.235476, 0};
    sendDataToServer(toJSON(pointNearUniversity), serverConfig);
}

void testLed(NotificationLeds notificationLeds){
    cout << "Test LED that notify the program execution" << endl;
    notificationLeds.programInExecution.switchOn();
    std::this_thread::sleep_for(1s);
    notificationLeds.programInExecution.switchOff();
    std::this_thread::sleep_for(1s);

    cout << "Test LED that notify the valid gps data" << endl;
    notificationLeds.validGpsData.switchOn();
    std::this_thread::sleep_for(1s);
    notificationLeds.validGpsData.switchOff();
    std::this_thread::sleep_for(1s);

    cout << "Test LED that notify that the data is being transferred to the server" << endl;
    notificationLeds.serverDataTransfering.switchOn();
    std::this_thread::sleep_for(1s);
    notificationLeds.serverDataTransfering.switchOff();
    std::this_thread::sleep_for(1s);

    cout << "Test LED that notify that the camera is taking a picture" << endl;
    notificationLeds.cameraIsShooting.switchOn();
    std::this_thread::sleep_for(1s);
    notificationLeds.cameraIsShooting.switchOff();
    std::this_thread::sleep_for(1s);
}

template <typename T>
void print_vector(const std::vector<T> v) {
    cout << "[";
    for (int i = 0; i < v.size(); ++i) {
        cout << v.at(i);
        cout << (i + 1 < v.size() ? ", " : "");
    }
    cout << "]" << endl;
}

void trainAccelerometer(const phd::configurations::SVMArgs &args) {

    std::vector<phd::devices::accelerometer::Features> features;
    std::vector<int> labels;

    auto sliding_function = [](int window) { return window - 1; };

    if (phd::io::is_dir(args.trainset.data())) {
        vector<cv::String> globs;
        cv::glob(args.trainset + "/*.json", globs);

        for (const string ds : globs) {
            const auto rawData = phd::devices::accelerometer::utils::readJSONDataset(ds);
            phd::devices::accelerometer::utils::toFeatures(rawData, "z", sliding_function, features, labels);
        }

    } else if (phd::io::is_file(args.trainset.data())) {

        const auto rawData = phd::devices::accelerometer::utils::readJSONDataset(args.trainset);
        phd::devices::accelerometer::utils::toFeatures(rawData, "z", sliding_function, features, labels);

    } else {
        cerr << "Undefined directory or file " << args.trainset << endl;
        exit(-3);
    }

    const cv::Mat train_data = toMat(features);
    const cv::Mat normalized_train_data = phd::devices::accelerometer::normalize(train_data, 0.1, 0.9,
                                                                                 cv::NORM_MINMAX);

    phd::devices::accelerometer::training(
            normalized_train_data,
            cv::Mat(cv::Size(1, static_cast<int>(labels.size())), CV_32SC1, labels.data()),
            args.model,
            args.C,
            args.gamma,
            args.max_iter,
            args.epsilon
    );
    features.clear();
    labels.clear();
}

void testAccelerometer(const phd::configurations::SVMArgs &args) {

    cout << "Testing Classifier against Test Set..." << endl;

    std::vector<phd::devices::accelerometer::Features> features;
    std::vector<int> labels;

    auto sliding_function = [](int window) { return window - 1; };

    if (phd::io::is_dir(args.testset.data())) {
        vector<cv::String> globs;
        cv::glob(args.testset + "/*.json", globs);

        for (const string ds : globs) {
            const auto rawData = phd::devices::accelerometer::utils::readJSONDataset(ds);
            phd::devices::accelerometer::utils::toFeatures(rawData, "z", sliding_function, features, labels);
        }

    } else if (phd::io::is_file(args.testset.data())) {

        const auto rawData = phd::devices::accelerometer::utils::readJSONDataset(args.testset);
        phd::devices::accelerometer::utils::toFeatures(rawData, "z", sliding_function, features, labels);

    } else {
        cerr << "Undefined directory or file " << args.testset << endl;
        exit(-3);
    }

    const cv::Mat test_data = toMat(features);

    const cv::Mat normalized_test_data = phd::devices::accelerometer::normalize(test_data, 0.1, 0.9, cv::NORM_MINMAX);

    auto test_labels = phd::devices::accelerometer::classify(normalized_test_data, args.model);
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