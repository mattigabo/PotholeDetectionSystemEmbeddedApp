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

    cout << "-o [== Run Observation process on the RasPi Camera] -{bayes, svm, multi}" << endl;

}

Mat go(const string &method, const string &model_name, Mat &image, const Configuration &config) {

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

    return labels;
}

void classificationPhase(char*argv[], Mat target, const Configuration &config){

    auto method = string(argv[2]);
    auto model_name = string(argv[5]);

    cout << "classify Method: " << method << endl;

    portable_mkdir(results_folder.data());
    portable_mkdir((results_folder + "/neg").data());
    portable_mkdir((results_folder + "/pos").data());

    /*--------------------------------- classify Phase ------------------------------*/
    int numberOfCandidatesFound = 0;
    numberOfCandidatesFound += go(method, model_name, target, config).cols;

    cout << "Candidates at first segmentation found: " << numberFirstSPCandidatesFound << endl;
    cout << "Candidates found: " << numberOfCandidatesFound << endl;
}

int main(int argc, char *argv[]) {

//    cout << phd::io::GetCurrentWorkingDir() << endl;

    if (argc < 2) {
        showHelper();
        return 0;
    } else {

        auto mode = string(argv[1]);
        auto poison_pill = false;
        config = loadProgramConfiguration(config_folder + "/config.json");

        printThresholds(config.primaryThresholds);
        printThresholds(config.secondaryThresholds);
        printOffsets(config.offsets);

        if (mode == "-o" && argc > 2) {
            while(!poison_pill) {
                // 1. Fetch Image
                // 2. Fetch GPS Coordinates
                //
                Mat image;
                classificationPhase(argv, image, config);
            }

        } else {
            showHelper();
        }
    }

    return 1;
}