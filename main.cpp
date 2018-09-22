#include <iostream>
#include <phdetection/ontologies.hpp>
#include <phdetection/core.hpp>
#include <phdetection/io.hpp>
#include <phdetection/svm.hpp>
#include <phdetection/bayes.hpp>

#include <opencv2/core.hpp>
#include <opencv2/ml.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

using namespace cv;
using namespace std;
using namespace cv::ml;
//using namespace phd::ml::utils;
using namespace phd::io;

int numberFirstSPCandidatesFound = 0;

Configuration config;

const string config_folder = "../res/config";
const string data_folder = "../res/features";
const string results_folder = "../res/results";
const string svm_folder = "../res/svm";
const string nbayes_folder = "../res/bayes";

void showHelper(void) {

    cout << "-o == Run Observation process on the RasPi Camera";

}

Mat go(const string &method, const string &model_name, const string &image, const Configuration &config) {

    cout << endl << "---------------" << image << endl;

    auto features = phd::getFeatures(image, config);

    cv::Mat labels;

    try {
        labels = phd::classify(method, svm_folder + "/" + model_name, nbayes_folder + "/" + model_name, features);
    } catch(phd::UndefinedMethod &ex)  {
        cerr << "ERROR: " << ex.what() << endl;
        exit(-1);
    }

    cout << "LABELS: " << labels << endl;

    for (int i = 0; i < features.size(); ++i) {

        string folder = results_folder + "/neg/";

        if (labels.at<int>(0, i) > 0 || labels.at<float>(0, i) > 0) {
            folder = results_folder + "/pos/";
        }

        imwrite(
                folder +
                extractFileName(set_format(image, "", false), "/") +
                "_L" + to_string(features[i].label) +
                "_" + to_string(features[i].id) +
                ".bmp",
                features[i].candidate
        );
    }

    return labels;
}

void classificationPhase(char*argv[], const Configuration &config){

    auto method = string(argv[2]);
    auto target_type = string(argv[3]);
    auto target = string(argv[4]);
    auto model_name = string(argv[5]);

    cout << "classify Method: " << method << endl;

    portable_mkdir(results_folder.data());
    portable_mkdir((results_folder + "/neg").data());
    portable_mkdir((results_folder + "/pos").data());

    /*--------------------------------- classify Phase ------------------------------*/
    int numberOfCandidatesFound = 0;
    if (target_type == "-d") { /// Whole folder

        vector<string> fn = extractImagePath(target);

        cout << "Number of image found in the directory: " << fn.size() << endl;
        for (const auto &image : fn) {
            numberOfCandidatesFound += go(method, model_name, image, config).cols;
        }
    } else if (target_type == "-i") { /// Single Image
        numberOfCandidatesFound += go(method, model_name, target, config).cols;
    }

    cout << "Candidates at first segmentation found: " << numberFirstSPCandidatesFound << endl;
    cout << "Candidates found: " << numberOfCandidatesFound << endl;
}

int main(int argc, char *argv[]) {

//    cout << phd::io::GetCurrentWorkingDir() << endl;

    if (argc < 2) {
        return 0;
    } else {

        auto mode = string(argv[1]);

        config = loadProgramConfiguration(config_folder + "/config.json");

        printThresholds(config.primaryThresholds);
        printThresholds(config.secondaryThresholds);
        printOffsets(config.offsets);

        if (mode == "-o" && argc > 2) {
            classificationPhase(argv, config);
        } else {
            showHelper();
        }
    }

    return 1;
}