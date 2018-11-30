#ifndef POTHOLEDETECTIONEMBEDDEDAPP_CONFIGURATION_UTILS_H
#define POTHOLEDETECTIONEMBEDDEDAPP_CONFIGURATION_UTILS_H
#include <iostream>
#include <opencv2/ml.hpp>

using namespace std;

namespace phd::configurations {

    typedef struct ServerConfig {
        std::string protocol;
        std::string hostname;
        int port;
        std::string api;
    } ServerConfig;

    typedef struct CVArgs {
        string method;
        string bayes;
        string svm;
        bool rotate;
    } CVArgs;

    template <typename T>
    struct CrossValidationArgs {
        std::string trainset;
        std::string testset;
        std::string model;
        std::pair<string, T> params;
    };

    typedef struct SVMParams {
        cv::ml::SVM::Types type;
        cv::ml::SVM::KernelTypes kernel;
        int kfold;
        int max_iter;
        double epsilon;
        double C;
        double gamma;
    } SVMParams;

    CVArgs loadCVArgs(const string path_to_config);

    string loadSerialPortFromConfig(string path_to_config);

    ServerConfig loadServerConfig(string path_to_config);

    CrossValidationArgs<SVMParams> loadSVMCrossValidationArgs(const string &path_to_config);
};
#endif //POTHOLEDETECTIONEMBEDDEDAPP_CONFIGURATION_UTILS_H