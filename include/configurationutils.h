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
    struct MLOptions {
        std::string train_set;
        std::string test_set;
        std::string model;
        cv::NormTypes norm_method;
        std::pair<float, float> norm_range;
        std::pair<string, T> params;
    };

    typedef struct SVMParams {
        cv::ml::SVM::Types type;
        cv::ml::SVM::KernelTypes kernel;
        int k_fold;
        int max_iter;
        double epsilon;
        double C;
        double gamma;
        bool balanced_folding;
    } SVMParams;

    CVArgs loadCVArgs(const string path_to_config);

    string loadSerialPortFromConfig(string path_to_config);

    ServerConfig loadServerConfig(string path_to_config);

    MLOptions<SVMParams> loadSVMOptions(const string &path_to_config);
};
#endif //POTHOLEDETECTIONEMBEDDEDAPP_CONFIGURATION_UTILS_H