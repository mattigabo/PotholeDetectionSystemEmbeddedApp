//
// Created by Xander on 27/9/2018.
//

#ifndef POTHOLEDETECTIONEMBEDDEDAPP_NETWORKING_H
#define POTHOLEDETECTIONEMBEDDEDAPP_NETWORKING_H

#include <string>
#include <stdio.h>
#include <iostream>
#include <curl/curl.h>
#include <vector>
#include "ConfigurationUtils.h"

using namespace phd::configurations;

namespace phd::devices::networking {


    std::string getURL(std::string protocol, std::string hostname, int port, std::string api);
    std::string getURL(ServerConfig config);

    namespace HTTP {

        CURLcode init();

        CURLcode POST (
                std::string url,
                std::vector<std::pair<std::string, std::string>> headers,
                std::string &payload
            );

        CURLcode GET (
                std::string url,
                std::vector<std::pair<std::string, std::string>> headers,
                std::vector<std::pair<std::string, std::string>> params
            );

        void close();
    };
}

#endif //POTHOLEDETECTIONEMBEDDEDAPP_NETWORKING_H
