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
        CvArgs cvConfig,
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

        this_thread::sleep_for(chrono::milliseconds(500));
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

void testFeatureExtraction() {

    std::vector<float> v(7);

    for (int i = 0; i < v.size(); ++i) {
        v[i] = static_cast<float>(i + 1);
    }

    cout << "Test Array: "; print_vector(v);

    int window_size = 2;

    vector<float> expected[6] = {
            vector<float>({ 1.0, 2.0 }),
            vector<float>({ 2.0, 3.0 }),
            vector<float>({ 3.0, 4.0 }),
            vector<float>({ 4.0, 5.0 }),
            vector<float>({ 5.0, 6.0 }),
            vector<float>({ 6.0, 7.0 }),
    };

    cout << "Test window creation..." << endl;

    auto res = phd::devices::accelerometer::getWindow(v, window_size);

    cout << "Result: "; print_vector(res);
    cout << "Expected: "; print_vector(expected[5]);

    cout << "Expected value matches calculated value: " <<
        (std::equal(res.begin(), res.begin(), expected[5].begin()) ? "true" : "false")
    << endl;

    int slider = 0, i = 0;

    while (slider < v.size() - window_size / 2) {

        res = phd::devices::accelerometer::getWindow(v, window_size, slider);

        slider += window_size / 2;

        cout << "Result: "; print_vector(res);
        cout << "Expected: "; print_vector(expected[i]);

        cout << "Expected value matches calculated value: " <<
            (std::equal(res.begin(), res.begin(), expected[i++].begin()) ? "true" : "false")
        << endl;

    }

    auto ft = phd::devices::accelerometer::getFeatures(v);

    cout << "Mean: " << ft.mean << " | Confidence: " << ft.mean_confidence << endl;
    cout << "Variance: " << ft.variance << endl;
    cout << "STD_DEV: " << ft.std_dev << " | Confidence: " << ft.std_dev_confidence << endl;
    cout << "RSD: " << ft.relative_std_dev << " | Confidence: " << ft.relative_std_dev_confidence << endl;
    cout << "Max-Min Diff: " << ft.max_min_diff << " | Confidence: " << ft.max_min_diff_confidence << endl;
    cout << "Confidence Sum: " << ft.confidences_sum << " | Confidence: " << ft.confidences_sum_confidence << endl;
    cout << "Num of (statistical) features over the threshold (n.d.r. have high-confidence): " << ft.thresholds_overpass_count << "/5" << endl;

}

void trainAccelerometer(char **argv) {

    auto trainset_type = std::string(argv[2]);
    auto trainset = std::string(argv[3]);
    auto testset_type = std::string(argv[4]);
    auto testset = std::string(argv[5]);
    auto svm_acc_model = std::string(argv[6]);

    std::vector<phd::devices::accelerometer::Features> features;
    std::vector<int> labels;

    auto sliding_function = [](int window) {return window - 1;};

    if (trainset_type == "-d") {
        vector<cv::String> globs;
        cv::glob(trainset + "/*.json", globs);

        for (const string ds : globs) {
            cout << ds << endl;
            const auto rawData = phd::devices::accelerometer::utils::readJSONDataset(ds);

            phd::devices::accelerometer::utils::toFeatures(rawData, "z", sliding_function, features, labels);
        }

    } else if (trainset_type == "-f") {

        const phd::devices::accelerometer::utils::RawData rawData =
                phd::devices::accelerometer::utils::readJSONDataset(trainset);

        phd::devices::accelerometer::utils::toFeatures(rawData, "z", sliding_function, features, labels);

    } else {
        cerr << "Undefined parameter " << trainset_type << endl;
        exit(-3);
    }

    auto stub = std::vector<int>();

    std::copy_if(labels.begin(), labels.end(), back_inserter(stub), [](int v){ return v == 1;});

    cout << stub.size() << endl;

    phd::devices::accelerometer::training(
            features,
            cv::Mat(cv::Size(1, static_cast<int>(labels.size())), CV_32SC1, labels.data()),
            svm_acc_model,
            5, 1000, exp(-5)
    );

    cout << "Testing Classifier against Test Set..." << endl;

    features.clear();
    labels.clear();

    if (testset_type == "-d") {
        vector<cv::String> globs;
        cv::glob(testset + "/*.json", globs);

        for (const string ds : globs) {
            cout << ds << endl;
            const auto rawData = phd::devices::accelerometer::utils::readJSONDataset(ds);

            phd::devices::accelerometer::utils::toFeatures(rawData, "z", sliding_function, features, labels);
        }

    } else if (testset_type == "-f") {

        const phd::devices::accelerometer::utils::RawData rawData =
                phd::devices::accelerometer::utils::readJSONDataset(testset);

        phd::devices::accelerometer::utils::toFeatures(rawData, "z", sliding_function, features, labels);

    } else {
        cerr << "Undefined parameter " << testset_type << endl;
        exit(-3);
    }

    stub.clear();
    std::copy_if(labels.begin(), labels.end(), back_inserter(stub), [](int v){ return v == 1;});
    cout << stub.size() << endl;

    auto test_labels = phd::devices::accelerometer::classify(features, svm_acc_model);
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

    cout << "TP: " << tp << endl;
    cout << "TN: " << tn << endl;
    cout << "FP: " << fp << endl;
    cout << "FN: " << fn << endl;
//    cout << "Accuracy: " << ((tp+tn)/labels.size()) << endl;
    cout << "Precision: " << (tp/(tp+fp)) << endl;
    cout << "Recall/Sensitivity: " << (tp/(tp+fn)) << endl;
    cout << "F1: " << (2*tp/(2*tp+fp+fn)) << endl;
}