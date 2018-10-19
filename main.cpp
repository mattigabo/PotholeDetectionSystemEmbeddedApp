#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>
#include <libgen.h>

#include <phdetection/ontologies.hpp>
#include <phdetection/core.hpp>
#include <phdetection/io.hpp>
#include <phdetection/svm.hpp>
#include <phdetection/bayes.hpp>

#include <opencv2/core.hpp>
#include <opencv2/ml.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/video.hpp>

#include <gps/GPSDataStore.h>
#include <gps/GPSDataUpdater.h>
#include <serialport/SerialPort.h>
#include <serialport/SigrokSerialPortWrapper.h>
#include <camera.h>
#include <networking.h>
#include <ConfigurationUtils.h>

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/istreamwrapper.h>


using namespace cv;
using namespace std;
using namespace cv::ml;
using namespace phd::io;
using namespace phd::devices::networking;
using namespace phd::devices::serialport;
using namespace phd::devices::gps;
using namespace phd::configurations;
using namespace rapidjson;

Configuration phdConfig;
ServerConfig serverConfig;
string serialPortName;

string config_folder = "/res/config";

void showHelper(void) {

    cout << "-o [== Run Observation process on the RasPi Camera]" << endl;
    cout << "-gps [== Test the gps communication]" << endl;
}

Mat go(const string &method, const string &bayes_model, const string &svm_model, Mat &image, const Configuration &config) {

    cout << endl << "---------------" << image << endl;

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

std::string toJSON(phd::devices::gps::Coordinates coordinates) {
    rapidjson::Document document;

    document.Parse("{}");

    assert(document.IsObject());

    document.AddMember("latitude", rapidjson::Value(coordinates.latitude), document.GetAllocator());
    document.AddMember("longitude", rapidjson::Value(coordinates.longitude), document.GetAllocator());

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    document.Accept(writer);

    return buffer.GetString();
}

SerialPort* initSerialPort(string portName){
    SerialPort* sp = new SigrokSerialPortWrapper(portName);
    sp->openPort(READ);
    return sp;
}

void runObservationMode(bool poison_pill, GPSDataStore* gpsDataStore){
    Args args = loadCvConfig(config_folder + "/config.json");

    const vector<pair<string, string>> headers({
                                                       pair<string, string>("Accept", "application/json"),
                                                       pair<string, string>("Content-Type","application/json"),
                                                       pair<string, string>("charset","utf-8")
                                               });


    std::cout << "Capture Device ID: " << cv::VideoCaptureAPIs::CAP_ANY << std::endl;

    while(!poison_pill) {

        std::string position = toJSON(gpsDataStore->fetch());

        Mat image = phd::devices::camera::fetch(cv::VideoCaptureAPIs::CAP_ANY);

        if (args.rotate) {
            cv::rotate(image, image, cv::ROTATE_180);
        }

//                cv::imshow("Capture", image);
//                waitKey(0);

        Mat labels = go(args.method, args.bayes, args.svm, image, phdConfig).row(0);

        vector<int> l(labels.ptr<int>(0), labels.ptr<int>(0) + labels.cols);

        if (std::find(l.begin(), l.end(), 1) != l.end() ||
            std::find(l.begin(), l.end(), 2) != l.end()) {

            CURLcode res = HTTP::POST(getURL(serverConfig), headers, position);

            cout << "HTTP Response Code:" << res << endl;
        }

//                this_thread::sleep_for(chrono::milliseconds(500));
    }
}

void testGPSCommunication(GPSDataStore* storage){
    for(int i=0; i < 10; i++) {
        Coordinates coordinates = storage->fetch();
        cout << "LATITUDE: " << coordinates.latitude <<
             " LONGITUDE:" << coordinates.longitude <<
             " ALTITUDE: " << coordinates.altitude << endl;
        std::this_thread::sleep_for(1s);
    }
}

int main(int argc, char *argv[]) {

//    cout << phd::io::GetCurrentWorkingDir() << endl;

    const string root = phd::io::getParentDirectory(string(dirname(argv[0])));

    config_folder = root + config_folder;

    cout << config_folder << endl;

    cout << "cURL Global Initialization: " << HTTP::init() << endl;

    if (argc < 2) {
        showHelper();
        return 0;
    } else {

        auto mode = std::string(argv[1]);
        auto poison_pill = false;

        phdConfig = loadProgramConfiguration(config_folder + "/config.json");
        serverConfig = loadServerConfig(config_folder + "/config.json");
        serialPortName = loadSerialPortFromConfig(config_folder + "/config.json");

        SerialPort* serialPort = initSerialPort(serialPortName);
        auto gpsDataStore = new GPSDataStore();
        auto updater = new phd::devices::gps::GPSDataUpdater(gpsDataStore, serialPort);

        if (mode == "-o") {
            runObservationMode(poison_pill, gpsDataStore);
        } else if(mode == "-gps") {
            testGPSCommunication(gpsDataStore);
        } else {
            showHelper();
        }

        updater->kill();
        updater->join();
        serialPort->closePort();
        delete(updater);
        delete(serialPort);
        delete(gpsDataStore);
    }


    HTTP::close();
    return 1;
}