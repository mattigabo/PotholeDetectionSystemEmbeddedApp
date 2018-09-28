//
// Created by Xander on 27/9/2018.
//

#ifndef POTHOLEDETECTIONOBSERVER_NETWORKING_H
#define POTHOLEDETECTIONOBSERVER_NETWORKING_H

#include <string>
#include <stdio.h>
#include <iostream>
#include <curl/curl.h>
#include <vector>

namespace phd::iot::networking {

    typedef struct ServerConfig {
        std::string protocol;
        std::string hostname;
        int port;
        std::string api;
    } ServerConfig;

    std::string getURL(std::string protocol, std::string hostname, int port, std::string api);
    std::string getURL(ServerConfig config);

    ServerConfig loadServerConfig (std::string path_to_config);

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

#endif //POTHOLEDETECTIONOBSERVER_NETWORKING_H
