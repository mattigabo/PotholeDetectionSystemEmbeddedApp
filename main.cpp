#include <iostream>
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
#include <opencv2/highgui.hpp>

#include <iot/camera.h>
#include <iot/gps.h>
#include <iot/networking.h>

#include <rapidjson/document.h>
#include <rapidjson/writer.h>

using namespace cv;
using namespace std;
using namespace cv::ml;
using namespace phd::io;
using namespace phd::iot::networking;

Configuration phdConfig;
ServerConfig serverConfig;

string config_folder = "/res/config";
string data_folder = "/res/features";
string results_folder = "/res/results";
string svm_folder = "/res/svm";
string nbayes_folder = "/res/bayes";

void showHelper(void) {

    cout << "-o [== Run Observation process on the RasPi Camera] -{bayes, svm, multi} -b bayes_model.yml -s svm_model.yml" << endl;

}

Mat go(const string &method, const string &bayes_model, const string &svm_model, Mat &image, const Configuration &config) {

    cout << endl << "---------------" << image << endl;

    auto features = phd::getFeatures(image, config);

    cv::Mat labels;

    try {
        labels = phd::classify(method, svm_folder + "/" + svm_model, nbayes_folder + "/" + bayes_model, features);
    } catch(phd::UndefinedMethod &ex)  {
        cerr << "ERROR: " << ex.what() << endl;
        exit(-1);
    }

    cout << "LABELS: " << labels << endl;

    return labels;
}

std::string toJSON(phd::iot::gps::Coordinates coordinates) {
    rapidjson::Document document;

    document.Parse("{}");

    assert(document.IsObject());

    document.AddMember("lat", rapidjson::Value(coordinates.lat), document.GetAllocator());
    document.AddMember("lng", rapidjson::Value(coordinates.lng), document.GetAllocator());

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    document.Accept(writer);

    return buffer.GetString();
}

int main(int argc, char *argv[]) {

//    cout << phd::io::GetCurrentWorkingDir() << endl;

    const string root = phd::io::getParentDirectory(string(dirname(argv[0])));

    config_folder = root + config_folder;
    data_folder = root + data_folder;
    results_folder = root + results_folder;
    svm_folder = root + svm_folder;
    nbayes_folder = root + nbayes_folder;

    cout << config_folder << endl;
    cout << data_folder << endl;
    cout << results_folder << endl;
    cout << svm_folder << endl;
    cout << nbayes_folder << endl;

    cout << "cURL Global Initialization: " << HTTP::init() << endl;

    if (argc < 2) {
        showHelper();
        return 0;
    } else {

        auto mode = string(argv[1]);
        auto poison_pill = false;

        phdConfig = loadProgramConfiguration(config_folder + "/config.json");
        serverConfig = phd::iot::networking::loadServerConfig(config_folder + "/config.json");

        if (mode == "-o" && argc > 6) {

            auto method = string(argv[2]);
            auto bayes_model = string();
            auto svm_model = string();

            if (string(argv[3]) == "-b" && string(argv[5]) == "-s") {
                bayes_model = string(argv[4]);
                svm_model = string(argv[6]);
            } else if (string(argv[5]) == "-b" && string(argv[3]) == "-s") {
                bayes_model = string(argv[6]);
                svm_model = string(argv[4]);
            } else {
                cerr << "Unknown model types " << string(argv[5]) << " or " << string(argv[3]) << endl;
            }

            const vector<pair<string, string>> headers({
                pair<string, string>("Accept", "application/json"),
                pair<string, string>("Content-Type","application/json"),
                pair<string, string>("charset","utf-8")
            });

            while(!poison_pill) {

                std::string position = toJSON(phd::iot::gps::fetch());

                Mat image = phd::iot::camera::fetch();

                Mat labels = cv::Mat::ones(1, 10, CV_32SC1);
//                        go(method, bayes_model, svm_model, image, phdConfig).row(0);

                vector<int> l(labels.ptr<int>(0), labels.ptr<int>(0) + labels.cols);

                if (std::find(l.begin(), l.end(), 1) != l.end() ||
                    std::find(l.begin(), l.end(), 2) != l.end()) {

                    CURLcode res = HTTP::POST(getURL(serverConfig), headers, position);

                    cout << "HTTP Response Code:" << res << endl;
                }

                this_thread::sleep_for(chrono::milliseconds(500));
            }

        } else {
            showHelper();
        }
    }

    HTTP::close();

    return 1;
}