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
#include <iot/camera.h>
#include <iot/gps.h>
#include <iot/networking.h>

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/istreamwrapper.h>

using namespace cv;
using namespace std;
using namespace cv::ml;
using namespace phd::io;
using namespace phd::iot::networking;
using namespace rapidjson;

Configuration phdConfig;
ServerConfig serverConfig;
typedef struct Arguments {
    string method;
    string bayes;
    string svm;
    bool rotate;
} Args;

string config_folder = "/res/config";

void showHelper(void) {

    cout << "-o [== Run Observation process on the RasPi Camera]" << endl;

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

Args load(const string path_to_config) {

    Args args;

    ifstream json(path_to_config, fstream::in);

    IStreamWrapper wrapper(json);

    Document config;

    config.ParseStream(wrapper);

    if (json.is_open() && config.IsObject()) {

        cout << "Opened file " << path_to_config << endl;

        assert(config.HasMember("args"));
        assert(config["args"].HasMember("method"));
        assert(config["args"].HasMember("bayes"));
        assert(config["args"].HasMember("svm"));
        assert(config["args"].HasMember("rotate"));

        args.method = config["args"]["method"].GetString();
        args.bayes = config["args"]["bayes"].GetString();
        args.svm = config["args"]["svm"].GetString();
        args.rotate = config["args"]["rotate"].GetBool();

    } else {
        cerr << "Program arguments configuration is missing. Check it's existence or create a new config.json"
             << " under the ../res/config/ folder inside the program directory. " << endl;

        exit(-3);
    }

    json.close();

    return args;

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
        serverConfig = phd::iot::networking::loadServerConfig(config_folder + "/config.json");

        if (mode == "-o" && argc > 4) {

            Args args = load(config_folder + "/config.json");

            const vector<pair<string, string>> headers({
                pair<string, string>("Accept", "application/json"),
                pair<string, string>("Content-Type","application/json"),
                pair<string, string>("charset","utf-8")
            });

            while(!poison_pill) {

                std::string position = toJSON(phd::iot::gps::fetch());

                Mat image = phd::iot::camera::fetch(cv::VideoCaptureAPIs::CAP_ANY);

                if (args.rotate) {
                    cv::rotate(image, image, cv::ROTATE_180);
                }

                Mat labels = go(args.method, args.bayes, args.svm, image, phdConfig).row(0);

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