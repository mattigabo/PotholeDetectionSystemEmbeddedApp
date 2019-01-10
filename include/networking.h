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
#include <raspberrypi/led.h>

#include "configurationutils.h"


namespace phd{
    namespace devices {
        namespace networking {


            std::string getURL(std::string protocol, std::string hostname, int port, std::string api);
            std::string getURL(phd::configurations::ServerConfig config);

            namespace HTTP {

                typedef std::vector<std::pair<std::string, std::string>> Params;
                typedef std::vector<std::pair<std::string, std::string>> Headers;
                typedef std::string URL;
                typedef std::string Payload;

                CURLcode init();

                CURLcode POST(
                        HTTP::URL url,
                        HTTP::Headers headers,
                        HTTP::Payload payload
                );

                CURLcode GET (
                        HTTP::URL url,
                        HTTP::Headers headers,
                        HTTP::Params params
                    );

                void close();

                namespace async {

                    CURLcode init();

                    void POST(HTTP::URL url,
                              HTTP::Headers headers,
                              HTTP::Payload payload,
                              std::function<void(CURLcode)> callback,
                              phd::devices::raspberry::led::Led *led = nullptr);

                    void close();
                }
            };
        }
    }
}

#endif //POTHOLEDETECTIONSYSTEM_EMBEDDEDAPP_NETWORKING_H
