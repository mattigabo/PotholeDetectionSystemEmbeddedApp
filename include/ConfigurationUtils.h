#ifndef POTHOLEDETECTIONEMBEDDEDAPP_CONFIGURATION_UTILS_H
#define POTHOLEDETECTIONEMBEDDEDAPP_CONFIGURATION_UTILS_H
#include <iostream>

using namespace std;

namespace phd::configurations {

        typedef struct ServerConfig {
            std::string protocol;
            std::string hostname;
            int port;
            std::string api;
        } ServerConfig;

        typedef struct Arguments {
            string method;
            string bayes;
            string svm;
            bool rotate;
        } CvArgs;

        CvArgs loadCvConfig(const string path_to_config);

        string loadSerialPortFromConfig(string path_to_config);

        ServerConfig loadServerConfig(string path_to_config);
};
#endif //POTHOLEDETECTIONEMBEDDEDAPP_CONFIGURATION_UTILS_H