//
// Created by Xander on 27/9/2018.
//

#include "networking.h"

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/prettywriter.h>

#include <iostream>
#include <fstream>
#include <stdio.h>

using namespace rapidjson;
using namespace std;

namespace phd::devices::networking {

    string getURL(const ServerConfig config) {
        return getURL(config.protocol, config.hostname, config.port, config.api);
    }

    string getURL(const string protocol, const string hostname, const int port, const string api) {
        return string().append(protocol).append("://")
                .append(hostname).append(":").append(to_string(port))
                .append(api);
    }

    namespace HTTP {

        CURLcode init() {
            return curl_global_init(CURL_GLOBAL_ALL);
        }

        void close() {
            curl_global_cleanup();
        }

        CURLcode POST(
                std::string url,
                std::vector<std::pair<std::string, std::string>> headers,
                std::string &payload
            ) {


            CURL *curl  = curl_easy_init();
            curl_slist *_headers = NULL;

            if (curl == NULL) {
                return CURLE_FAILED_INIT;
            }

            cout << url << endl;

            for (auto p : headers) {
                auto h = std::string().append(p.first).append(": ").append(p.second);
                _headers = curl_slist_append(_headers, h.data());
            }

            curl_easy_setopt(curl, CURLOPT_URL, url.data());
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, _headers);
            curl_easy_setopt(curl, CURLOPT_USERAGENT, "curl/7.x");
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.data());

            CURLcode res = curl_easy_perform(curl);
            curl_slist_free_all(_headers);
            curl_easy_cleanup(curl);

            return res;
        }

        CURLcode GET (
                std::string url,
                std::vector<std::pair<std::string, std::string>> headers,
                std::vector<std::pair<std::string, std::string>> params
            ) {

            CURL *curl = curl_easy_init();;
            curl_slist *_headers = NULL;

            if (curl == NULL) {
                return CURLE_FAILED_INIT;
            }

            for (auto p : headers) {
                auto h = std::string().append(p.first).append(": ").append(p.second);
                _headers = curl_slist_append(_headers, h.data());
            }

            curl_easy_setopt(curl, CURLOPT_URL, url.data());
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, _headers);
            curl_easy_setopt(curl, CURLOPT_USERAGENT, "curl/7.x");

            CURLcode res = curl_easy_perform(curl);
            curl_slist_free_all(_headers);
            curl_easy_cleanup(curl);

            return res;
        }

    }

}