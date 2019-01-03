#ifndef POTHOLEDETECTIONSYSTEM_EMBEDDEDAPP_CONFIGURATION_UTILS_H
#define POTHOLEDETECTIONSYSTEM_EMBEDDEDAPP_CONFIGURATION_UTILS_H

#include <iostream>
#include <opencv2/ml.hpp>

#include <phdetection/io.hpp>

namespace phd {
    namespace configurations {

        typedef phd::io::Configuration PhDConfig;

        struct ServerConfig {
            std::string protocol;
            std::string hostname;
            int port;
            std::string api;
        };

        struct CVArgs {
            std::string method;
            std::string bayes;
            std::string svm;
            bool rotate;
        };

        template <typename T>
        struct MLOptions {
            std::string train_set;
            std::string test_set;
            std::string model;
            cv::NormTypes norm_method;
            std::pair<float, float> norm_range;
            std::pair<std::string, T> params;
        };

        struct SVMParams {
            cv::ml::SVM::Types type;
            cv::ml::SVM::KernelTypes kernel;
            int k_fold;
            int max_iter;
            double epsilon;
            double C;
            double gamma;
            bool balanced_folding;
        };

        struct EmbeddedAppConfiguration{
            CVArgs cvConfig;
            PhDConfig phdConfig;
            ServerConfig serverConfig;
            MLOptions<SVMParams> svmConfig;
            std::string serialPortName;
        };

        struct CommandLineArgs {
            std::string mode;
            bool withoutRx;
            bool useCamera;
            bool saveAxelValues;
            bool savePositiveCaptures;
            std::string axelOutputLocation;
            std::string captureSaveLocation;
        };

        CVArgs loadCVArgs(const std::string path_to_config);

        std::string loadSerialPortFromConfig(std::string path_to_config);

        ServerConfig loadServerConfig(std::string path_to_config);

        MLOptions<SVMParams> loadSVMOptions(const std::string &path_to_config);

        CommandLineArgs parseCommandLine(int argc, char* argv[]);

    }};
#endif //POTHOLEDETECTIONSYSTEM_EMBEDDEDAPP_CONFIGURATION_UTILS_H