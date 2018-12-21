//
// Created by Xander on 27/9/2018.
//

#ifndef POTHOLEDETECTIONSYSTEM_EMBEDDEDAPP_NETWORKING_H
#define POTHOLEDETECTIONSYSTEM_EMBEDDEDAPP_NETWORKING_H

#include <string>
#include <stdio.h>
#include <iostream>
#include <curl/curl.h>
#include <vector>
#include "configurationutils.h"

//using namespace phd::configurations;

namespace phd{
    namespace devices {
        namespace networking {


            std::string getURL(std::string protocol, std::string hostname, int port, std::string api);
            std::string getURL(phd::configurations::ServerConfig config);

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
    }
}

#endif //POTHOLEDETECTIONSYSTEM_EMBEDDEDAPP_NETWORKING_H
